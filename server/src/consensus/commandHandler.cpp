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
#include <variant>

using namespace chappie::messages;
namespace chappie::consensus {
    ExecutionHandler::ExecutionHandler(const concord::kvbc::IReader& reader, concord::kvbc::IBlockAdder& block_adder) : reader_{reader}, block_adder_{block_adder} {}
    
    template<>
    ChappieReply ExecutionHandler::executeChappieRequest(const Node& nodeReq) {
        return ChappieReply{};
    }
    template<>
    ChappieReply ExecutionHandler::executeChappieRequest(const GetUpdates& getUpdatesReq) {
        return ChappieReply{};
    }
    template<>
    ChappieReply ExecutionHandler::executeChappieRequest(const HeartBeat& hrertbeatReq) {
        return ChappieReply{};
    } 

    void ExecutionHandler::execute(ExecutionRequestsQueue& requestList,
               std::optional<bftEngine::Timestamp> timestamp,
               const std::string& batchCid,
               concordUtils::SpanWrapper& parent_span) {
                   for (auto& req : requestList) {
                        ChappieRequest chappie_req;
                        deserialize(std::vector<uint8_t>(req.request, req.request + req.requestSize), chappie_req);
                        ChappieReply chappie_rep = std::visit([&](auto&& arg) { return executeChappieRequest(arg); }, chappie_req.request);

                        std::vector<uint8_t> serialized_response;
                        serialize(serialized_response, chappie_rep);
                        if (serialized_response.size() <= req.maxReplySize) {
                            std::copy(serialized_response.begin(), serialized_response.end(), req.outReply);
                            req.outActualReplySize = serialized_response.size();
                            req.outReplicaSpecificInfoSize = 0;
                        } else {
                            std::string error("Response is too big");
                            LOG_ERROR(GL, error);
                            req.outActualReplySize = 0;
                        }      
                    }
               }
    void ExecutionHandler::setPerformanceManager(std::shared_ptr<concord::performance::PerformanceManager> perfManager) {};
}