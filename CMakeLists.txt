cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_GENERATOR_PLATFORM}/${CMAKE_BUILD_TYPE})

project(PrimeApp VERSION 1.0)

find_package(OpenGL REQUIRED)

link_directories("C:\\Program Files\\Photometrics\\PVCamSDK\\Lib\\amd64")

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(SFML
    GIT_REPOSITORY "https://github.com/SFML/SFML.git"
    GIT_TAG "2f11710abc5aa478503a7ff3f9e654bd2078ebab"
    GIT_PROGRESS   TRUE 
    )

FetchContent_Declare(imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui"
    GIT_PROGRESS   TRUE 
    )

FetchContent_MakeAvailable(SFML imgui)

FetchContent_GetProperties(imgui)

set(IMGUI_SFML_DIR ${CMAKE_BINARY_DIR}/_deps/imgui-sfml)

execute_process(
    COMMAND git clone https://github.com/eliasdaler/imgui-sfml ${IMGUI_SFML_DIR} 
)

execute_process(
    COMMAND cat ${IMGUI_SFML_DIR}/imgconfig-SFML.h >> ${imgui_SOURCE_DIR}/imconfig.h
    ) 

file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
add_executable(${PROJECT_NAME} ${IMGUI_SOURCES} ${IMGUI_SFML_DIR}/imgui-SFML.cpp)

add_subdirectory(src include)

set(SFML_LIBS sfml-graphics sfml-system sfml-window)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC ${SFML_LIBS}
    PRIVATE opengl32
    PRIVATE pvcam64
    )

target_include_directories(${PROJECT_NAME} PRIVATE
    "include"
    ${imgui_SOURCE_DIR}
    ${IMGUI_SFML_DIR}
    "C:\\Program Files\\Photometrics\\PVCamSDK\\Inc"
    ) 

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})