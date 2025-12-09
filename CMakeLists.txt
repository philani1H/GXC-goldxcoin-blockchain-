cmake_minimum_required(VERSION 3.16)
project(GXC_Blockchain VERSION 2.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
option(BUILD_GUI "Build Qt GUI applications" OFF)
option(BUILD_MINING_CLIENT "Build mining client software" ON)
option(BUILD_TESTS "Build test suite" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_STATIC "Build static executables" ON)
option(ENABLE_CCACHE "Enable ccache for faster compilation" ON)
option(ENABLE_LTO "Enable Link Time Optimization" OFF)
option(ENABLE_PROFILING "Enable profiling support" OFF)

# Platform-specific settings
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")
endif()

# Compiler-specific settings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(ENABLE_PROFILING)
        add_compile_options(-pg)
        add_link_options(-pg)
    endif()
    if(BUILD_STATIC)
        # Static linking flags
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif()

if(MSVC)
    add_compile_options(/W4 /permissive-)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    if(BUILD_STATIC)
        # Use static runtime on MSVC
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

# Enable ccache if available
if(ENABLE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    endif()
endif()

# Link Time Optimization
if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT lto_error)
    if(lto_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "Link Time Optimization enabled")
    else()
        message(WARNING "LTO not supported: ${lto_error}")
    endif()
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}")
    message(STATUS "  Include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "  Libraries: ${OPENSSL_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenSSL not found. Please install libssl-dev")
endif()

# System-specific libraries
find_package(Threads REQUIRED)
if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SYSTEMD libsystemd)
endif()

# Third-party libraries
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

# JSON library
if(NOT TARGET nlohmann_json::nlohmann_json)
    find_package(nlohmann_json QUIET)
    if(NOT nlohmann_json_FOUND)
        if(EXISTS "${THIRD_PARTY_DIR}/json/json.hpp")
            add_library(nlohmann_json INTERFACE)
            target_include_directories(nlohmann_json INTERFACE "${THIRD_PARTY_DIR}/json")
            add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
            message(STATUS "Using header-only nlohmann/json from third_party")
        else()
            message(WARNING "nlohmann_json not found. Some features may be unavailable.")
        endif()
    endif()
endif()

# LevelDB for blockchain storage
find_path(LEVELDB_INCLUDE_DIR leveldb/db.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
)

find_library(LEVELDB_LIBRARY leveldb
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /usr/lib/x86_64-linux-gnu
)

find_library(SNAPPY_LIBRARY snappy
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /usr/lib/x86_64-linux-gnu
)

if(LEVELDB_INCLUDE_DIR AND LEVELDB_LIBRARY)
    message(STATUS "Found LevelDB: ${LEVELDB_LIBRARY}")
    message(STATUS "  Include: ${LEVELDB_INCLUDE_DIR}")
    
    # Create LevelDB target
    add_library(LevelDB::LevelDB UNKNOWN IMPORTED)
    set_target_properties(LevelDB::LevelDB PROPERTIES
        IMPORTED_LOCATION "${LEVELDB_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LEVELDB_INCLUDE_DIR}"
    )
    
    if(SNAPPY_LIBRARY)
        message(STATUS "Found Snappy: ${SNAPPY_LIBRARY}")
        set_property(TARGET LevelDB::LevelDB APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES "${SNAPPY_LIBRARY}")
    endif()
else()
    message(FATAL_ERROR "LevelDB not found. Please install libleveldb-dev")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party)

# Source files for core blockchain
set(CORE_SOURCES
    src/main.cpp
    src/Blockchain.cpp
    src/block.cpp
    src/Transaction.cpp
    src/Wallet.cpp
    src/Validator.cpp
    src/SalectValidator.cpp
    src/ProofOfPrice.cpp
    src/CrossChainBridge.cpp
    src/HashUtils.cpp
    src/Crypto.cpp
    src/Database.cpp
    src/Config.cpp
    src/Logger.cpp
    src/Utils.cpp
    src/Keccak256.cpp
    src/Blake2b.cpp
    src/Argon2id.cpp
    src/EthashAlgorithm.cpp
)

# Network and API sources
set(NETWORK_SOURCES
    src/Network.cpp
    src/RPCAPI.cpp
    src/RESTServer.cpp
    src/WebSocketServer.cpp
    src/PeerManager.cpp
    src/MessageHandler.cpp
    src/Stratum.cpp
)

# Mining sources
set(MINING_SOURCES
    src/mining/MiningManager.cpp
    src/mining/SHA256Miner.cpp
    src/mining/EthashMiner.cpp
    src/mining/GXHashMiner.cpp
    src/mining/PoolManager.cpp
    src/mining/HardwareDetector.cpp
    src/mining/MiningOptimizer.cpp
)

# Governance and tokenization
set(GOVERNANCE_SOURCES
    src/governance/Governance.cpp
    src/governance/Proposals.cpp
    src/governance/Voting.cpp
    src/tokens/GoldToken.cpp
    src/tokens/StockContract.cpp
    src/tokens/TokenManager.cpp
)

# Security Engine
set(SECURITY_SOURCES
    src/security/SecurityEngine.cpp
)

# Header files
set(HEADERS
    include/Block.h
    include/blockchain.h
    include/transaction.h
    include/Wallet.h
    include/Validator.h
    include/SalectValidator.h
    include/ProofOfPrice.h
    include/CrossChainBridge.h
    include/HashUtils.h
    include/Network.h
    include/RPCAPI.h
    include/Governance.h
    include/GoldToken.h
    include/StockContract.h
    include/Database.h
    include/Config.h
    include/Logger.h
    include/Utils.h
)

