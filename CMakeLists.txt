# CMakeLists.txt 文件
cmake_minimum_required(VERSION 3.10)

project(TicketSystem)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 为 MyProject 目标添加包含目录
add_executable(code src/bpt/BPT.cpp)

# 指定 STL 头文件所在的目录
target_include_directories(TicketSystem PRIVATE ${PROJECT_SOURCE_DIR}/src/stl)
