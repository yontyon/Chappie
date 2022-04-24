cmake_minimum_required(VERSION 3.5)
project(Chappie)

# Require C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

string(APPEND CMAKE_CXX_FLAGS " -pedantic") # Mostly for -Werror=vla
string(APPEND CMAKE_CXX_FLAGS " -march=x86-64") # Generate code specific for x68-64 architecture
string(APPEND CMAKE_CXX_FLAGS " -mtune=generic") # Generate code optimized for the most common processors
string(APPEND CMAKE_EXE_LINKER_FLAGS " -pie -Wl,-z,relro,-z,now")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Storage options
set(BUILD_ROCKSDB_STORAGE TRUE)
set(USE_ROCKSDB TRUE)
# Default USE_LOG4CPP to true
option(USE_LOG4CPP "Enable LOG4CPP" TRUE)
set(CONCORD_LOGGER_NAME "concord.bft")

# Default BUILD_COMM_TCP_PLAIN to FALSE
option(BUILD_COMM_TCP_PLAIN "Enable TCP communication" FALSE)

# Default BUILD_COMM_TCP_TLS to true
option(BUILD_COMM_TCP_TLS "Enable TCP TLS communication" TRUE)

# Our CMake helper modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Concord-BFT CMake helper modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/submodules/concord-bft/cmake")

# Concord-BFT CMF compiler path
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/submodules/concord-bft/messages/compiler/")

find_package(Boost 1.65 COMPONENTS system thread program_options filesystem
        REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(CMFC REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)

# find_package(Evmone REQUIRED)
# include_directories(${EVM_INCLUDE_DIR})
# set(EVM_LIBRARIES ${Evmone_LIBRARIES})
set(BUILD_TESTING false)
find_package(cryptopp REQUIRED)

find_package(OpenSSL REQUIRED)

add_subdirectory("submodules/concord-bft")
add_subdirectory("server")
add_subdirectory("cmf")
