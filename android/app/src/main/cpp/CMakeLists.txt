cmake_minimum_required(VERSION 3.22.1)

project(pokeemerald_native C)

set(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../")
set(PORT_ROOT "${REPO_ROOT}/port")
set(NATIVE_APP_GLUE "${ANDROID_NDK}/sources/android/native_app_glue")

add_library(
    pokeemerald_native
    SHARED
    native_main.c
    "${PORT_ROOT}/src/port_runtime.c"
    "${PORT_ROOT}/src/port_gba_host.c"
    "${NATIVE_APP_GLUE}/android_native_app_glue.c"
)

target_include_directories(
    pokeemerald_native
    PRIVATE
    "${PORT_ROOT}/include"
    "${REPO_ROOT}/include"
    "${NATIVE_APP_GLUE}"
)

target_compile_features(pokeemerald_native PRIVATE c_std_17)
target_compile_definitions(pokeemerald_native PRIVATE PLATFORM_ANDROID=1 MODERN=1)
target_compile_options(pokeemerald_native PRIVATE -Wall -Wextra -Wpedantic)

find_library(ANDROID_LIB android)
find_library(LOG_LIB log)

target_link_libraries(
    pokeemerald_native
    PRIVATE
    ${ANDROID_LIB}
    ${LOG_LIB}
)

target_link_options(
    pokeemerald_native
    PRIVATE
    "-Wl,--undefined=ANativeActivity_onCreate"
)
