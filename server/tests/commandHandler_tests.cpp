// Chappie
//
// Copyright (c) 2018-2021 yontyon, Inc. All Rights Reserved.
//
// This product is licensed to you under the Apache 2.0 license (the "License").
// You may not use this product except in compliance with the Apache 2.0 License.
//
// This product may include a number of subcomponents with separate copyright
// notices and license terms. Your use of these subcomponents is subject to the
// terms and conditions of the subcomponent's license, as noted in the LICENSE
// file.

#include "gtest/gtest.h"
#include "common.hpp"
#include "commandHandler.hpp"
#include "chappie.cmf.hpp"

using namespace chappie::consensus;
namespace chappie::tests
{
    TEST_F(test_rocksdb, test_create_node)
    {
        TestStorage storage(db);
        ExecutionHandler handler(storage, storage);
        chappie::messages::Node node{"Yoni", false, "/shared/yoni", "Hello world"};
        bftEngine::Timestamp time;
        time.time_since_epoch = std::chrono::duration_cast<bftEngine::ConsensusTime>(std::chrono::system_clock::now().time_since_epoch());
        chappie::messages::ChappieReply rep = handler.executeChappieRequest(node, time);
        ASSERT_EQ(storage.getLastBlockId(), 1);
        ASSERT_TRUE(rep.succ);
        ASSERT_EQ(std::get<chappie::messages::CreateNodeReply>(rep.reply).blockid, 1);
    }

    TEST_F(test_rocksdb, test_heratbeat)
    {
        TestStorage storage(db);
        ExecutionHandler handler(storage, storage);
        chappie::messages::HeartBeat heartbeat{"Yoni"};
        bftEngine::Timestamp time;
        time.time_since_epoch = std::chrono::duration_cast<bftEngine::ConsensusTime>(std::chrono::system_clock::now().time_since_epoch());
        chappie::messages::ChappieReply rep = handler.executeChappieRequest(heartbeat, time);
        ASSERT_EQ(storage.getLastBlockId(), 1);
        ASSERT_TRUE(rep.succ);
        ASSERT_EQ(std::get<chappie::messages::HeartBeatReply>(rep.reply).blockid, 1);
    }

    TEST_F(test_rocksdb, test_getUpdates)
    {
        TestStorage storage(db);
        ExecutionHandler handler(storage, storage);
        chappie::messages::Node node{"Yoni", false, "/shared/yoni", "Hello world"};
        bftEngine::Timestamp time;
        time.time_since_epoch = std::chrono::duration_cast<bftEngine::ConsensusTime>(std::chrono::system_clock::now().time_since_epoch());
        handler.executeChappieRequest(node, time);
        ASSERT_EQ(storage.getLastBlockId(), 1);
        chappie::messages::HeartBeat heartbeat{"Yoni"};
        bftEngine::Timestamp time1;
        time1.time_since_epoch = std::chrono::duration_cast<bftEngine::ConsensusTime>(std::chrono::system_clock::now().time_since_epoch());
        handler.executeChappieRequest(heartbeat, time1);
        ASSERT_EQ(storage.getLastBlockId(), 2);

        chappie::messages::GetUpdates getUpdates{"Yoni", 1, 2};
        chappie::messages::ChappieReply rep = handler.executeChappieRequest(getUpdates, std::nullopt);
        ASSERT_TRUE(rep.succ);
        auto updateRep = std::get<chappie::messages::Updates>(rep.reply);
        ASSERT_EQ(updateRep.updates.size(), 2);
        auto node_data = std::get<chappie::messages::Node>(updateRep.updates[0].data);
        ASSERT_EQ(node_data.data, "Hello world");
        ASSERT_EQ(node_data.path, "/shared/yoni");
        auto timestamp = bftEngine::ConsensusTime(updateRep.updates[0].timestamp);
        ASSERT_EQ(time.time_since_epoch, timestamp);
        ASSERT_EQ(updateRep.updates.size(), 2);
        auto heartbeat_data = std::get<chappie::messages::HeartBeat>(updateRep.updates[1].data);
        ASSERT_EQ(heartbeat_data.sender, "Yoni");
        auto timestamp1 = bftEngine::ConsensusTime(updateRep.updates[1].timestamp);
        ASSERT_EQ(time1.time_since_epoch, timestamp1);
    }
}
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
