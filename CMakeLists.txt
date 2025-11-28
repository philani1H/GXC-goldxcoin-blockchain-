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
        message(STATUS "Link Time Optimization enabled")
    else()
        message(WARNING "LTO not supported: ${lto_error}")
    endif()
endif()

# Custom OpenSSL finder function
function(find_openssl_custom)
    # Windows-specific OpenSSL search
    if(WIN32)
        # Common OpenSSL installation paths on Windows
        # Common OpenSSL installation paths on Windows
        # Note: Can't use $ENV{ProgramFiles(x86)} due to parentheses in variable name
        # Using hardcoded paths and ProgramFiles env var (without x86)
        set(OPENSSL_SEARCH_PATHS
            "C:/Program Files/OpenSSL-Win64"
            "C:/Program Files (x86)/OpenSSL-Win64"
            "C:/OpenSSL-Win64"
            "C:/OpenSSL"
        )
        
        # Add environment variable paths if they exist
        if(DEFINED ENV{OPENSSL_ROOT_DIR})
            list(APPEND OPENSSL_SEARCH_PATHS "$ENV{OPENSSL_ROOT_DIR}")
        endif()
        
        if(DEFINED ENV{ProgramFiles})
            list(APPEND OPENSSL_SEARCH_PATHS "$ENV{ProgramFiles}/OpenSSL-Win64")
        endif()
        
        # ProgramFiles(x86) - use hardcoded path since env var has parentheses
        # Already included in hardcoded paths above
        
        # Initialize found flags (local to function)
        set(OPENSSL_FOUND_INCLUDE_LOCAL FALSE)
        set(OPENSSL_FOUND_CRYPTO_LOCAL FALSE)
        set(OPENSSL_FOUND_SSL_LOCAL FALSE)
        
        foreach(SEARCH_PATH ${OPENSSL_SEARCH_PATHS})
            if(EXISTS "${SEARCH_PATH}/include/openssl/ssl.h")
                set(OPENSSL_INCLUDE_DIR "${SEARCH_PATH}/include" PARENT_SCOPE)
                set(OPENSSL_FOUND_INCLUDE_LOCAL TRUE)
                message(STATUS "Found OpenSSL includes at: ${SEARCH_PATH}/include")
                
                # Look for libraries in various subdirectories
                set(LIB_SEARCH_PATHS
                    "${SEARCH_PATH}/lib"
                    "${SEARCH_PATH}/lib/VC"
                    "${SEARCH_PATH}/lib/VC/x64/MD"
                    "${SEARCH_PATH}/lib/VC/x86/MD"
                    "${SEARCH_PATH}/lib/VC/static"
                    "${SEARCH_PATH}/out32"
                    "${SEARCH_PATH}/out32dll"
                )
                
                foreach(LIB_PATH ${LIB_SEARCH_PATHS})
                    # Check for crypto library
                    if(NOT OPENSSL_FOUND_CRYPTO_LOCAL)
                        if(EXISTS "${LIB_PATH}/libcrypto.lib")
                            set(OPENSSL_CRYPTO_LIBRARY "${LIB_PATH}/libcrypto.lib" PARENT_SCOPE)
                            set(OPENSSL_FOUND_CRYPTO_LOCAL TRUE)
                            message(STATUS "Found OpenSSL crypto library: ${LIB_PATH}/libcrypto.lib")
                        elseif(EXISTS "${LIB_PATH}/libeay32.lib")
                            set(OPENSSL_CRYPTO_LIBRARY "${LIB_PATH}/libeay32.lib" PARENT_SCOPE)
                            set(OPENSSL_FOUND_CRYPTO_LOCAL TRUE)
                            message(STATUS "Found OpenSSL crypto library: ${LIB_PATH}/libeay32.lib")
                        endif()
                    endif()
                    
                    # Check for SSL library
                    if(NOT OPENSSL_FOUND_SSL_LOCAL)
                        if(EXISTS "${LIB_PATH}/libssl.lib")
                            set(OPENSSL_SSL_LIBRARY "${LIB_PATH}/libssl.lib" PARENT_SCOPE)
                            set(OPENSSL_FOUND_SSL_LOCAL TRUE)
                            message(STATUS "Found OpenSSL SSL library: ${LIB_PATH}/libssl.lib")
                        elseif(EXISTS "${LIB_PATH}/ssleay32.lib")
                            set(OPENSSL_SSL_LIBRARY "${LIB_PATH}/ssleay32.lib" PARENT_SCOPE)
                            set(OPENSSL_FOUND_SSL_LOCAL TRUE)
                            message(STATUS "Found OpenSSL SSL library: ${LIB_PATH}/ssleay32.lib")
                        endif()
                    endif()
                    
                    if(OPENSSL_FOUND_CRYPTO_LOCAL AND OPENSSL_FOUND_SSL_LOCAL)
                        message(STATUS "Found OpenSSL libraries at: ${LIB_PATH}")
                        break()
                    endif()
                endforeach()
                
                if(OPENSSL_FOUND_INCLUDE_LOCAL AND OPENSSL_FOUND_CRYPTO_LOCAL AND OPENSSL_FOUND_SSL_LOCAL)
                    break()
                endif()
            endif()
        endforeach()
        
        # Set flags for later check
        set(OPENSSL_FOUND_INCLUDE ${OPENSSL_FOUND_INCLUDE_LOCAL} PARENT_SCOPE)
        set(OPENSSL_FOUND_CRYPTO ${OPENSSL_FOUND_CRYPTO_LOCAL} PARENT_SCOPE)
        set(OPENSSL_FOUND_SSL ${OPENSSL_FOUND_SSL_LOCAL} PARENT_SCOPE)
    endif()
    
    # Unix-like systems (Linux, macOS)
    if(UNIX)
        # Standard system paths
        find_path(OPENSSL_INCLUDE_DIR
            NAMES openssl/ssl.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
                /usr/local/ssl/include
                /usr/local/opt/openssl/include
                /opt/homebrew/include
                /sw/include
        )
        
        find_library(OPENSSL_CRYPTO_LIBRARY
            NAMES crypto libcrypto
            PATHS
                /usr/lib
                /usr/local/lib
                /opt/local/lib
                /usr/local/ssl/lib
                /usr/local/opt/openssl/lib
                /opt/homebrew/lib
                /sw/lib
                /usr/lib/x86_64-linux-gnu
                /usr/lib/i386-linux-gnu
        )
        
        find_library(OPENSSL_SSL_LIBRARY
            NAMES ssl libssl
            PATHS
                /usr/lib
                /usr/local/lib
                /opt/local/lib
                /usr/local/ssl/lib
                /usr/local/opt/openssl/lib
                /opt/homebrew/lib
                /sw/lib
                /usr/lib/x86_64-linux-gnu
                /usr/lib/i386-linux-gnu
        )
    endif()
    
    # Verify all components found (check both Windows and Unix paths)
    set(OPENSSL_ALL_FOUND FALSE)
    
    if(WIN32)
        # For Windows, check if we found all components
        # Check the local variables that were set in the loop above
        if(OPENSSL_FOUND_INCLUDE_LOCAL AND OPENSSL_FOUND_CRYPTO_LOCAL AND OPENSSL_FOUND_SSL_LOCAL)
            set(OPENSSL_ALL_FOUND TRUE)
            message(STATUS "OpenSSL verification: All components found ✓")
        else()
            message(STATUS "OpenSSL check: INCLUDE=${OPENSSL_FOUND_INCLUDE_LOCAL}, CRYPTO=${OPENSSL_FOUND_CRYPTO_LOCAL}, SSL=${OPENSSL_FOUND_SSL_LOCAL}")
        endif()
    else()
        # For Unix, check if find_path and find_library succeeded
        if(OPENSSL_INCLUDE_DIR AND OPENSSL_CRYPTO_LIBRARY AND OPENSSL_SSL_LIBRARY)
            set(OPENSSL_ALL_FOUND TRUE)
        endif()
    endif()
    
    if(OPENSSL_ALL_FOUND)
        # Set the variables that find_package(OpenSSL) expects
        set(OPENSSL_FOUND TRUE PARENT_SCOPE)
        
        # Ensure all variables are set to parent scope
        if(WIN32)
            # Variables already set to PARENT_SCOPE in Windows section
            # Just verify they're set
            get_property(INCLUDE_DIR_SET DIRECTORY PROPERTY OPENSSL_INCLUDE_DIR)
            if(NOT INCLUDE_DIR_SET)
                set(OPENSSL_INCLUDE_DIR "${OPENSSL_INCLUDE_DIR}" PARENT_SCOPE)
            endif()
        else()
            set(OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR} PARENT_SCOPE)
            set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_CRYPTO_LIBRARY} PARENT_SCOPE)
            set(OPENSSL_SSL_LIBRARY ${OPENSSL_SSL_LIBRARY} PARENT_SCOPE)
        endif()
        
        set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY} PARENT_SCOPE)
        
        # Create imported targets
        if(NOT TARGET OpenSSL::Crypto)
            add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
            set_target_properties(OpenSSL::Crypto PROPERTIES
                IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
            )
        endif()
        
        if(NOT TARGET OpenSSL::SSL)
            add_library(OpenSSL::SSL UNKNOWN IMPORTED)
            set_target_properties(OpenSSL::SSL PROPERTIES
                IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
                INTERFACE_LINK_LIBRARIES OpenSSL::Crypto
            )
        endif()
        
        # Windows-specific linking requirements
        if(WIN32)
            # Additional Windows-specific setup if needed
        endif()
    else()
        # OpenSSL not found - set to false
        set(OPENSSL_FOUND FALSE PARENT_SCOPE)
        message(WARNING "OpenSSL not found. Some features may be unavailable.")
    endif()
