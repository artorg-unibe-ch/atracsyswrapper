PROJECT(AtracsysWrapperTest)

cmake_minimum_required(VERSION 3.0)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

ADD_EXECUTABLE(atracsyswrapper_test   main.cpp)
TARGET_LINK_LIBRARIES(atracsyswrapper_test CONAN_PKG::AtracsysWrapper)
