// Chappie
//
// Copyright (c) 2019-2020 yontyon, Inc. All Rights Reserved.
//
// This product is licensed to you under the Apache 2.0 license (the "License").
// You may not use this product except in compliance with the Apache 2.0
// License.
//
// This product may include a number of subcomponents with separate copyright
// notices and license terms. Your use of these subcomponents is subject to the
// terms and conditions of the subcomponent's license, as noted in the LICENSE
// file.

#include "commandHandler.hpp"
#include "Replica.hpp"
#include "endianness.hpp"
#include "db.hpp"
#include "categorization/db_categories.h"
#include "categorization/updates.h"
#include <variant>
#include <string>
#include <regex>
#include <experimental/filesystem>
#include <fstream>

namespace fs = std::experimental::filesystem;

using namespace chappie::messages;
namespace chappie::consensus
{
    ExecutionHandler::ExecutionHandler(const concord::kvbc::IReader &reader, concord::kvbc::IBlockAdder &block_adder, const std::string &metadata_path, bool keep_old_data) : reader_{reader}, block_adder_{block_adder}, metadata_path_{metadata_path}
    {
        if (!keep_old_data && fs::exists(metadata_path))
        {
            fs::remove_all(metadata_path);
        }
        fs::create_directories(metadata_path);
    }
    concord::kvbc::BlockId ExecutionHandler::addBlock(const std::vector<uint8_t> &data, std::string key, const std::optional<bftEngine::Timestamp> &timestamp)
    {
        concord::kvbc::categorization::VersionedUpdates ver_updates;
        if (timestamp.has_value())
        {
            auto str_time = concordUtils::toBigEndianStringBuffer(timestamp.value().time_since_epoch.count());
            ver_updates.addUpdate(std::string{chappie::db::timestamp_key} + str_time, std::move(str_time));
        }
        ver_updates.addUpdate(std::move(key), std::string(data.begin(), data.end()));
        concord::kvbc::categorization::Updates updates;
        updates.add(chappie::db::CHAPPIE_CATEGORY, std::move(ver_updates));
        try
        {
            return block_adder_.add(std::move(updates));
        }
        catch (const std::exception &e)
        {
            LOG_ERROR(getLogger(), "failed to persist the chappie block: " << e.what());
            throw;
        }
        return 0;
    }
    template <>
    ChappieReply ExecutionHandler::executeChappieRequest(const Node &nodeReq, const std::optional<bftEngine::Timestamp> &timestamp)
    {
        concord::kvbc::BlockId block_id{0};
        std::regex path_pattern("^(/[^/ ]*)+/?$");
        if (!std::regex_search(nodeReq.path, path_pattern))
        {
            LOG_INFO(getLogger(), "invalid path: " << nodeReq.path);
            return ChappieReply{false, {}};
        }
        try
        {
            std::vector<uint8_t> data_vec;
            chappie::messages::serialize(data_vec, nodeReq);
            block_id = addBlock(data_vec, std::string{chappie::db::node_key} + nodeReq.path, timestamp);
            fs::path path(metadata_path_ + nodeReq.path);
            if (!fs::exists(path))
            {
                fs::create_directories(path.parent_path());
            }
            std::ofstream file(path);
            file << std::to_string(block_id) << std::endl;
            file.close();
        }
        catch (...)
        {
            return ChappieReply{false, {}};
        }
        return ChappieReply{true, CreateNodeReply{block_id, std::string()}};
    }
    template <>
    ChappieReply ExecutionHandler::executeChappieRequest(const GetUpdates &getUpdatesReq, const std::optional<bftEngine::Timestamp> &timestamp)
    {
        chappie::messages::Updates updates;
        if (getUpdatesReq.full_history)
        {
            updates = getFullPathUpdate(getUpdatesReq.path);
        }
        else
        {
            updates = getLatestPathData(getUpdatesReq.path);
        }
        return ChappieReply{true, updates};
    }
    template <>
    ChappieReply ExecutionHandler::executeChappieRequest(const HeartBeat &heartbeatReq, const std::optional<bftEngine::Timestamp> &timestamp)
    {
        concord::kvbc::BlockId block_id{0};
        try
        {
            std::vector<uint8_t> data_vec;
            chappie::messages::serialize(data_vec, heartbeatReq);
            block_id = addBlock(data_vec, std::string{chappie::db::heartbeat_key} + heartbeatReq.sender, timestamp);
        }
        catch (...)
        {
            return ChappieReply{false, {}};
        }
        return ChappieReply{true, HeartBeatReply{block_id}};
    }

