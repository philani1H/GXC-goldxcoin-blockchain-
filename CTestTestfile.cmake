# CMake generated Testfile for 
# Source directory: /workspace
# Build directory: /workspace
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(blockchain_tests "/workspace/gxc-tests" "--gtest_filter=Blockchain*")
set_tests_properties(blockchain_tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;625;add_test;/workspace/CMakeLists.txt;0;")
add_test(transaction_tests "/workspace/gxc-tests" "--gtest_filter=Transaction*")
set_tests_properties(transaction_tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;626;add_test;/workspace/CMakeLists.txt;0;")
add_test(wallet_tests "/workspace/gxc-tests" "--gtest_filter=Wallet*")
set_tests_properties(wallet_tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;627;add_test;/workspace/CMakeLists.txt;0;")
add_test(mining_tests "/workspace/gxc-tests" "--gtest_filter=Mining*")
set_tests_properties(mining_tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;628;add_test;/workspace/CMakeLists.txt;0;")
add_test(staking_tests "/workspace/gxc-tests" "--gtest_filter=Staking*")
set_tests_properties(staking_tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;629;add_test;/workspace/CMakeLists.txt;0;")
add_test(network_tests "/workspace/gxc-tests" "--gtest_filter=Network*")
set_tests_properties(network_tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;630;add_test;/workspace/CMakeLists.txt;0;")
add_test(api_tests "/workspace/gxc-tests" "--gtest_filter=API*")
set_tests_properties(api_tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;631;add_test;/workspace/CMakeLists.txt;0;")
add_test(governance_tests "/workspace/gxc-tests" "--gtest_filter=Governance*")
set_tests_properties(governance_tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;632;add_test;/workspace/CMakeLists.txt;0;")
add_test(token_tests "/workspace/gxc-tests" "--gtest_filter=Token*")
set_tests_properties(token_tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;633;add_test;/workspace/CMakeLists.txt;0;")
add_test(crypto_tests "/workspace/gxc-tests" "--gtest_filter=Crypto*")
set_tests_properties(crypto_tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/CMakeLists.txt;634;add_test;/workspace/CMakeLists.txt;0;")
