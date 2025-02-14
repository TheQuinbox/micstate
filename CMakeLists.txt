cmake_minimum_required(VERSION 3.20)
project(micstate LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)

add_executable(micstate WIN32 micstate.c)
target_include_directories(micstate PRIVATE include)
target_link_directories(micstate PRIVATE lib)
target_link_libraries(micstate PRIVATE Bass UniversalSpeechStatic Kernel32 Version Psapi)

file(COPY "bin/bass.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release)
file(COPY "bin/nvdaControllerClient64.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release)
file(COPY "bin/SAAPI64.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release)
file(COPY "README.md" DESTINATION ${CMAKE_BINARY_DIR}/Release)
file(COPY "bin/zdsrapi.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release)
