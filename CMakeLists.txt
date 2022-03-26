cmake_minimum_required(VERSION 3.20)
project(video_finder)

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_SKIP_BUILD_RPATH TRUE)

#set(CMAKE_BUILD_TYPE debug)
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
include_directories(/opt/homebrew/Cellar/ffmpeg/4.4.1_4/include /opt/homebrew/Cellar/jpeg/9d/include /opt/homebrew/Cellar/libpng/1.6.37/include)
include_directories(include  /Users/weiyanguo/miniforge3/envs/default-dev-py39/include/curl)

link_directories(/opt/homebrew/Cellar/ffmpeg/4.4.1_4/lib  /Users/weiyanguo/miniforge3/envs/default-dev-py39/lib)

set(ffmpeg_prefix /opt/homebrew/Cellar/ffmpeg/4.4.1_4)
set(png_prefix /opt/homebrew/Cellar/libpng/1.6.37)
set(jpeg_prefix /opt/homebrew/Cellar/jpeg/9d)
set(curl_prefix /Users/weiyanguo/miniforge3/envs/default-dev-py39)


add_library(libjpeg STATIC IMPORTED)
set_target_properties(libjpeg PROPERTIES IMPORTED_LOCATION ${jpeg_prefix}/lib/libjpeg.a)
set_target_properties(libjpeg PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${jpeg_prefix}/include)

add_library(libpng STATIC IMPORTED)
set_target_properties(libpng PROPERTIES IMPORTED_LOCATION ${png_prefix}/lib/libpng.a)
set_target_properties(libpng PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${png_prefix}/include)


#add_executable(cimg_test src/cimg_test.cpp)
#target_include_directories(cimg_test PRIVATE jpeg-9d/install/include)
#target_link_libraries(cimg_test -lX11 libjpeg.a)

#add_executable(phash_test src/phash_test.cpp)
#target_link_libraries(phash_test libjpeg.a)


add_executable(compute_hash src/compute_hash.cpp)
target_link_libraries(compute_hash libjpeg libavformat.a libavcodec.a libavutil.a libswscale.a )

add_executable(finder src/finder.cpp)
target_link_libraries(finder libjpeg libpng libavformat.a libavcodec.a libavutil.a libswscale.a libavresample.a  -lcurl -lz)

#add_executable(io_test src/io_test.cpp)
#target_link_libraries(io_test libjpeg.a libavformat.a libavutil.a libswscale.a libavcodec.a)

add_executable(merge_datasets src/merge_datasets.cpp)
