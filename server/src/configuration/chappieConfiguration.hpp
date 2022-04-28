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
#include "communication/CommDefs.hpp"
namespace chappie::configuraion
{
    struct BftCryptoSysConfig
    {
    };
    struct SecretsConfig
    {
        std::string rsa_private_key;
        std::set<std::pair<uint16_t, const std::string>> rsa_public_keys_of_replicas;

        uint32_t slow_commit_thresh;
        uint32_t commit_thresh;
        uint32_t optimistic_thresh;
        uint32_t num_signers;

        std::pair<std::string, std::string> commit_cryptosys_type;
        std::pair<std::string, std::string> optimisitc_cryptosys_type;
        std::pair<std::string, std::string> slow_cryptosys_type;
        std::string commit_private_key;
        std::string optimistic_commit_private_key;
        std::string slow_commit_private_key;
        std::vector<std::string> commit_pubkeys;
        std::vector<std::string> optimistic_pubkeys;
        std::vector<std::string> slow_commit_pubkeys;

        std::pair<std::string, std::string> commit_cryptosys;
        std::pair<std::string, std::string> optimistic_commit_cryptosys;
    };

    struct BftCommConfig
    {
        std::string communication_type;
        bool multiplex_enabled;
        uint16_t listen_port;
        std::string listen_ip;
        uint32_t buffer_length;
        std::unordered_map<bft::communication::NodeNum, bft::communication::NodeInfo> nodes;
        uint32_t max_server_id;
        uint32_t id;
        std::string cert_root_path;
        std::string tls_cipher_suite_list;
        std::unordered_map<bft::communication::NodeNum, bft::communication::NodeNum> endpoint_id_to_node_id_map;
        bft::communication::UPDATE_CONNECTIVITY_FN status_callback;
        std::optional<concord::secretsmanager::SecretData> secret_data;
    };

    struct BftConfig
    {
        std::string keysFilePrefix;
        std::string certRootPath;
        std::string txnSigningKeysPath;
        std::string crypto_system_config_path;
        std::map<uint16_t, std::set<uint32_t>> principalsMapping;
        uint16_t replica_id;
        uint32_t batching_max_req_num;
        uint16_t n_val;
        uint16_t f_val;
        std::string blockchain_db_path;
        uint32_t num_bft_clients_per_client_node;
        BftCommConfig comm_config;
    };

    struct Config
    {
        std::map<uint16_t, std::string> ips;
        std::string chabby_fs_metadata_root;
        uint32_t num_client_nodes;
        BftConfig bft_config;
        SecretsConfig secret_config;
        static Config parse(const std::string &path);
    };

}