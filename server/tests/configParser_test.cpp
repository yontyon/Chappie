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
#include "chappieConfiguration.hpp"
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
namespace chappie::tests
{
    TEST(parse_configuration, parse_configuration)
    {
        auto conf = chappie::configuraion::Config::parse(fs::current_path() / "resources/test_conf.yaml");
        int a = 1;
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
