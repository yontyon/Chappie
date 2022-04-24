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

namespace chappie::consensus {
    ExecutionHandler::ExecutionHandler(const concord::kvbc::IReader& reader, concord::kvbc::IBlockAdder& block_adder) : reader_{reader}, block_adder_{block_adder} {}
    void ExecutionHandler::execute(ExecutionRequestsQueue& requestList,
               std::optional<bftEngine::Timestamp> timestamp,
               const std::string& batchCid,
               concordUtils::SpanWrapper& parent_span) {};
        void ExecutionHandler::setPerformanceManager(std::shared_ptr<concord::performance::PerformanceManager> perfManager) {};
}