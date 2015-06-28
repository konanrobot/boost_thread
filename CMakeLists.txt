

# Version required
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

# Project Name
PROJECT( BOOST_THREAD_YEL )

# Basic Settings
SET(CMAKE_CXX_COMPILER "g++")
# c++编译器参数
#CMAKE_CXX_FLAGS_DEBUG debug版本对应的编译器参数
#CMAKE_CXX_FLAGS_RELEASE release版本对应的编译器参数
SET(CMAKE_CXX_FLAGS "-Wall -std=c++11")
#PROJECT_SOURCE_DIR:工程的源文件目录，通常是包含CMakeLists.txt（有Project命令的）的目录
MESSAGE(STATUS "This is SOURCE dir "${CMAKE_YEL_SOURCE_DIR})
MESSAGE(STATUS "This is BINARY dir " ${CMAKE_YEL_BINARY_DIR})
#EXECUTABLE_OUTPUT_PATH:可执行文件的输出目录
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)



# Debug Settings控制 Debug 和 Release 模式的构建
SET( CMAKE_BUILD_TYPE Debug )

# Include directories包含目录
#将./include加入包含目录列表
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/include )






# Add new CMake Modules
#LIST为列表操作
#LIST( APPEND|INSERT|LENGTH|GET| REMOVE_ITEM|REMOVE_AT|SORT ...)
#cmake 为上百个软件包提供了查找器(finder):FindXXXX.cmake
#当使用非cmake自带的finder时，需要指定finder的路径，这就是CMAKE_MODULE_PATH，配合 FIND_PACKAGE()使用

MESSAGE(STATUS "PROJECT_SOURCE_DIR is dir: " ${PROJECT_SOURCE_DIR})
#这个指令用于向当前工程添加存放源文件的子目录，并可以指定中间二进制和目标二进制存放的位置。
ADD_SUBDIRECTORY(src bin)








#INCLUDE_DIRECTORIES(${includedir}) #相当与gcc的-I。
#LINK_DIRECTORIES(${libdir}) #相当与gcc的-L
#TARGET_LINK_LIBRARIES(helloworld ${linkflags}) #相当与gcc的-l
#ADD_DEFINITIONS(${cflags}) #相当与gcc的-D，向C/CPP添加宏定义
#ADD_DEPENDICIES(target_name, depend_name)：定义target对其他target的依赖关系
