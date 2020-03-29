cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_SH "CMAKE_SH-NOTFOUND")
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

project(wav2mp3 VERSION 1.0.0)
add_subdirectory(src wav2mp3)
target_include_directories(wav2mp3 PUBLIC include)