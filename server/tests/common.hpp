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

#pragma once
#include "gtest/gtest.h"
#include "KVBCInterfaces.h"
#include "db_interfaces.h"
#include "storage/test/storage_test_common.h"
#include "categorization/kv_blockchain.h"
#include "db.hpp"

using namespace concord::kvbc;
using namespace concord::storage;
using namespace concord::kvbc::categorization;

namespace chappie::tests
{
    class test_rocksdb : public ::testing::Test
    {
        void SetUp() override
        {
            destroyDb();
            db = TestRocksDb::createNative();
        }

        void TearDown() override { destroyDb(); }

        void destroyDb()
        {
            db.reset();
            ASSERT_EQ(0, db.use_count());
            cleanup();
        }

    protected:
        std::shared_ptr<concord::storage::rocksdb::NativeClient> db;
    };

    class TestStorage : public IReader, public IBlockAdder, public IBlocksDeleter
    {
    public:
        TestStorage(std::shared_ptr<::concord::storage::rocksdb::NativeClient> native_client)
            : bc_{native_client,
                  false,
                  std::map<std::string, CATEGORY_TYPE>{{chappie::db::CHAPPIE_CATEGORY, CATEGORY_TYPE::versioned_kv},
                                                       {"concord_internal", CATEGORY_TYPE::versioned_kv}}} {}

        // IBlockAdder interface
        BlockId add(categorization::Updates &&updates) override { return bc_.addBlock(std::move(updates)); }

        // IReader interface
        std::optional<categorization::Value> get(const std::string &category_id,
                                                 const std::string &key,
                                                 BlockId block_id) const override
        {
            return bc_.get(category_id, key, block_id);
        }

        std::optional<categorization::Value> getLatest(const std::string &category_id,
                                                       const std::string &key) const override
        {
            return bc_.getLatest(category_id, key);
        }

        void multiGet(const std::string &category_id,
                      const std::vector<std::string> &keys,
                      const std::vector<BlockId> &versions,
                      std::vector<std::optional<categorization::Value>> &values) const override
        {
            bc_.multiGet(category_id, keys, versions, values);
        }

        void multiGetLatest(const std::string &category_id,
                            const std::vector<std::string> &keys,
                            std::vector<std::optional<categorization::Value>> &values) const override
        {
            bc_.multiGetLatest(category_id, keys, values);
        }

        void multiGetLatestVersion(const std::string &category_id,
                                   const std::vector<std::string> &keys,
                                   std::vector<std::optional<categorization::TaggedVersion>> &versions) const override
        {
            bc_.multiGetLatestVersion(category_id, keys, versions);
        }

        std::optional<categorization::TaggedVersion> getLatestVersion(const std::string &category_id,
                                                                      const std::string &key) const override
        {
            return bc_.getLatestVersion(category_id, key);
        }

        std::optional<categorization::Updates> getBlockUpdates(BlockId block_id) const override
        {
            return bc_.getBlockUpdates(block_id);
        }

        BlockId getGenesisBlockId() const override
        {
            if (mockGenesisBlockId.has_value())
                return mockGenesisBlockId.value();
            return bc_.getGenesisBlockId();
        }

        BlockId getLastBlockId() const override { return bc_.getLastReachableBlockId(); }

        // IBlocksDeleter interface
        void deleteGenesisBlock() override
        {
            const auto genesisBlock = bc_.getGenesisBlockId();
            bc_.deleteBlock(genesisBlock);
        }

        BlockId deleteBlocksUntil(BlockId until) override
        {
            const auto genesisBlock = bc_.getGenesisBlockId();
            if (genesisBlock == 0)
            {
                throw std::logic_error{"Cannot delete a block range from an empty blockchain"};
            }
            else if (until <= genesisBlock)
            {
                throw std::invalid_argument{"Invalid 'until' value passed to deleteBlocksUntil()"};
            }

            const auto lastReachableBlock = bc_.getLastReachableBlockId();
            const auto lastDeletedBlock = std::min(lastReachableBlock, until - 1);
            for (auto i = genesisBlock; i <= lastDeletedBlock; ++i)
            {
                ConcordAssert(bc_.deleteBlock(i));
            }
            return lastDeletedBlock;
        }

        void setGenesisBlockId(BlockId bid) { mockGenesisBlockId = bid; }

    private:
        concord::kvbc::categorization::KeyValueBlockchain bc_;
        std::optional<BlockId> mockGenesisBlockId = {};
    };
}