# Core blockchain library
add_library(gxc_core STATIC
    ${CORE_SOURCES}
    ${NETWORK_SOURCES}
    ${GOVERNANCE_SOURCES}
    ${SECURITY_SOURCES}
)

target_link_libraries(gxc_core
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    LevelDB::LevelDB
)

if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(gxc_core nlohmann_json::nlohmann_json)
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(gxc_core ws2_32 wsock32 iphlpapi)
elseif(UNIX)
    target_link_libraries(gxc_core dl)
    if(SYSTEMD_FOUND)
        target_link_libraries(gxc_core ${SYSTEMD_LIBRARIES})
        target_include_directories(gxc_core PRIVATE ${SYSTEMD_INCLUDE_DIRS})
        target_compile_definitions(gxc_core PRIVATE HAVE_SYSTEMD)
    endif()
endif()

# Main GXC node executable
add_executable(gxc-node
    src/node_main.cpp
)

target_link_libraries(gxc-node gxc_core)

# Mining library
add_library(gxc_mining STATIC
    ${MINING_SOURCES}
)

target_link_libraries(gxc_mining gxc_core)

# CUDA support for GPU mining
find_package(CUDA QUIET)
if(CUDA_FOUND AND BUILD_MINING_CLIENT)
    enable_language(CUDA)
    add_definitions(-DHAVE_CUDA)
    target_link_libraries(gxc_mining ${CUDA_LIBRARIES})
    target_include_directories(gxc_mining PRIVATE ${CUDA_INCLUDE_DIRS})
endif()

# OpenCL support for GPU mining (optional)
find_package(OpenCL QUIET)
if(OpenCL_FOUND AND BUILD_MINING_CLIENT)
    message(STATUS "OpenCL found - GPU mining support enabled")
    add_definitions(-DHAVE_OPENCL)
    if(TARGET OpenCL::OpenCL)
        target_link_libraries(gxc_mining OpenCL::OpenCL)
    else()
        target_link_libraries(gxc_mining ${OpenCL_LIBRARIES})
        target_include_directories(gxc_mining PRIVATE ${OpenCL_INCLUDE_DIRS})
    endif()
else()
    if(BUILD_MINING_CLIENT)
        message(STATUS "OpenCL not found - GPU mining will use CUDA or CPU only")
    endif()
endif()

# Mining client executables
if(BUILD_MINING_CLIENT)
    # GXC Miner (all algorithms) - CLI
    add_executable(gxc-miner
        mining/main.cpp
    )
    target_link_libraries(gxc-miner gxc_mining gxc_core)
    
    # Specialized miners - CLI
    add_executable(gxc-sha256-miner
        mining/sha256_main.cpp
    )
    target_link_libraries(gxc-sha256-miner gxc_mining gxc_core)
    
    add_executable(gxc-ethash-miner
        mining/ethash_main.cpp
    )
    target_link_libraries(gxc-ethash-miner gxc_mining gxc_core)
    
    add_executable(gxc-gxhash-miner
        mining/gxhash_main.cpp
    )
    target_link_libraries(gxc-gxhash-miner gxc_mining gxc_core)
    
    # Pool mining proxy - CLI
    add_executable(gxc-pool-proxy
        mining/pool_proxy_main.cpp
    )
    target_link_libraries(gxc-pool-proxy gxc_mining gxc_core)
endif()

# Command-line tools
add_executable(gxc-cli
    tools/cli_main.cpp
    tools/CliInterface.cpp
)
target_link_libraries(gxc-cli gxc_core)

add_executable(gxc-keygen
    tools/keygen_main.cpp
    tools/KeyGenerator.cpp
)
target_link_libraries(gxc-keygen gxc_core)

add_executable(gxc-tx
    tools/tx_main.cpp
    tools/TransactionBuilder.cpp
)
target_link_libraries(gxc-tx gxc_core)

# Block explorer tool
add_executable(gxc-explorer
    tools/explorer_main.cpp
    tools/BlockExplorer.cpp
)
target_link_libraries(gxc-explorer gxc_core)

# Network diagnostic tool
add_executable(gxc-netdiag
    tools/netdiag_main.cpp
    tools/NetworkDiagnostic.cpp
)
target_link_libraries(gxc-netdiag gxc_core)

# Installation
include(GNUInstallDirs)

# Install executables
install(TARGETS gxc-node gxc-cli gxc-keygen gxc-tx gxc-explorer gxc-netdiag
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(BUILD_MINING_CLIENT)
    install(TARGETS gxc-miner gxc-sha256-miner gxc-ethash-miner gxc-gxhash-miner gxc-pool-proxy
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Install libraries
install(TARGETS gxc_core gxc_mining
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install headers
install(FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gxc
)

# Configuration files
install(FILES 
    config/gxc.conf.example
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/gxc
)

# Display build configuration
message(STATUS "")
message(STATUS "GXC Blockchain Build Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  OpenSSL found: ${OPENSSL_FOUND}")
message(STATUS "  LevelDB found: ${LEVELDB_LIBRARY}")
message(STATUS "  Build static: ${BUILD_STATIC}")
message(STATUS "  Build GUI: ${BUILD_GUI}")
message(STATUS "  Build mining client: ${BUILD_MINING_CLIENT}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
if(CUDA_FOUND)
    message(STATUS "  CUDA support: YES")
endif()
if(OpenCL_FOUND)
    message(STATUS "  OpenCL support: YES")
endif()
message(STATUS "")
