cmake_minimum_required(VERSION 3.4.1)

#jni的文件目录
set(jni_dir src/main/cpp/jni)
#video的文件目录
set(video_dir src/main/cpp/video)
#video的so文件目录
set(video_so_dir ../../../../src/main/cpp/video/so)
#fdkaac的文件目录
set(fdk_aac_dir ${video_so_dir}/dk-aac-0.1.5)

include_directories(
        ${jni_dir}/include
        ${video_dir}/include
        ${video_dir}/so/include
        ${video_dir}/so/include/libfdkaac

)
#导入so文件

add_library(libffmpeg SHARED IMPORTED)
set_target_properties( libffmpeg
                       PROPERTIES IMPORTED_LOCATION
                       ${video_so_dir}/armeabi-v7a/libffmpeg.so )

add_library(libfdkaac SHARED IMPORTED)
set_target_properties( libfdkaac
                       PROPERTIES IMPORTED_LOCATION
                       ${video_so_dir}/armeabi-v7a/libfdkaac.so )

file(GLOB jni_dir_src ${jni_dir}/*.cpp)
file(GLOB video_dir_src ${video_dir}/*.cpp)

add_library(libframework SHARED
            ${jni_dir_src}
            ${video_dir_src}
            )
find_library(ndk-lib
             log
             )

target_link_libraries(libframework
                     libffmpeg
                     libfdkaac
                     android
					 ${ndk-lib}
					  )
