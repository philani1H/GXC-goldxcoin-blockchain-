cmake_minimum_required(VERSION 3.16)
project(GXC_Blockchain VERSION 2.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
option(BUILD_GUI "Build Qt GUI applications" ON)
option(BUILD_MINING_CLIENT "Build mining client software" ON)
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_STATIC "Build static executables" OFF)
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
endif()

if(MSVC)
    add_compile_options(/W4 /permissive-)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
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
        message(STATUS "LTO enabled")
    else()
        message(WARNING "LTO not supported: ${lto_error}")
    endif()
endif()

# Find required packages
# Auto-detect Scoop OpenSSL location on Windows if not provided
if (WIN32 AND NOT DEFINED OPENSSL_ROOT_DIR)
    # Typical Scoop path e.g. C:/Users/User/scoop/apps/openssl/current
    if (EXISTS "$ENV{USERPROFILE}/scoop/apps/openssl/current")
        set(OPENSSL_ROOT_DIR "$ENV{USERPROFILE}/scoop/apps/openssl/current" CACHE PATH "Default Scoop OpenSSL path" FORCE)
        message(STATUS "OPENSSL_ROOT_DIR not set; using Scoop path: ${OPENSSL_ROOT_DIR}")
    endif()
endif()

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig QUIET)

# Find optional packages
find_package(Qt6 QUIET COMPONENTS Core Widgets Charts Network WebSockets)
if(NOT Qt6_FOUND)
    find_package(Qt5 QUIET COMPONENTS Core Widgets Charts Network WebSockets)
endif()

# System-specific libraries
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
        add_subdirectory(${THIRD_PARTY_DIR}/json EXCLUDE_FROM_ALL)
    endif()
endif()

# Crypto++ library
find_package(cryptopp QUIET)
if(NOT cryptopp_FOUND)
    add_subdirectory(${THIRD_PARTY_DIR}/cryptopp EXCLUDE_FROM_ALL)
endif()

# LevelDB for blockchain storage
find_package(leveldb QUIET)
if(NOT leveldb_FOUND)
    add_subdirectory(${THIRD_PARTY_DIR}/leveldb EXCLUDE_FROM_ALL)
endif()

# SQLite for wallet storage
find_package(SQLite3 REQUIRED)

# WebSocket library
find_package(websocketpp QUIET)
if(NOT websocketpp_FOUND)
    add_subdirectory(${THIRD_PARTY_DIR}/websocketpp EXCLUDE_FROM_ALL)
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
    src/Database.cpp
    src/Config.cpp
    src/Logger.cpp
    src/Utils.cpp
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

# Mining headers
set(MINING_HEADERS
    mining/GXCMiner.h
    mining/MiningManager.h
    mining/HardwareDetector.h
    mining/MiningOptimizer.h
    mining/ConfigManager.h
)

# Core blockchain library
add_library(gxc_core STATIC
    ${CORE_SOURCES}
    ${NETWORK_SOURCES}
    ${GOVERNANCE_SOURCES}
)

target_link_libraries(gxc_core
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    SQLite::SQLite3
)

if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(gxc_core nlohmann_json::nlohmann_json)
endif()

if(TARGET leveldb)
    target_link_libraries(gxc_core leveldb)
endif()

if(TARGET cryptopp)
    target_link_libraries(gxc_core cryptopp)
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

# OpenCL support for GPU mining
find_package(OpenCL QUIET)
if(OpenCL_FOUND AND BUILD_MINING_CLIENT)
    add_definitions(-DHAVE_OPENCL)
    target_link_libraries(gxc_mining OpenCL::OpenCL)
endif()

# Mining client executables
if(BUILD_MINING_CLIENT)
    # GXC Miner (all algorithms)
    add_executable(gxc-miner
        mining/main.cpp
        mining/MinerGUI.cpp
    )
    target_link_libraries(gxc-miner gxc_mining gxc_core)
    
    # Specialized miners
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
    
    # Pool mining proxy
    add_executable(gxc-pool-proxy
        mining/pool_proxy_main.cpp
    )
    target_link_libraries(gxc-pool-proxy gxc_mining gxc_core)
endif()

# Qt GUI applications
if(BUILD_GUI)
    if(Qt6_FOUND)
        set(QT_VERSION 6)
        set(QT_LIBRARIES Qt6::Core Qt6::Widgets Qt6::Charts Qt6::Network Qt6::WebSockets)
    elseif(Qt5_FOUND)
        set(QT_VERSION 5)
        set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Charts Qt5::Network Qt5::WebSockets)
    else()
        message(WARNING "Qt not found. GUI applications will not be built.")
        set(BUILD_GUI OFF)
    endif()
