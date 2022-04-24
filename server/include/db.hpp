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
#include <string>

namespace chappie::db {
    struct Consts {
        static const CHAPPIE_CATEGORY{"chappie_cat"};
    };

    struct Keys {
        char ephemeral_node = 0x0;
        char consisent_node = 0x1;
    }
};