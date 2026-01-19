# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(TicketSystem)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置 CMake 输出编译命令文件（供IDE/工具使用）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/container
    ${PROJECT_SOURCE_DIR}/include/System
    ${PROJECT_SOURCE_DIR}/include/memoryriver
)

# 设置源文件
set(SOURCE_FILES
    src/system/System.cpp
    src/system/TrainSystem.cpp
    src/system/TicketSystem.cpp
    src/system/UserSystem.cpp
    src/parser.cpp
    src/main.cpp
)

# 添加可执行文件
add_executable(code ${SOURCE_FILES})

# 为目标 "code" 设置包含目录
target_include_directories(code PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/container
    ${PROJECT_SOURCE_DIR}/include/System
    ${PROJECT_SOURCE_DIR}/include/memoryriver
)

# 如果有需要链接的库，可以在这里添加
# target_link_libraries(code PRIVATE some_library)