endif()

if(BUILD_GUI AND QT_VERSION)
    # Set Qt-specific settings
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    
    # Main wallet GUI
    add_executable(gxc-wallet
        gui/main.cpp
        gui/MainWindow.cpp
        gui/WalletWidget.cpp
        gui/MiningWidget.cpp
        gui/StakingWidget.cpp
        gui/NetworkWidget.cpp
        gui/SettingsDialog.cpp
        gui/TransactionDialog.cpp
        gui/AddressBookDialog.cpp
        gui/resources.qrc
    )
    
    target_link_libraries(gxc-wallet 
        gxc_core 
        gxc_mining 
        ${QT_LIBRARIES}
    )
    
    # Node management GUI
    add_executable(gxc-node-gui
        gui/node_main.cpp
        gui/NodeWindow.cpp
        gui/PeerWidget.cpp
        gui/BlockExplorer.cpp
        gui/NetworkStats.cpp
        gui/node_resources.qrc
    )
    
    target_link_libraries(gxc-node-gui 
        gxc_core 
        ${QT_LIBRARIES}
    )
    
    # Mining GUI
    add_executable(gxc-mining-gui
        gui/mining_main.cpp
        gui/MiningWindow.cpp
        gui/DeviceWidget.cpp
        gui/PoolWidget.cpp
        gui/StatsWidget.cpp
        gui/mining_resources.qrc
    )
    
    target_link_libraries(gxc-mining-gui 
        gxc_core 
        gxc_mining 
        ${QT_LIBRARIES}
    )
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

# Test suite
if(BUILD_TESTS)
    enable_testing()
    
    # Find test framework
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        add_subdirectory(${THIRD_PARTY_DIR}/googletest EXCLUDE_FROM_ALL)
        set(GTEST_LIBRARIES gtest gtest_main)
    else()
        set(GTEST_LIBRARIES GTest::gtest GTest::gtest_main)
    endif()
    
    # Test sources
    set(TEST_SOURCES
        tests/test_main.cpp
        tests/test_blockchain.cpp
        tests/test_transactions.cpp
        tests/test_wallet.cpp
        tests/test_mining.cpp
        tests/test_staking.cpp
        tests/test_network.cpp
        tests/test_api.cpp
        tests/test_governance.cpp
        tests/test_tokens.cpp
        tests/test_crypto.cpp
        tests/test_database.cpp
    )
    
    add_executable(gxc-tests ${TEST_SOURCES})
    target_link_libraries(gxc-tests 
        gxc_core 
        gxc_mining 
        ${GTEST_LIBRARIES}
    )
    
    # Register tests
    add_test(NAME blockchain_tests COMMAND gxc-tests --gtest_filter=Blockchain*)
    add_test(NAME transaction_tests COMMAND gxc-tests --gtest_filter=Transaction*)
    add_test(NAME wallet_tests COMMAND gxc-tests --gtest_filter=Wallet*)
    add_test(NAME mining_tests COMMAND gxc-tests --gtest_filter=Mining*)
    add_test(NAME staking_tests COMMAND gxc-tests --gtest_filter=Staking*)
    add_test(NAME network_tests COMMAND gxc-tests --gtest_filter=Network*)
    add_test(NAME api_tests COMMAND gxc-tests --gtest_filter=API*)
    add_test(NAME governance_tests COMMAND gxc-tests --gtest_filter=Governance*)
    add_test(NAME token_tests COMMAND gxc-tests --gtest_filter=Token*)
    add_test(NAME crypto_tests COMMAND gxc-tests --gtest_filter=Crypto*)
    
    # Performance benchmarks
    if(BUILD_BENCHMARKS)
        find_package(benchmark QUIET)
        if(NOT benchmark_FOUND)
            add_subdirectory(${THIRD_PARTY_DIR}/benchmark EXCLUDE_FROM_ALL)
        endif()
        
        add_executable(gxc-benchmarks
            benchmarks/benchmark_main.cpp
            benchmarks/benchmark_hashing.cpp
            benchmarks/benchmark_mining.cpp
            benchmarks/benchmark_transactions.cpp
            benchmarks/benchmark_network.cpp
        )
        
        target_link_libraries(gxc-benchmarks 
            gxc_core 
            gxc_mining 
            benchmark::benchmark
        )
    endif()