endfunction()

# Call the custom OpenSSL finder
find_openssl_custom()

# Check if OpenSSL is required
if(NOT OPENSSL_FOUND)
    message("")
    message("==========================================")
    message("OpenSSL not found!")
    message("==========================================")
    message("Please install OpenSSL development libraries:")
    message("")
    message("Windows:")
    message("  1. Download from: https://slproweb.com/products/Win32OpenSSL.html")
    message("  2. Install to default location (C:/Program Files/OpenSSL-Win64)")
    message("  3. Or set OPENSSL_ROOT_DIR environment variable to OpenSSL path")
    message("")
    message("Linux (Ubuntu/Debian):")
    message("  sudo apt-get install libssl-dev")
    message("")
    message("Linux (RHEL/CentOS):")
    message("  sudo yum install openssl-devel")
    message("")
    message("macOS:")
    message("  brew install openssl")
    message("  export OPENSSL_ROOT_DIR=/usr/local/opt/openssl")
    message("==========================================")
    message("")
    message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL development libraries.")
endif()

# GUI Framework
if(NOT Qt6_FOUND)
    find_package(Qt5 QUIET COMPONENTS Core Widgets Charts Network WebSockets)
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
        if(EXISTS ${THIRD_PARTY_DIR}/json)
            add_subdirectory(${THIRD_PARTY_DIR}/json EXCLUDE_FROM_ALL)
        else()
            message(WARNING "nlohmann_json not found. Some features may be unavailable.")
        endif()
    endif()
