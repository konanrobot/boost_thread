# Empyt Project Template
# YAO=liang
#src的CMakeLists
# Version required检查CMake版本
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

# Project Name定义工程名称
#PROJECT( CMAKE_YEL )

# Source directories包含子目录
#ADD_SUBDIRECTORY( ${PROJECT_SOURCE_DIR}/src )
#MESSAGE(STATUS "This is SOURCE dir "${CMAKE_YEL_SOURCE_DIR})
#ADD_LIBRARY:源文件列表里的文件生成一个动/静态链接库libSLAM3d
#ADD_LIBRARY(SLAM3d GraphicEnd.cpp ParameterReader.cpp GraphicEnd2.cpp)
#将若干库文件链接到生成的目标SLAM3d(libSLAM3d.a或libSLAM3d.so)
#TARGET_LINK_LIBRARIES( SLAM3d
#  ${OpenCV_LIBS} g2o_core pthread ${PCL_LIBRARIES} g2o_types_slam3d g2o_solver_csparse g2o_stuff g2o_csparse_extension ${CSPARSE_LIBRARY} yaml-cpp)

#编译可执行程序，指定编译
#源文件列表里的文件/run_SLAM.cpp生成一个可执行文件run_SLAM
#ADD_EXECUTABLE( run_SLAM run_SLAM.cpp )
#ADD_EXECUTABLE(hello ${CMAKE_YEL_SOURCE_DIR})

ADD_EXECUTABLE(thread boost_thread.cpp)
#以下非常重要！！！！！！！！！！！！！！！！！！！！！！！！！！！
TARGET_LINK_LIBRARIES(thread  boost_system  boost_thread boost_timer)