endif()

# Python bindings
find_package(Python3 COMPONENTS Interpreter Development)
if(Python3_FOUND)
    # pybind11 for Python bindings
    find_package(pybind11 QUIET)
    if(pybind11_FOUND OR EXISTS ${THIRD_PARTY_DIR}/pybind11)
        if(NOT pybind11_FOUND)
            add_subdirectory(${THIRD_PARTY_DIR}/pybind11)
        endif()
        
        pybind11_add_module(pygxc
            python/bindings/main.cpp
            python/bindings/blockchain_bindings.cpp
            python/bindings/wallet_bindings.cpp
            python/bindings/mining_bindings.cpp
            python/bindings/network_bindings.cpp
        )
        
        target_link_libraries(pygxc PRIVATE gxc_core gxc_mining)
        
        # Copy Python API to build directory
        configure_file(
            ${CMAKE_SOURCE_DIR}/python/gxc_api.py
            ${CMAKE_BINARY_DIR}/python/gxc_api.py
            COPYONLY
        )
    endif()
endif()

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

if(BUILD_GUI AND QT_VERSION)
    install(TARGETS gxc-wallet gxc-node-gui gxc-mining-gui
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

install(FILES ${MINING_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gxc/mining
)

# Install Python API
if(Python3_FOUND)
    install(FILES python/gxc_api.py
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages
    )
    
    if(TARGET pygxc)
        install(TARGETS pygxc
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages
        )
    endif()
endif()

# Configuration files
install(FILES 
    config/gxc.conf.example
    config/mining.conf.example
    config/pool.conf.example
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/gxc
)

# Service files for systemd
if(UNIX AND NOT APPLE)
    install(FILES 
        scripts/gxc-node.service
        scripts/gxc-miner.service
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system
    )
endif()

# Documentation
install(FILES 
    README.md
    LICENSE
    CHANGELOG.md
    docs/API.md
    docs/MINING.md
    docs/STAKING.md
    docs/GOVERNANCE.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# Create desktop entries for GUI applications
if(BUILD_GUI AND QT_VERSION AND UNIX AND NOT APPLE)
    install(FILES
        desktop/gxc-wallet.desktop
        desktop/gxc-node-gui.desktop
        desktop/gxc-mining-gui.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
    
    install(FILES
        icons/gxc-wallet.png
        icons/gxc-node.png
        icons/gxc-miner.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
    )
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "GXC-Blockchain")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GXC Blockchain - Advanced Cryptocurrency with Hybrid Consensus")
set(CPACK_PACKAGE_VENDOR "GXC Development Team")
set(CPACK_PACKAGE_CONTACT "support@gxc.network")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Platform-specific packaging
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "GXC Blockchain")
    set(CPACK_NSIS_PACKAGE_NAME "GXC-Blockchain")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://gxc.network")
    set(CPACK_NSIS_HELP_LINK "https://docs.gxc.network")
    set(CPACK_NSIS_MODIFY_PATH ON)
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
    set(CPACK_DMG_VOLUME_NAME "GXC Blockchain")
    set(CPACK_DMG_FORMAT "UDBZ")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    
    # Debian package
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "GXC Development Team <support@gxc.network>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "net")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://gxc.network")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libssl1.1, libsqlite3-0, libc6")
    
    # RPM package
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")
    set(CPACK_RPM_PACKAGE_URL "https://gxc.network")
    set(CPACK_RPM_PACKAGE_REQUIRES "openssl-libs, sqlite, glibc")
endif()

include(CPack)

# Display build configuration
message(STATUS "")
message(STATUS "GXC Blockchain Build Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build GUI: ${BUILD_GUI}")
if(BUILD_GUI)
    message(STATUS "  Qt version: ${QT_VERSION}")
endif()
message(STATUS "  Build mining client: ${BUILD_MINING_CLIENT}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "  Enable LTO: ${ENABLE_LTO}")
message(STATUS "  Enable profiling: ${ENABLE_PROFILING}")
message(STATUS "  Static build: ${BUILD_STATIC}")
if(CUDA_FOUND)
    message(STATUS "  CUDA support: YES")
endif()
if(OpenCL_FOUND)
    message(STATUS "  OpenCL support: YES")
endif()
if(Python3_FOUND)
    message(STATUS "  Python bindings: YES (${Python3_VERSION})")
endif()
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")