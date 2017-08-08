cmake_minimum_required(VERSION 3.4.1)


#ffmpeg的库文件所在位置
set(lib_DIR ../../../../src/main/jniLibs/${ANDROID_ABI})
#fdk-aac根目录
set(fdk_aac_dir src/main/jniLibs/fdk-aac-0.1.5)
#video的目录下面的文件夹
set(video_dir src/main/cpp/video)
set(video_encode_dir ${video_dir}/encode)
set(video_format_dir ${video_dir}/videoutils)
set(video_effect_dir ${video_dir}/effect)
set(video_common_dir ${video_dir}/common)
set(video_media_dir ${video_dir}/media)

#include目录，也就是头文件目录
include_directories(
        src/main/jniLibs/include
        ${video_encode_dir}/include
        ${video_format_dir}/include
        ${video_effect_dir}/include
        ${video_common_dir}/include
        ${video_media_dir}/include
)

#导入so文件

add_library(libmbffmpeg SHARED IMPORTED)
set_target_properties( libmbffmpeg
                       PROPERTIES IMPORTED_LOCATION
                       ${lib_DIR}/libsgffmpeg.so )
add_library(libyuv SHARED IMPORTED)
set_target_properties( libyuv
                       PROPERTIES IMPORTED_LOCATION
                       ${lib_DIR}/libyuv.so )
add_library(libsoundtouch SHARED IMPORTED)
set_target_properties( libsoundtouch
                       PROPERTIES IMPORTED_LOCATION
                       ${lib_DIR}/libsoundtouch.so )

 file(GLOB video_encode_dir_src ${video_encode_dir}/*.cpp)
 file(GLOB video_format_dir_src ${video_format_dir}/*.cpp)
 file(GLOB video_effect_dir_src ${video_effect_dir}/*.cpp)
 file(GLOB video_common_dir_src ${video_common_dir}/*.cpp)
 file(GLOB video_media_dir_src ${video_media_dir}/*.cpp)
 add_library(libframework SHARED
             ${video_encode_dir_src}
             ${video_format_dir_src}
             ${video_effect_dir_src}
             ${video_common_dir_src}
             ${video_media_dir_src}
             )


 #支持-std=gnu++11
 #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

 #判断编译器类型,如果是gcc编译器,则在编译选项中加入c++11支持
 #if(CMAKE_COMPILER_IS_GNUCXX)
 #   set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
 #    message(STATUS "optional:-std=c++11")
 #endif(CMAKE_COMPILER_IS_GNUCXX)

 find_library(ndk-lib
              log
              )

 target_link_libraries(libframework
                      libmbffmpeg
                      libyuv
                      libsoundtouch
                      android
 					 ${ndk-lib}
 					  )
