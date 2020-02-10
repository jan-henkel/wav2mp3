add_executable(wav2mp3 main.cpp convert.cpp memory_layout.cpp util.cpp wav.cpp)
target_link_libraries(wav2mp3 ${PROJECT_SOURCE_DIR}/lib/windows/libmp3lame.a)