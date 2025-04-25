cmake_minimum_required(VERSION 3.10)
project(GXC_Blockchain)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenSSL package
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add executable
add_executable(gxc_blockchain src/main.cpp)

# Link OpenSSL libraries
target_link_libraries(gxc_blockchain OpenSSL::SSL OpenSSL::Crypto)