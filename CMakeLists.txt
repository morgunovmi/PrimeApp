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

# Find necessary libs
find_package(imgui REQUIRED)
find_package(SFML COMPONENTS system window graphics REQUIRED)
find_package(ImGui-SFML REQUIRED)
find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)
find_package(pybind11 REQUIRED)
find_package(OpenImageIO REQUIRED)
find_package(fmt)
find_package(range-v3)
find_package(nlohmann_json REQUIRED)

set(SFML_LIBS sfml-graphics sfml-system sfml-window)

set(APP_NAME "App")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
add_executable(${APP_NAME}
        vendor/ImGuiFileDialog/ImGuiFileDialog.cpp
        vendor/simple_serial_port/simple-serial-port/simple-serial-port/SimpleSerial.cpp
        )
add_subdirectory(src)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_EXTENSIONS OFF)
target_compile_features(${APP_NAME} PUBLIC cxx_std_20)
target_compile_options(${APP_NAME} PRIVATE ${SANITIZER_FLAGS} ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS})

target_include_directories(
        ${APP_NAME} PRIVATE
        vendor/ImGuiFileDialog
        vendor/simple_serial_port/simple-serial-port/simple-serial-port
        src
        "C:\\Program Files\\Photometrics\\PVCamSDK\\Inc"
)

target_link_libraries(
        ${APP_NAME} PRIVATE
        ${SANITIZER_FLAGS}
        ${SFML_LIBS}
        imgui::imgui
        ImGui-SFML::ImGui-SFML
        pvcam64
        range-v3::range-v3
        ${OpenCV_LIBS}
        fmt::fmt-header-only
        spdlog::spdlog_header_only
        pybind11::embed
        OpenImageIO::OpenImageIO
        nlohmann_json::nlohmann_json
        OneCore
)

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})