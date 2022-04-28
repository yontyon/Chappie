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

#include "chappieReplica.hpp"
#include "communication/CommFactory.hpp"
#include <memory>

namespace chappie::consensus
{
    bft::communication::ICommunication *Replica::create_replica_comm(chappie::configuraion::BftCommConfig &config)
    {
        bft::communication::ICommunication *icomm = nullptr;
        std::unique_ptr<bft::communication::BaseCommConfig> comm_config;
        if (config.communication_type == "tls")
        {
            if (config.multiplex_enabled)
            {
                comm_config = std::make_unique<bft::communication::TlsMultiplexConfig>(
                    config.listen_ip, config.listen_port, config.buffer_length, config.nodes, config.max_server_id,
                    config.id, config.cert_root_path, config.tls_cipher_suite_list, config.endpoint_id_to_node_id_map,
                    config.status_callback, config.secret_data);
            }
            else
            {
                comm_config = std::make_unique<bft::communication::TlsTcpConfig>(
                    config.listen_ip, config.listen_port, config.buffer_length, config.nodes, config.max_server_id,
                    config.id, config.cert_root_path, config.tls_cipher_suite_list, config.status_callback,
                    config.secret_data);
            }
            icomm = bft::communication::CommFactory::create(*comm_config.get());
        }
        else if (config.communication_type == "udp")
        {
            bft::communication::PlainUdpConfig configuration(config.listen_ip, config.listen_port,
                                                             config.buffer_length, config.nodes, config.id,
                                                             config.status_callback);
            icomm = bft::communication::CommFactory::create(configuration);
        }
        else
        {
            throw std::invalid_argument("Unknown communication module type" + config.communication_type);
        }
        return icomm;
    }

    bftEngine::ReplicaConfig &Replica::init_replica_configuration(chappie::configuraion::Config &config)
    {
        bftEngine::ReplicaConfig &conf = bftEngine::ReplicaConfig::instance();
        conf.replicaPrivateKey = config.secret_config.rsa_private_key;
        conf.publicKeysOfReplicas = config.secret_config.rsa_public_keys_of_replicas;
        conf.viewChangeProtocolEnabled = 20000; // TODO: hardcoded
        conf.statusReportTimerMillisec = 3000;
        conf.concurrencyLevel = 5;
        conf.numReplicas = config.bft_config.n_val;
        conf.numRoReplicas = 0;
        conf.replicaId = config.bft_config.replica_id;
        conf.fVal = config.bft_config.f_val;
        conf.cVal = 0;
        conf.numOfClientProxies = 0;
        conf.numOfExternalClients = config.num_client_nodes * config.bft_config.num_bft_clients_per_client_node;
        conf.numOfClientServices = config.num_client_nodes;
        conf.preExecutionFeatureEnabled = false;
        conf.debugStatisticsEnabled = false;
        conf.keyExchangeOnStart = true;
        conf.waitForFullCommOnStartup = false;
        conf.blockAccumulation = true;
        conf.keyViewFilePath = config.bft_config.keysFilePrefix;
        conf.clientBatchingEnabled = true;
        conf.enableMultiplexChannel = config.bft_config.comm_config.multiplex_enabled;
        conf.clientBatchingMaxMsgsNbr = 40;
        conf.batchingPolicy = bftEngine::BatchingPolicy::BATCH_SELF_ADJUSTED;
        conf.batchFlushPeriod = 1000;
        conf.maxNumOfRequestsInBatch = 50;
        conf.maxBatchSizeInBytes = 33554432;
        conf.maxInitialBatchSize = 350;
        conf.batchingFactorCoefficient = 4;
        conf.sizeOfInternalThreadPool = 8;
        conf.clientTransactionSigningEnabled = !config.bft_config.txnSigningKeysPath.empty();
        conf.viewChangeProtocolEnabled = true;

        // Crypto-systems
        std::unique_ptr<Cryptosystem> slow_commit_cryptosys = std::make_unique<Cryptosystem>(config.secret_config.slow_cryptosys_type.first, config.secret_config.slow_cryptosys_type.second, config.secret_config.num_signers, config.secret_config.slow_commit_thresh);
        std::unique_ptr<Cryptosystem> commit_cryptosys = std::make_unique<Cryptosystem>(config.secret_config.commit_cryptosys_type.first, config.secret_config.commit_cryptosys_type.second, config.secret_config.num_signers, config.secret_config.commit_thresh);
        std::unique_ptr<Cryptosystem> optimistic_commit_cryptosys = std::make_unique<Cryptosystem>(config.secret_config.optimisitc_cryptosys_type.first, config.secret_config.optimisitc_cryptosys_type.second, config.secret_config.num_signers, config.secret_config.optimistic_thresh);

        auto slow_commit_pubkeys = config.secret_config.slow_commit_pubkeys;
        slow_commit_pubkeys.insert(slow_commit_pubkeys.begin(), "");

        auto commit_pubkeys = config.secret_config.commit_pubkeys;
        commit_pubkeys.insert(commit_pubkeys.begin(), "");

        auto optimistic_pubkeys = config.secret_config.optimistic_pubkeys;
        optimistic_pubkeys.insert(optimistic_pubkeys.begin(), "");

        slow_commit_cryptosys->loadKeys(slow_commit_pubkeys[config.bft_config.replica_id + 1], slow_commit_pubkeys);
        slow_commit_cryptosys->loadPrivateKey(config.bft_config.replica_id + 1, config.secret_config.slow_commit_private_key);
        commit_cryptosys->loadKeys(commit_pubkeys[config.bft_config.replica_id + 1], commit_pubkeys);
        commit_cryptosys->loadPrivateKey(config.bft_config.replica_id + 1, config.secret_config.commit_private_key);
        optimistic_commit_cryptosys->loadKeys(optimistic_pubkeys[config.bft_config.replica_id + 1], optimistic_pubkeys);
        optimistic_commit_cryptosys->loadPrivateKey(config.bft_config.replica_id + 1, config.secret_config.optimistic_commit_private_key);

        return conf;
    }

}