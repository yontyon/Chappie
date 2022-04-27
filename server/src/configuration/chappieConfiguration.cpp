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

#include "chappieConfiguration.hpp"
#include <yaml-cpp/yaml.h>
#include <unistd.h>

namespace chappie::configuraion
{
    Config Config::parse(const std::string &path)
    {
        Config conf;
        YAML::Node config = YAML::LoadFile(path);
        uint16_t id = 0;
        for (const auto &ip : config["ips"])
        {
            conf.ips[id] = ip.as<std::string>();
            id++;
        }
        conf.chabby_fs_metadata_root = config["metadata_root"].as<std::string>();
        YAML::Node bft_config = config["bft_config"];
        auto &bft_conf = conf.bft_config;
        bft_conf.batching_max_req_num = bft_config["batching_max_req_num"].as<uint32_t>();
        bft_conf.certRootPath = bft_config["cert_root_path"].as<std::string>();
        bft_conf.keysFilePrefix = bft_config["keys_file_prefix"].as<std::string>();
        bft_conf.replica_id = bft_config["replica_id"].as<uint16_t>();
        bft_conf.txnSigningKeysPath = bft_config["tx_singing_key_path"].as<std::string>();
        bft_conf.n_val = bft_config["n"].as<uint32_t>();
        bft_conf.f_val = bft_config["f"].as<uint32_t>();
        for (const auto &element : bft_config["client_groups"])
        {
            auto gid = element.first.as<uint32_t>();
            for (const auto &id : element.second)
            {
                bft_conf.principalsMapping[gid].emplace(id.as<uint32_t>());
            }
        }
        return conf;
    }
}