cmake_minimum_required(VERSION 3.16)
project(grtcserver)

add_definitions("-g -pipe -W -Wall -fPIC -std=gnu++14 -DWEBRTC_POSIX
    -DWEBRTC_LINUX")
include_directories(
    "./src"
    "./third_party/include"
    "../rtcbase/src"
   
)

link_directories(
    "./third_party/lib"
    "../rtcbase/out"
)

file(GLOB all_src
    "./src/*.cpp"
    "./src/server/*.cpp"
    "./src/base/*.cpp"
)


add_executable(grtcserver ${all_src})

target_link_libraries(grtcserver libyaml-cpp.a librtcbase.a -lpthread
    libabsl_strings.a libabsl_throw_delegate.a libev.a libjsoncpp.a
)