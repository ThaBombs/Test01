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
    "${PORT_ROOT}/src/port_gba_bios.c"
    "${PORT_ROOT}/src/port_gba_dma.c"
    "${PORT_ROOT}/src/port_gba_timing.c"
    "${PORT_ROOT}/src/port_gba_renderer.c"
    "${PORT_ROOT}/src/port_gba_rtc.c"
    "${PORT_ROOT}/src/port_gba_flash.c"
    "${PORT_ROOT}/src/port_gba_audio.c"
    "${REPO_ROOT}/src/gpu_regs.c"
    "${REPO_ROOT}/src/dma3_manager.c"
    "${NATIVE_APP_GLUE}/android_native_app_glue.c"
)

target_include_directories(
    pokeemerald_native
    PRIVATE
    "${PORT_ROOT}/include"
    "${NATIVE_APP_GLUE}"
)

target_compile_features(pokeemerald_native PRIVATE c_std_17)
target_compile_definitions(pokeemerald_native PRIVATE PLATFORM_ANDROID=1 MODERN=1)
target_compile_options(
    pokeemerald_native
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    "-iquote${REPO_ROOT}/include"
)

find_library(ANDROID_LIB android)
find_library(LOG_LIB log)
find_library(M_LIB m)

target_link_libraries(
    pokeemerald_native
    PRIVATE
    ${ANDROID_LIB}
    ${LOG_LIB}
    ${M_LIB}
)

target_link_options(
    pokeemerald_native
    PRIVATE
    "-Wl,--undefined=ANativeActivity_onCreate"
)


# Compile-only portability probe for the real Emerald main engine source.
# This target intentionally is not linked into the APK yet; unresolved runtime
# dependencies are handled subsystem-by-subsystem after source portability is clean.
add_library(
    pokeemerald_engine_probe
    STATIC
    "${REPO_ROOT}/src/main.c"
)

target_include_directories(
    pokeemerald_engine_probe
    PRIVATE
    "${PORT_ROOT}/include"
)

target_compile_features(pokeemerald_engine_probe PRIVATE c_std_17)
target_compile_definitions(pokeemerald_engine_probe PRIVATE PLATFORM_ANDROID=1 MODERN=1)
target_compile_options(
    pokeemerald_engine_probe
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    "-iquote${REPO_ROOT}/include"
)


add_custom_command(
    TARGET pokeemerald_engine_probe
    POST_BUILD
    COMMAND "${CMAKE_NM}" -u "$<TARGET_FILE:pokeemerald_engine_probe>"
    COMMENT "Engine probe unresolved symbols"
    VERBATIM
)


# Gradle requests pokeemerald_native specifically. Force the compile-only
# engine probe to build as part of every APK so CI genuinely validates main.c.
add_dependencies(pokeemerald_native pokeemerald_engine_probe)
