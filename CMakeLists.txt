cmake_minimum_required(VERSION 3.15.0)
project(micstate LANGUAGES CXX)

add_executable(micstate WIN32 micstate.cpp)
target_include_directories(micstate PRIVATE C:/dev/include)
target_link_directories(micstate PRIVATE C:/dev/lib)
target_link_libraries(micstate PRIVATE Mmdevapi Tolk)

file(COPY "c:/dev/bin/Tolk.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release)
file(COPY "c:/dev/bin/SAAPI64.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release)
file(COPY "c:/dev/bin/nvdaControllerClient64.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release)