endif()

# Crypto++ library
find_package(cryptopp QUIET)
if(NOT cryptopp_FOUND)
    if(EXISTS ${THIRD_PARTY_DIR}/cryptopp)
        add_subdirectory(${THIRD_PARTY_DIR}/cryptopp EXCLUDE_FROM_ALL)
    endif()
endif()

# LevelDB for blockchain storage
find_package(leveldb QUIET)
if(NOT leveldb_FOUND)
    if(EXISTS ${THIRD_PARTY_DIR}/leveldb)
        add_subdirectory(${THIRD_PARTY_DIR}/leveldb EXCLUDE_FROM_ALL)
    endif()
endif()

# SQLite for wallet storage - custom finder
function(find_sqlite3_custom)
    # Windows-specific SQLite3 search
    if(WIN32)
        # Common SQLite3 installation paths on Windows
        set(SQLITE3_SEARCH_PATHS
            "C:/Program Files/SQLite"
            "C:/Program Files (x86)/SQLite"
            "C:/SQLite"
            "C:/sqlite3"
            "${CMAKE_SOURCE_DIR}/third_party/sqlite3"
        )
        
        # Add environment variable paths if they exist
        if(DEFINED ENV{SQLITE3_ROOT_DIR})
            list(APPEND SQLITE3_SEARCH_PATHS "$ENV{SQLITE3_ROOT_DIR}")
        endif()
        
        if(DEFINED ENV{ProgramFiles})
            list(APPEND SQLITE3_SEARCH_PATHS "$ENV{ProgramFiles}/SQLite")
        endif()
        
        # Initialize found flags (local to function)
        set(SQLITE3_FOUND_INCLUDE_LOCAL FALSE)
        set(SQLITE3_FOUND_LIBRARY_LOCAL FALSE)
        
        foreach(SEARCH_PATH ${SQLITE3_SEARCH_PATHS})
            if(EXISTS "${SEARCH_PATH}/include/sqlite3.h" OR EXISTS "${SEARCH_PATH}/sqlite3.h")
                if(EXISTS "${SEARCH_PATH}/include/sqlite3.h")
                    set(SQLITE3_INCLUDE_DIR "${SEARCH_PATH}/include" PARENT_SCOPE)
                else()
                    set(SQLITE3_INCLUDE_DIR "${SEARCH_PATH}" PARENT_SCOPE)
                endif()
                set(SQLITE3_FOUND_INCLUDE_LOCAL TRUE)
                message(STATUS "Found SQLite3 includes at: ${SQLITE3_INCLUDE_DIR}")
                
                # Look for libraries in various subdirectories
                set(LIB_SEARCH_PATHS
                    "${SEARCH_PATH}/lib"
                    "${SEARCH_PATH}/lib/x64"
                    "${SEARCH_PATH}/lib/x86"
                    "${SEARCH_PATH}/lib/VC/x64"
                    "${SEARCH_PATH}/lib/VC/x86"
                )
                
                foreach(LIB_PATH ${LIB_SEARCH_PATHS})
                    if(NOT SQLITE3_FOUND_LIBRARY_LOCAL)
                        if(EXISTS "${LIB_PATH}/sqlite3.lib")
                            set(SQLITE3_LIBRARY "${LIB_PATH}/sqlite3.lib" PARENT_SCOPE)
                            set(SQLITE3_FOUND_LIBRARY_LOCAL TRUE)
                            message(STATUS "Found SQLite3 library: ${LIB_PATH}/sqlite3.lib")
                        elseif(EXISTS "${LIB_PATH}/sqlite3d.lib")
                            set(SQLITE3_LIBRARY "${LIB_PATH}/sqlite3d.lib" PARENT_SCOPE)
                            set(SQLITE3_FOUND_LIBRARY_LOCAL TRUE)
                            message(STATUS "Found SQLite3 library: ${LIB_PATH}/sqlite3d.lib")
                        endif()
                    endif()
                    
                    if(SQLITE3_FOUND_LIBRARY_LOCAL)
                        break()
                    endif()
                endforeach()
                
                if(SQLITE3_FOUND_INCLUDE_LOCAL AND SQLITE3_FOUND_LIBRARY_LOCAL)
                    break()
                endif()
            endif()
        endforeach()
        
        # Also try system-wide search using find_path and find_library (expanded search)
        if(NOT SQLITE3_FOUND_INCLUDE_LOCAL)
            find_path(SQLITE3_INCLUDE_DIR
                NAMES sqlite3.h
                PATHS
                    "C:/Program Files/SQLite/include"
                    "C:/Program Files (x86)/SQLite/include"
                    "C:/SQLite/include"
                    "C:/SQLite"
                    "C:/sqlite3/include"
                    "C:/sqlite3"
                    "C:/Program Files/Common Files/SQLite/include"
                    "$ENV{ProgramFiles}/SQLite/include"
                    "$ENV{LOCALAPPDATA}/SQLite/include"
                    "$ENV{APPDATA}/SQLite/include"
                    "C:/tools/sqlite/include"
                    "C:/tools/sqlite3/include"
                PATH_SUFFIXES include
                NO_DEFAULT_PATH
            )
            if(NOT SQLITE3_INCLUDE_DIR)
                # Try without NO_DEFAULT_PATH to search system paths
                find_path(SQLITE3_INCLUDE_DIR NAMES sqlite3.h)
            endif()
            if(SQLITE3_INCLUDE_DIR)
                set(SQLITE3_FOUND_INCLUDE_LOCAL TRUE)
                set(SQLITE3_INCLUDE_DIR ${SQLITE3_INCLUDE_DIR} PARENT_SCOPE)
                message(STATUS "Found SQLite3 includes at: ${SQLITE3_INCLUDE_DIR}")
            endif()
        endif()
        
        if(NOT SQLITE3_FOUND_LIBRARY_LOCAL)
            find_library(SQLITE3_LIBRARY
                NAMES sqlite3 sqlite3d sqlite3.lib sqlite3d.lib
                PATHS
                    "C:/Program Files/SQLite/lib"
                    "C:/Program Files (x86)/SQLite/lib"
                    "C:/SQLite/lib"
                    "C:/SQLite"
                    "C:/sqlite3/lib"
                    "C:/sqlite3"
                    "C:/Program Files/Common Files/SQLite/lib"
                    "$ENV{ProgramFiles}/SQLite/lib"
                    "$ENV{LOCALAPPDATA}/SQLite/lib"
                    "$ENV{APPDATA}/SQLite/lib"
                    "C:/tools/sqlite/lib"
                    "C:/tools/sqlite3/lib"
                PATH_SUFFIXES lib lib/x64 lib/x86 lib/VC/x64 lib/VC/x86
                NO_DEFAULT_PATH
            )
            if(NOT SQLITE3_LIBRARY)
                # Try without NO_DEFAULT_PATH to search system paths
                find_library(SQLITE3_LIBRARY NAMES sqlite3 sqlite3d)
            endif()
            if(SQLITE3_LIBRARY)
                set(SQLITE3_FOUND_LIBRARY_LOCAL TRUE)
                set(SQLITE3_LIBRARY ${SQLITE3_LIBRARY} PARENT_SCOPE)
                message(STATUS "Found SQLite3 library: ${SQLITE3_LIBRARY}")
            endif()
        endif()
        
        # If we found library but not include, try to find include near library
        if(SQLITE3_FOUND_LIBRARY_LOCAL AND NOT SQLITE3_FOUND_INCLUDE_LOCAL)
            get_filename_component(SQLITE3_LIB_DIR ${SQLITE3_LIBRARY} DIRECTORY)
            get_filename_component(SQLITE3_LIB_PARENT ${SQLITE3_LIB_DIR} DIRECTORY)
            if(EXISTS "${SQLITE3_LIB_PARENT}/include/sqlite3.h")
                set(SQLITE3_INCLUDE_DIR "${SQLITE3_LIB_PARENT}/include" PARENT_SCOPE)
                set(SQLITE3_FOUND_INCLUDE_LOCAL TRUE)
                message(STATUS "Found SQLite3 includes at: ${SQLITE3_INCLUDE_DIR} (from library location)")
            elseif(EXISTS "${SQLITE3_LIB_DIR}/sqlite3.h")
                set(SQLITE3_INCLUDE_DIR ${SQLITE3_LIB_DIR} PARENT_SCOPE)
                set(SQLITE3_FOUND_INCLUDE_LOCAL TRUE)
                message(STATUS "Found SQLite3 includes at: ${SQLITE3_INCLUDE_DIR} (same as library)")
            endif()
        endif()
        
        # If still not found, check if we can use the amalgamation (single-file SQLite)
        if(NOT SQLITE3_FOUND_INCLUDE_LOCAL OR NOT SQLITE3_FOUND_LIBRARY_LOCAL)
            if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/sqlite3/sqlite3.h" AND 
               EXISTS "${CMAKE_SOURCE_DIR}/third_party/sqlite3/sqlite3.c")
                set(SQLITE3_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/sqlite3" PARENT_SCOPE)
                set(SQLITE3_FOUND_INCLUDE_LOCAL TRUE)
                set(SQLITE3_USE_AMALGAMATION TRUE PARENT_SCOPE)
                message(STATUS "Found SQLite3 amalgamation in third_party directory")
            endif()
        endif()
    else()
        # Unix-like systems (Linux, macOS) - use standard find_package
        find_path(SQLITE3_INCLUDE_DIR
            NAMES sqlite3.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/local/include
        )
        
        find_library(SQLITE3_LIBRARY
            NAMES sqlite3
            PATHS
                /usr/lib
                /usr/local/lib
                /opt/local/lib
                /usr/lib/x86_64-linux-gnu
                /usr/lib/i386-linux-gnu
        )
        
        if(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)
            set(SQLITE3_FOUND_INCLUDE_LOCAL TRUE)
            set(SQLITE3_FOUND_LIBRARY_LOCAL TRUE)
            set(SQLITE3_INCLUDE_DIR ${SQLITE3_INCLUDE_DIR} PARENT_SCOPE)
            set(SQLITE3_LIBRARY ${SQLITE3_LIBRARY} PARENT_SCOPE)
        endif()
    endif()
    
    # Verify all components found
    set(SQLITE3_ALL_FOUND FALSE)
    
    if(WIN32)
        # For Windows, check if we found all components (library OR amalgamation)
        if(SQLITE3_FOUND_INCLUDE_LOCAL AND (SQLITE3_FOUND_LIBRARY_LOCAL OR SQLITE3_USE_AMALGAMATION))
            set(SQLITE3_ALL_FOUND TRUE)
            message(STATUS "SQLite3 verification: All components found ✓")
        else()
            message(STATUS "SQLite3 check: INCLUDE=${SQLITE3_FOUND_INCLUDE_LOCAL}, LIBRARY=${SQLITE3_FOUND_LIBRARY_LOCAL}, AMALGAMATION=${SQLITE3_USE_AMALGAMATION}")
        endif()
    else()
        if(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)
            set(SQLITE3_ALL_FOUND TRUE)
        endif()
    endif()
    
    if(SQLITE3_ALL_FOUND)
        set(SQLITE3_FOUND TRUE PARENT_SCOPE)
        
        # Ensure all variables are set to parent scope
        if(WIN32)
            if(NOT SQLITE3_INCLUDE_DIR)
                message(WARNING "SQLITE3_INCLUDE_DIR not set despite finding SQLite3")
            endif()
            if(NOT SQLITE3_LIBRARY AND NOT SQLITE3_USE_AMALGAMATION)
                message(WARNING "SQLITE3_LIBRARY not set despite finding SQLite3")
            endif()
        else()
            set(SQLITE3_INCLUDE_DIR ${SQLITE3_INCLUDE_DIR} PARENT_SCOPE)
            set(SQLITE3_LIBRARY ${SQLITE3_LIBRARY} PARENT_SCOPE)
        endif()
        
        # Set amalgamation flag to parent scope if needed
        if(SQLITE3_USE_AMALGAMATION)
            set(SQLITE3_USE_AMALGAMATION TRUE PARENT_SCOPE)
        endif()
    else()
        set(SQLITE3_FOUND FALSE PARENT_SCOPE)
        message(WARNING "SQLite3 not found. Some features may be unavailable.")
    endif()
endfunction()

# Call the custom SQLite3 finder
find_sqlite3_custom()

# If SQLite3 not found, download and use amalgamation
if(NOT SQLITE3_FOUND)
    message(STATUS "SQLite3 not found in system. Downloading amalgamation...")
    
    set(SQLITE3_DIR "${CMAKE_SOURCE_DIR}/third_party/sqlite3")
    set(SQLITE3_AMALGAMATION_URL "https://www.sqlite.org/2024/sqlite-amalgamation-3490200.zip")
    set(SQLITE3_ZIP "${CMAKE_BINARY_DIR}/sqlite3.zip")
    
    # Create directory
    file(MAKE_DIRECTORY ${SQLITE3_DIR})
    
    # Check if we already have it
    if(NOT EXISTS "${SQLITE3_DIR}/sqlite3.h" OR NOT EXISTS "${SQLITE3_DIR}/sqlite3.c")
        message(STATUS "Downloading SQLite3 amalgamation from sqlite.org...")
        
        # Download using file(DOWNLOAD)
        file(DOWNLOAD 
            ${SQLITE3_AMALGAMATION_URL}
            ${SQLITE3_ZIP}
            SHOW_PROGRESS
            STATUS DOWNLOAD_STATUS
        )
        
        list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
        if(STATUS_CODE EQUAL 0)
            message(STATUS "Extracting SQLite3...")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E tar xzf ${SQLITE3_ZIP}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                RESULT_VARIABLE EXTRACT_RESULT
            )
            
            # Move files to third_party/sqlite3
            if(EXTRACT_RESULT EQUAL 0)
                file(GLOB SQLITE3_FILES "${CMAKE_BINARY_DIR}/sqlite-amalgamation-*/sqlite3.*")
                foreach(FILE ${SQLITE3_FILES})
                    get_filename_component(FILENAME ${FILE} NAME)
                    file(COPY ${FILE} DESTINATION ${SQLITE3_DIR})
                endforeach()
            endif()
        else()
            message(WARNING "Failed to download SQLite3. Will try to use existing files if available.")
        endif()
    endif()
    
    # Check if we have the files now
    if(EXISTS "${SQLITE3_DIR}/sqlite3.h" AND EXISTS "${SQLITE3_DIR}/sqlite3.c")
        set(SQLITE3_INCLUDE_DIR ${SQLITE3_DIR})
        set(SQLITE3_FOUND TRUE)
        set(SQLITE3_USE_AMALGAMATION TRUE)
        message(STATUS "Using SQLite3 amalgamation from ${SQLITE3_DIR}")
    else()
        # Last resort: provide clear manual instructions
        message("")
        message("==========================================")
        message("SQLite3 Auto-Download Failed")
        message("==========================================")
        message("Please manually download SQLite3 amalgamation:")
        message("")
        message("1. Go to: https://www.sqlite.org/download.html")
        message("2. Download: sqlite-amalgamation-*.zip")
        message("3. Extract sqlite3.h and sqlite3.c")
        message("4. Place them in: ${SQLITE3_DIR}/")
        message("")
        message("Or install SQLite3 system-wide:")
        message("  Windows: Extract to C:/SQLite")
        message("  Linux: sudo apt-get install libsqlite3-dev")
        message("  macOS: brew install sqlite")
        message("==========================================")
        message("")
        message(FATAL_ERROR "SQLite3 is required. Please download and install it manually.")
    endif()