    void ExecutionHandler::execute(ExecutionRequestsQueue &requestList,
                                   std::optional<bftEngine::Timestamp> timestamp,
                                   const std::string &batchCid,
                                   concordUtils::SpanWrapper &parent_span)
    {
        for (auto &req : requestList)
        {
            ChappieRequest chappie_req;
            deserialize(std::vector<uint8_t>(req.request, req.request + req.requestSize), chappie_req);
            ChappieReply chappie_rep = std::visit([&](auto &&arg)
                                                  { return executeChappieRequest(arg, timestamp); },
                                                  chappie_req.request);
            std::vector<uint8_t> serialized_response;
            serialize(serialized_response, chappie_rep);
            if (serialized_response.size() <= req.maxReplySize)
            {
                std::copy(serialized_response.begin(), serialized_response.end(), req.outReply);
                req.outActualReplySize = serialized_response.size();
                req.outReplicaSpecificInfoSize = 0;
            }
            else
            {
                std::string error("Response is too big");
                LOG_ERROR(getLogger(), error);
                req.outActualReplySize = 0;
            }
        }
    }
    void ExecutionHandler::setPerformanceManager(std::shared_ptr<concord::performance::PerformanceManager> perfManager){};
    Updates ExecutionHandler::getFullPathUpdate(const std::string &path)
    {
        chappie::messages::Updates updates;
        for (auto i = reader_.getGenesisBlockId(); i <= reader_.getLastBlockId(); i++)
        {
            auto data = getDataByBlockNum(i);
            if (!data)
                continue;
            if (std::holds_alternative<chappie::messages::Node>(data->data))
            {
                auto node = std::get<chappie::messages::Node>(data->data);
                if (!(node.path.rfind(path, 0) == 0))
                    continue;
            }
            updates.updates.emplace_back(*data);
        }
        return updates;
    }
    std::optional<chappie::messages::Data> ExecutionHandler::getDataByBlockNum(concord::kvbc::BlockId bid)
    {
        chappie::messages::Data data;
        data.blockid = bid;
        auto block_updates = reader_.getBlockUpdates(bid);
        if (!block_updates)
            return std::nullopt;
        const auto chapp_updates = block_updates->categoryUpdates(chappie::db::CHAPPIE_CATEGORY);
        if (!chapp_updates)
            return std::nullopt;
        const auto versioned = std::get<concord::kvbc::categorization::VersionedInput>(chapp_updates->get());
        for (const auto &[k, v] : versioned.kv)
        {
            if (k.rfind(std::string{chappie::db::node_key}, 0) == 0)
            {
                chappie::messages::Node node;
                chappie::messages::deserialize(std::vector<uint8_t>(v.data.begin(), v.data.end()), node);
                data.data = node;
            }
            else if (k.rfind(std::string{chappie::db::heartbeat_key}, 0) == 0)
            {
                chappie::messages::HeartBeat heartbeat;
                chappie::messages::deserialize(std::vector<uint8_t>(v.data.begin(), v.data.end()), heartbeat);
                data.data = heartbeat;
            }
            else if (k.rfind(std::string{chappie::db::timestamp_key}, 0) == 0)
            {
                data.timestamp = concordUtils::fromBigEndianBuffer<uint64_t>(v.data.c_str());
            }
        }
        return data;
    }

    chappie::messages::Updates ExecutionHandler::getLatestPathData(const std::string &path)
    {
        chappie::messages::Updates updates;
        std::set<std::string> seen_clients;
        for (const auto &entry : fs::recursive_directory_iterator(metadata_path_ + path))
        {
            if (fs::is_regular_file(entry.path()))
            {
                std::ifstream file(entry.path());
                int bid;
                file >> bid;
                file.close();
                auto data = getDataByBlockNum(bid);
                if (!data)
                    LOG_ERROR(getLogger(), "Unable to get specfic data from chain" << KVLOG(bid, path));
                seen_clients.emplace(std::get<chappie::messages::Node>(data->data).owner);
                updates.updates.emplace_back(*data);
            }
        }
        for (const auto &client : seen_clients)
        {
            auto chapp_updates = reader_.getLatest(chappie::db::CHAPPIE_CATEGORY, std::string{chappie::db::heartbeat_key} + client);
            ConcordAssert(chapp_updates != std::nullopt);
            const auto val = std::get<concord::kvbc::categorization::VersionedValue>(*chapp_updates);
            chappie::messages::HeartBeat heartbeat;
            chappie::messages::deserialize(std::vector<uint8_t>(val.data.begin(), val.data.end()), heartbeat);
            Data data;
            data.blockid = val.block_id;
            data.data = heartbeat;
            updates.updates.emplace_back(data);
        }
        return updates;
    }
}