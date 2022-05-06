cmake_minimum_required(VERSION 3.14)

project("Prime App" LANGUAGES CXX)

# default compiler options and warnings
include(cmake/DefaultCompilerOptionsAndWarnings.cmake)

# static analyzers
include(cmake/StaticAnalyzers.cmake)

# sanitizers
include(cmake/Sanitizers.cmake)

# Set project directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

# Hardcoded pvcam directory
link_directories("C:\\Program Files\\Photometrics\\PVCamSDK\\Lib\\amd64")

if(PYTHON_EXECUTABLE)
    message(${PYTHON_EXECUTABLE})
else()
    message(FATAL_ERROR "PYTHON_EXECUTABLE env variable not set")
endif()

add_definitions( -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} )

# Find necessary libs
find_package(imgui CONFIG REQUIRED)
find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(OpenImageIO CONFIG REQUIRED)

set(SFML_LIBS sfml-graphics sfml-system sfml-window)

set(APP_NAME "App")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
add_executable(${APP_NAME} "")
add_subdirectory(src)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_EXTENSIONS OFF)
target_compile_features(${APP_NAME} PUBLIC cxx_std_20)
target_compile_options(${APP_NAME} PRIVATE ${SANITIZER_FLAGS} ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS})

target_include_directories(
        ${APP_NAME} PRIVATE
        "src"
        "C:\\Program Files\\Photometrics\\PVCamSDK\\Inc"
)

target_link_libraries(
        ${APP_NAME} PRIVATE
        ${SANITIZER_FLAGS}
        ${SFML_LIBS}
        ImGui-SFML::ImGui-SFML
        pvcam64
        ${OpenCV_LIBS}
        spdlog::spdlog spdlog::spdlog_header_only
        pybind11::embed
        OpenImageIO::OpenImageIO
)

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})