endif()

# Create SQLite3 target (either imported or from source)
if(SQLITE3_USE_AMALGAMATION)
    # Compile SQLite3 from amalgamation source
    add_library(SQLite::SQLite3 STATIC
        ${SQLITE3_DIR}/sqlite3.c
    )
    target_include_directories(SQLite::SQLite3 PUBLIC ${SQLITE3_INCLUDE_DIR})
    set_target_properties(SQLite::SQLite3 PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
    message(STATUS "SQLite3 will be compiled from amalgamation source")
elseif(SQLITE3_FOUND)
    # Use pre-built library
    if(NOT TARGET SQLite::SQLite3)
        add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION "${SQLITE3_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_INCLUDE_DIR}"
        )
    endif()
endif()

# Create SQLite3 target (either imported or from source)
if(SQLITE3_USE_AMALGAMATION)
    # Compile SQLite3 from amalgamation source
    add_library(SQLite::SQLite3 STATIC
        ${SQLITE3_DIR}/sqlite3.c
    )
    target_include_directories(SQLite::SQLite3 PUBLIC ${SQLITE3_INCLUDE_DIR})
    set_target_properties(SQLite::SQLite3 PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
    message(STATUS "SQLite3 will be compiled from amalgamation source")
elseif(SQLITE3_FOUND)
    # Use pre-built library
    if(NOT TARGET SQLite::SQLite3)
        add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION "${SQLITE3_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_INCLUDE_DIR}"
        )
    endif()
