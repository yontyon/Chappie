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

#include "Replica.h"
#include "chappieConfiguration.hpp"
#include "communication/CommDefs.hpp"
#include "ReplicaConfig.hpp"
#include <memory>

namespace chappie::consensus
{
    class Replica
    {
    private:
        bft::communication::ICommunication *create_replica_comm(chappie::configuraion::BftCommConfig &config);
        bftEngine::ReplicaConfig &init_replica_configuration(chappie::configuraion::Config &config);
        concord::kvbc::Replica bft_replica_;
    };
}
