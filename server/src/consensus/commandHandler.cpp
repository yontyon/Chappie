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

using namespace chappie::messages;
namespace chappie::consensus
{
    ExecutionHandler::ExecutionHandler(const concord::kvbc::IReader &reader, concord::kvbc::IBlockAdder &block_adder) : reader_{reader}, block_adder_{block_adder} {}
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
        auto from = getUpdatesReq.from;
        auto to = getUpdatesReq.to;
        if (to < from)
        {
            LOG_INFO(getLogger(), "invalid parametes, to < from");
            return ChappieReply{false, {}};
        }

        if (from < reader_.getGenesisBlockId())
        {
            LOG_INFO(getLogger(), "invalid parametes, request boundries are out of chain.");
            return ChappieReply{false, {}};
        }
        chappie::messages::Updates updates;
        for (auto i = from; i <= to; i++)
        {
            if (i > reader_.getLastBlockId())
                break;
            chappie::messages::Data data;
            data.blockid = i;
            auto block_updates = reader_.getBlockUpdates(i);
            if (!block_updates)
                continue;
            const auto chapp_updates = block_updates->categoryUpdates(chappie::db::CHAPPIE_CATEGORY);
            if (!chapp_updates)
                continue;
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
            updates.updates.emplace_back(data);
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
}