endif()

# WebSocket library
find_package(websocketpp QUIET)
if(NOT websocketpp_FOUND)
    if(EXISTS ${THIRD_PARTY_DIR}/websocketpp)
        add_subdirectory(${THIRD_PARTY_DIR}/websocketpp EXCLUDE_FROM_ALL)
    endif()
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

# OpenCL support for GPU mining (optional - won't fail build if not found)
find_package(OpenCL QUIET)
if(OpenCL_FOUND AND BUILD_MINING_CLIENT)
    message(STATUS "OpenCL found - GPU mining support enabled")
    add_definitions(-DHAVE_OPENCL)
    if(TARGET OpenCL::OpenCL)
        target_link_libraries(gxc_mining OpenCL::OpenCL)
    else()
        # Fallback for older CMake versions
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
        if(EXISTS ${THIRD_PARTY_DIR}/googletest)
            add_subdirectory(${THIRD_PARTY_DIR}/googletest EXCLUDE_FROM_ALL)
            set(GTEST_LIBRARIES gtest gtest_main)
        else()
            message(WARNING "Google Test not found. Tests will not be built.")
            set(BUILD_TESTS OFF)
        endif()
    else()
        set(GTEST_LIBRARIES GTest::gtest GTest::gtest_main)
    endif()
    
    if(BUILD_TESTS)
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
    endif()
    
    # Performance benchmarks
    if(BUILD_BENCHMARKS)
        find_package(benchmark QUIET)
        if(NOT benchmark_FOUND)
            if(EXISTS ${THIRD_PARTY_DIR}/benchmark)
                add_subdirectory(${THIRD_PARTY_DIR}/benchmark EXCLUDE_FROM_ALL)
            else()
                message(WARNING "Google Benchmark not found. Benchmarks will not be built.")
                set(BUILD_BENCHMARKS OFF)
            endif()
        endif()
        
        if(BUILD_BENCHMARKS)
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
message(STATUS "  OpenSSL found: ${OPENSSL_FOUND}")
if(OPENSSL_FOUND)
    message(STATUS "  OpenSSL include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "  OpenSSL crypto: ${OPENSSL_CRYPTO_LIBRARY}")
    message(STATUS "  OpenSSL SSL: ${OPENSSL_SSL_LIBRARY}")
endif()
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