cmake_minimum_required(VERSION 3.22)
project(Delay-Intervals)
set(CMAKE_CXX_STANDARD 20)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_BUILD_TYPE Release)
include(cmake/CPM.cmake)

CPMAddPackage(
    NAME JUCE
    GITHUB_REPOSITORY juce-framework/JUCE
    GIT_TAG 8.0.2
    VERSION 8.0.2
    SOURCE_DIR ${LIB_DIR}/juce
)

if (MSVC)
    add_compile_options(/Wall)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(plugin)

include (FetchContent)

FetchContent_Declare (melatonin_perfetto
  GIT_REPOSITORY https://github.com/sudara/melatonin_perfetto.git
  GIT_TAG origin/main)

FetchContent_MakeAvailable (melatonin_perfetto)

target_link_libraries(${PROJECT_NAME} PRIVATE Melatonin::Perfetto)