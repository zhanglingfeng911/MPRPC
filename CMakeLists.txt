cmake_minimum_required(VERSION 3.0)#cmake版本要求
project(mprpc)#项目名字
set(CMKAE_BUILD_TYPE "Debug")#生成debug版本 进行gdb调试
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)#设置可执行文件的输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)#设置项目库文件的输出路径
include_directories(${PROJECT_SOURCE_DIR}/src/include)#设置头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/example)
link_directories(${PROJECT_SOURCE_DIR}/lib)#编译可执行文件时库文件的搜索路径
add_subdirectory(src)#进入子目录下执行CMakeLists文件 （src包含了框架所有的相关代码)
add_subdirectory(example)#(包含了example使用mprpc框架的示例代码)