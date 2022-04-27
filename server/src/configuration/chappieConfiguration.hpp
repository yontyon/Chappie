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
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <yaml-cpp/yaml.h>

namespace chappie::configuraion
{
    struct BftConfig
    {
        std::string keysFilePrefix;
        std::string certRootPath;
        std::string txnSigningKeysPath;
        std::map<uint16_t, std::set<uint32_t>> principalsMapping;
        uint16_t replica_id;
        uint32_t batching_max_req_num;
        uint16_t n_val;
        uint16_t f_val;
    };

    struct Config
    {
        std::map<uint16_t, std::string> ips;
        std::string chabby_fs_metadata_root;
        BftConfig bft_config;
        static Config parse(const std::string &path);
    };

}