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

#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <utility>
#include "chappie.cmf.hpp"
#include "KVBCInterfaces.h"
#include "db_interfaces.h"

namespace chappie::consensus
{
    class ExecutionHandler : public concord::kvbc::ICommandsHandler
    {
    public:
        ExecutionHandler(const concord::kvbc::IReader &reader, concord::kvbc::IBlockAdder &block_adder);
        void execute(ExecutionRequestsQueue &requestList,
                     std::optional<bftEngine::Timestamp> timestamp,
                     const std::string &batchCid,
                     concordUtils::SpanWrapper &parent_span) override;
        void setPerformanceManager(std::shared_ptr<concord::performance::PerformanceManager> perfManager) override;
        void preExecute(IRequestsHandler::ExecutionRequest &req,
                        std::optional<bftEngine::Timestamp> timestamp,
                        const std::string &batchCid,
                        concordUtils::SpanWrapper &parent_span) override {}
        template <typename T>
        chappie::messages::ChappieReply executeChappieRequest(const T &request, const std::optional<bftEngine::Timestamp> &timestamp);

    private:
        concord::kvbc::BlockId addBlock(const std::vector<uint8_t> &data, std::string key, const std::optional<bftEngine::Timestamp> &timestamp);
        logging::Logger getLogger() const
        {
            static logging::Logger logger_(logging::getLogger("chappie.consensus.commandHandler.reconfiguration"));
            return logger_;
        }

        const concord::kvbc::IReader &reader_;
        concord::kvbc::IBlockAdder &block_adder_;
    };
};
