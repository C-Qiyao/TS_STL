cmake_minimum_required(VERSION 3.15)
project(TS_STL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)
endif()

# 包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 创建测试可执行文件 - 基础测试
add_executable(test_thread_safe_vector test/test_thread_safe_vector.cpp)
target_compile_features(test_thread_safe_vector PRIVATE cxx_std_17)

# 创建高级功能测试可执行文件
add_executable(test_advanced_features test/test_advanced_features.cpp)
target_compile_features(test_advanced_features PRIVATE cxx_std_17)

# 创建 List 测试可执行文件
add_executable(test_thread_safe_list test/test_thread_safe_list.cpp)
target_compile_features(test_thread_safe_list PRIVATE cxx_std_17)

# 创建示例可执行文件
add_executable(example_usage examples/example_usage.cpp)
target_compile_features(example_usage PRIVATE cxx_std_17)

# 创建性能基准测试可执行文件
add_executable(performance_benchmark examples/performance_benchmark.cpp)
target_compile_features(performance_benchmark PRIVATE cxx_std_17)

# 创建 Map 示例可执行文件
add_executable(example_map_usage examples/example_map_usage.cpp)
target_compile_features(example_map_usage PRIVATE cxx_std_17)

# 需要线程库
find_package(Threads REQUIRED)
target_link_libraries(test_thread_safe_vector PRIVATE Threads::Threads)
target_link_libraries(test_advanced_features PRIVATE Threads::Threads)
target_link_libraries(test_thread_safe_list PRIVATE Threads::Threads)
target_link_libraries(example_usage PRIVATE Threads::Threads)
target_link_libraries(performance_benchmark PRIVATE Threads::Threads)
target_link_libraries(example_map_usage PRIVATE Threads::Threads)

# 添加测试
enable_testing()
add_test(NAME ThreadSafeVectorTests COMMAND test_thread_safe_vector)
add_test(NAME AdvancedFeaturesTests COMMAND test_advanced_features)
add_test(NAME ThreadSafeListTests COMMAND test_thread_safe_list)
