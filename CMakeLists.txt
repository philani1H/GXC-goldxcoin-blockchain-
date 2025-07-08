cmake_minimum_required(VERSION 3.10)
project(GXC_Blockchain)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
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
)

# Header files (for IDE support)
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
    include/Governance.h
    include/GoldToken.h
    include/StockContract.h
)

# Add executable
add_executable(gxc_blockchain ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(gxc_blockchain 
    OpenSSL::SSL 
    OpenSSL::Crypto
    Threads::Threads
)

# Compiler flags
target_compile_options(gxc_blockchain PRIVATE 
    -Wall 
    -Wextra 
    -O2
)

# Enable debug symbols in debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(gxc_blockchain PRIVATE -g)
endif()

# Create static library for the blockchain core (for testing)
add_library(gxc_core STATIC
    src/Blockchain.cpp
    src/block.cpp
    src/Transaction.cpp
    src/Wallet.cpp
    src/Validator.cpp
    src/SalectValidator.cpp
    src/ProofOfPrice.cpp
    src/CrossChainBridge.cpp
    src/HashUtils.cpp
)

target_link_libraries(gxc_core
    OpenSSL::SSL 
    OpenSSL::Crypto
)

# Install targets
install(TARGETS gxc_blockchain
    RUNTIME DESTINATION bin
)

install(TARGETS gxc_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS}
    DESTINATION include/gxc
)