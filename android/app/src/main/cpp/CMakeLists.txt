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
    "${PORT_ROOT}/src/port_native_heap.c"
    "${PORT_ROOT}/src/port_game_bootstrap.c"
    "${PORT_ROOT}/src/port_gba_host.c"
    "${PORT_ROOT}/src/port_gba_bios.c"
    "${PORT_ROOT}/src/port_gba_dma.c"
    "${PORT_ROOT}/src/port_gba_timing.c"
    "${PORT_ROOT}/src/port_gba_renderer.c"
    "${PORT_ROOT}/src/port_gba_rtc.c"
    "${PORT_ROOT}/src/port_gba_flash.c"
    "${PORT_ROOT}/src/port_gba_audio.c"
    "${REPO_ROOT}/src/main.c"
    "${REPO_ROOT}/src/random.c"
    "${REPO_ROOT}/src/gpu_regs.c"
    "${REPO_ROOT}/src/dma3_manager.c"
    "${REPO_ROOT}/src/task.c"
    "${REPO_ROOT}/src/bg.c"
    "${REPO_ROOT}/src/palette.c"
    "${REPO_ROOT}/src/sprite.c"
    "${REPO_ROOT}/src/trig.c"
    "${REPO_ROOT}/src/decompress.c"
    "${REPO_ROOT}/src/decompress_error_handler.c"
    "${REPO_ROOT}/src/util.c"
    "${REPO_ROOT}/src/international_string_util.c"
    "${REPO_ROOT}/src/dynamic_placeholder_text_util.c"
    "${NATIVE_APP_GLUE}/android_native_app_glue.c"
)

target_include_directories(
    pokeemerald_native
    PRIVATE
    "${PORT_ROOT}/include"
    "${NATIVE_APP_GLUE}"
)

target_compile_features(pokeemerald_native PRIVATE c_std_17)
target_compile_definitions(
    pokeemerald_native
    PRIVATE
    PLATFORM_ANDROID=1
    MODERN=1
    TESTING=0
    PORT_BOOTSTRAP_NO_WEATHER=1
    PORT_BOOTSTRAP_NO_OVERWORLD=1
    PORT_BOOTSTRAP_NO_POKEMON_PICS=1
    PORT_BOOTSTRAP_MENU_ONLY=1
)
target_compile_options(
    pokeemerald_native
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -ffunction-sections
    -fdata-sections
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
    "-Wl,--error-limit=0"
    "-Wl,--gc-sections"
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
target_compile_definitions(pokeemerald_engine_probe PRIVATE PLATFORM_ANDROID=1 MODERN=1 TESTING=0)
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


# Native compile probe for the actual CONTINUE / NEW GAME / OPTION menu.
# Android intentionally skips the copyright screen, intro movie, and Press Start
# title screen. The normal graphics preprocessing step is still required because
# main_menu.c embeds converted interface assets through INCGFX_* macros.
set(MAIN_MENU_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/main_menu_android.c")
add_custom_command(
    OUTPUT "${MAIN_MENU_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 '${REPO_ROOT}/src/main_menu.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${REPO_ROOT}/src/main_menu.c' '${REPO_ROOT}/charmap.txt' > '${MAIN_MENU_ANDROID_C}'"
    DEPENDS
        "${REPO_ROOT}/src/main_menu.c"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

add_library(
    pokeemerald_main_menu_probe
    STATIC
    "${MAIN_MENU_ANDROID_C}"
)
target_include_directories(
    pokeemerald_main_menu_probe
    PRIVATE
    "${PORT_ROOT}/include"
)
target_compile_features(pokeemerald_main_menu_probe PRIVATE c_std_17)
target_compile_definitions(pokeemerald_main_menu_probe PRIVATE PLATFORM_ANDROID=1 MODERN=1 TESTING=0)
target_compile_options(
    pokeemerald_main_menu_probe
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    "-iquote${REPO_ROOT}/include"
)
add_custom_command(
    TARGET pokeemerald_main_menu_probe
    POST_BUILD
    COMMAND "${CMAKE_NM}" -u "$<TARGET_FILE:pokeemerald_main_menu_probe>"
    COMMENT "Main-menu probe unresolved symbols"
    VERBATIM
)

add_dependencies(pokeemerald_native pokeemerald_main_menu_probe)

# Android-preprocessed Emerald UI sources needed by the real main menu.
# text.c and menu.c contain INCGFX assets, so they must pass through the same
# pokeemerald preprocessor used for main_menu.c.
set(TEXT_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/text_android.c")
add_custom_command(
    OUTPUT "${TEXT_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 '${REPO_ROOT}/src/text.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${REPO_ROOT}/src/text.c' '${REPO_ROOT}/charmap.txt' > '${TEXT_ANDROID_C}'"
    DEPENDS
        "${REPO_ROOT}/src/text.c"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

set(MENU_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/menu_android.c")
add_custom_command(
    OUTPUT "${MENU_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 '${REPO_ROOT}/src/menu.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${REPO_ROOT}/src/menu.c' '${REPO_ROOT}/charmap.txt' > '${MENU_ANDROID_C}'"
    DEPENDS
        "${REPO_ROOT}/src/menu.c"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

# Android-preprocessed font glyphs and window-frame graphics required by text.c
# and the real Emerald main menu.
set(FONTS_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/fonts_android.c")
add_custom_command(
    OUTPUT "${FONTS_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 '${REPO_ROOT}/src/fonts.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${REPO_ROOT}/src/fonts.c' '${REPO_ROOT}/charmap.txt' > '${FONTS_ANDROID_C}'"
    DEPENDS
        "${REPO_ROOT}/src/fonts.c"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

set(TEXT_WINDOW_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/text_window_android.c")
add_custom_command(
    OUTPUT "${TEXT_WINDOW_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 '${REPO_ROOT}/src/text_window.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${REPO_ROOT}/src/text_window.c' '${REPO_ROOT}/charmap.txt' > '${TEXT_WINDOW_ANDROID_C}'"
    DEPENDS
        "${REPO_ROOT}/src/text_window.c"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

# Encoded-string utility also requires pokeemerald's charmap preprocessor.
set(STRING_UTIL_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/string_util_android.c")
add_custom_command(
    OUTPUT "${STRING_UTIL_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 '${REPO_ROOT}/src/string_util.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${REPO_ROOT}/src/string_util.c' '${REPO_ROOT}/charmap.txt' > '${STRING_UTIL_ANDROID_C}'"
    DEPENDS
        "${REPO_ROOT}/src/string_util.c"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

# Real Emerald option menu used by the Android main-menu OPTION entry.
set(OPTION_MENU_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/option_menu_android.c")
add_custom_command(
    OUTPUT "${OPTION_MENU_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 '${REPO_ROOT}/src/option_menu.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${REPO_ROOT}/src/option_menu.c' '${REPO_ROOT}/charmap.txt' > '${OPTION_MENU_ANDROID_C}'"
    DEPENDS
        "${REPO_ROOT}/src/option_menu.c"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

# Android-native Littleroot slice. Unlike the GBA map tables, this is emitted as
# native C so pointers have the correct width on 64-bit Android.
set(LITTLEROOT_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/littleroot_android.c")
add_custom_command(
    OUTPUT "${LITTLEROOT_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && mkdir -p build/assets/data/tilesets/primary/general/palettes build/assets/data/tilesets/secondary/petalburg/palettes build/assets/data/tilesets/primary/building/palettes build/assets/data/tilesets/secondary/brendans_mays_house/palettes build/assets/data/tilesets/secondary/lab/palettes && for p in data/tilesets/primary/general/palettes/*.pal data/tilesets/secondary/petalburg/palettes/*.pal data/tilesets/primary/building/palettes/*.pal data/tilesets/secondary/brendans_mays_house/palettes/*.pal data/tilesets/secondary/lab/palettes/*.pal; do out='build/assets/'\"$p\"'.gbapal'; tools/gbagfx/gbagfx \"$p\" \"$out\"; done && tools/gbagfx/gbagfx data/tilesets/primary/general/tiles.png build/assets/data/tilesets/primary/general/tiles.png.4bpp && tools/gbagfx/gbagfx data/tilesets/secondary/petalburg/tiles.png build/assets/data/tilesets/secondary/petalburg/tiles.png_num_tiles_159__Wnum_tiles.4bpp -num_tiles 159 -Wnum_tiles && tools/gbagfx/gbagfx data/tilesets/primary/building/tiles.png build/assets/data/tilesets/primary/building/tiles.png_num_tiles_502__Wnum_tiles.4bpp -num_tiles 502 -Wnum_tiles && tools/gbagfx/gbagfx data/tilesets/secondary/brendans_mays_house/tiles.png build/assets/data/tilesets/secondary/brendans_mays_house/tiles.png.4bpp && tools/gbagfx/gbagfx data/tilesets/secondary/lab/tiles.png build/assets/data/tilesets/secondary/lab/tiles.png_num_tiles_500__Wnum_tiles.4bpp -num_tiles 500 -Wnum_tiles && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 -DPORT_BOOTSTRAP_NO_OVERWORLD=1 '${PORT_ROOT}/src/port_littleroot_assets.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${PORT_ROOT}/src/port_littleroot_assets.c' '${REPO_ROOT}/charmap.txt' > '${LITTLEROOT_ANDROID_C}'"
    DEPENDS
        "${PORT_ROOT}/src/port_littleroot_assets.c"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

# Android-native player slice using Emerald's Brendan walking graphics.
set(TEST_PLAYER_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/test_player_android.c")
add_custom_command(
    OUTPUT "${TEST_PLAYER_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && mkdir -p build/assets/graphics/object_events/pics/people/brendan build/assets/graphics/object_events/palettes && tools/gbagfx/gbagfx graphics/object_events/pics/people/brendan/walking.png build/assets/graphics/object_events/pics/people/brendan/walking.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/palettes/brendan.pal build/assets/graphics/object_events/palettes/brendan.pal.gbapal && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 -DPORT_BOOTSTRAP_NO_OVERWORLD=1 '${PORT_ROOT}/src/port_test_player.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${PORT_ROOT}/src/port_test_player.c' '${REPO_ROOT}/charmap.txt' > '${TEST_PLAYER_ANDROID_C}'"
    DEPENDS
        "${PORT_ROOT}/src/port_test_player.c"
        "${REPO_ROOT}/graphics/object_events/pics/people/brendan/walking.png"
        "${REPO_ROOT}/graphics/object_events/palettes/brendan.pal"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

# Android-native Littleroot NPC slice using Emerald's original object-event art.
set(TEST_NPC_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/test_npc_android.c")
add_custom_command(
    OUTPUT "${TEST_NPC_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && mkdir -p build/assets/graphics/object_events/pics/people build/assets/graphics/object_events/palettes && tools/gbagfx/gbagfx graphics/object_events/pics/people/twin.png build/assets/graphics/object_events/pics/people/twin.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/pics/people/fat_man.png build/assets/graphics/object_events/pics/people/fat_man.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/pics/people/boy_2.png build/assets/graphics/object_events/pics/people/boy_2.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/palettes/npc_1.pal build/assets/graphics/object_events/palettes/npc_1.pal.gbapal && tools/gbagfx/gbagfx graphics/object_events/palettes/npc_2.pal build/assets/graphics/object_events/palettes/npc_2.pal.gbapal && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 -DPORT_BOOTSTRAP_NO_OVERWORLD=1 '${PORT_ROOT}/src/port_test_npc.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${PORT_ROOT}/src/port_test_npc.c' '${REPO_ROOT}/charmap.txt' > '${TEST_NPC_ANDROID_C}'"
    DEPENDS
        "${PORT_ROOT}/src/port_test_npc.c"
        "${PORT_ROOT}/include/port_test_npc.h"
        "${REPO_ROOT}/graphics/object_events/pics/people/twin.png"
        "${REPO_ROOT}/graphics/object_events/pics/people/fat_man.png"
        "${REPO_ROOT}/graphics/object_events/pics/people/boy_2.png"
        "${REPO_ROOT}/graphics/object_events/palettes/npc_1.pal"
        "${REPO_ROOT}/graphics/object_events/palettes/npc_2.pal"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

# Reduced Emerald-style field Start menu used by the Android overworld slice.
set(TEST_FIELD_MENU_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/test_field_menu_android.c")
add_custom_command(
    OUTPUT "${TEST_FIELD_MENU_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 -DPORT_BOOTSTRAP_NO_OVERWORLD=1 '${PORT_ROOT}/src/port_test_field_menu.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${PORT_ROOT}/src/port_test_field_menu.c' '${REPO_ROOT}/charmap.txt' > '${TEST_FIELD_MENU_ANDROID_C}'"
    DEPENDS
        "${PORT_ROOT}/src/port_test_field_menu.c"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

# Compile-only probe for the Android boot handoff that replaces the skipped
# copyright/intro/title presentation flow. Once its dependencies are linked,
# this source becomes part of pokeemerald_native and PortGame_InitialCallback
# can transfer control directly to the real main menu.
add_library(
    pokeemerald_main_menu_startup_probe
    STATIC
    "${PORT_ROOT}/src/port_main_menu_startup.c"
)
target_include_directories(
    pokeemerald_main_menu_startup_probe
    PRIVATE
    "${PORT_ROOT}/include"
)
target_compile_features(pokeemerald_main_menu_startup_probe PRIVATE c_std_17)
target_compile_definitions(pokeemerald_main_menu_startup_probe PRIVATE PLATFORM_ANDROID=1 MODERN=1 TESTING=0 PORT_BOOTSTRAP_MENU_ONLY=1)
target_compile_options(
    pokeemerald_main_menu_startup_probe
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    "-iquote${REPO_ROOT}/include"
)
add_custom_command(
    TARGET pokeemerald_main_menu_startup_probe
    POST_BUILD
    COMMAND "${CMAKE_NM}" -u "$<TARGET_FILE:pokeemerald_main_menu_startup_probe>"
    COMMENT "Direct main-menu startup unresolved symbols"
    VERBATIM
)
add_dependencies(pokeemerald_native pokeemerald_main_menu_startup_probe)


# Non-blocking Android syntax probes for the next milestone: NEW GAME -> overworld.
# They deliberately capture compiler diagnostics without failing the known-good
# menu/options APK, so CI can upload a small actionable report.
add_custom_target(
    pokeemerald_new_game_probe ALL
    COMMAND /bin/bash -c
        "'${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -fsyntax-only -std=gnu17 -Wall -Wextra -Wpedantic -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DTESTING=0 -DPORT_BOOTSTRAP_MENU_ONLY=1 -DPORT_BOOTSTRAP_NO_OVERWORLD=1 '${REPO_ROOT}/src/new_game.c' > '${REPO_ROOT}/android/new-game-compile.txt' 2>&1 || true"
    VERBATIM
)
add_dependencies(pokeemerald_native pokeemerald_new_game_probe)

add_custom_target(
    pokeemerald_overworld_probe ALL
    COMMAND /bin/bash -c
        "'${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -fsyntax-only -std=gnu17 -Wall -Wextra -Wpedantic -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DTESTING=0 -DPORT_BOOTSTRAP_MENU_ONLY=1 -DPORT_BOOTSTRAP_NO_OVERWORLD=1 -DPORT_BOOTSTRAP_NO_WEATHER=1 -DPORT_BOOTSTRAP_NO_POKEMON_PICS=1 '${REPO_ROOT}/src/overworld.c' > '${REPO_ROOT}/android/overworld-compile.txt' 2>&1 || true"
    VERBATIM
)
add_dependencies(pokeemerald_native pokeemerald_overworld_probe)

# These Emerald UI units are internal implementation details of the APK.
# Hiding their global symbols lets --gc-sections discard menu/text functions
# that are not reachable from the direct Android start-menu path.
set_source_files_properties(
    "${REPO_ROOT}/src/window.c"
    "${TEXT_ANDROID_C}"
    "${MENU_ANDROID_C}"
    "${MAIN_MENU_ANDROID_C}"
    "${FONTS_ANDROID_C}"
    "${TEXT_WINDOW_ANDROID_C}"
    "${OPTION_MENU_ANDROID_C}"
    "${STRING_UTIL_ANDROID_C}"
    "${REPO_ROOT}/src/international_string_util.c"
    "${REPO_ROOT}/src/dynamic_placeholder_text_util.c"
    "${REPO_ROOT}/src/fieldmap.c"
    "${REPO_ROOT}/src/field_camera.c"
    "${LITTLEROOT_ANDROID_C}"
    "${TEST_PLAYER_ANDROID_C}"
    "${TEST_NPC_ANDROID_C}"
    "${TEST_FIELD_MENU_ANDROID_C}"
    "${PORT_ROOT}/src/port_test_dialogue.c"
    PROPERTIES
    COMPILE_OPTIONS "-fvisibility=hidden"
)

# Integration branch: link the actual direct-menu startup and real main-menu
# translation unit into the APK so PR CI reports the remaining runtime deps.
target_sources(
    pokeemerald_native
    PRIVATE
    "${PORT_ROOT}/src/port_main_menu_startup.c"
    "${PORT_ROOT}/src/port_menu_only_stubs.c"
    "${PORT_ROOT}/src/port_test_overworld.c"
    "${REPO_ROOT}/src/fieldmap.c"
    "${REPO_ROOT}/src/field_camera.c"
    "${LITTLEROOT_ANDROID_C}"
    "${TEST_PLAYER_ANDROID_C}"
    "${TEST_NPC_ANDROID_C}"
    "${TEST_FIELD_MENU_ANDROID_C}"
    "${PORT_ROOT}/src/port_test_dialogue.c"
    "${REPO_ROOT}/src/window.c"
    "${REPO_ROOT}/src/blit.c"
    "${REPO_ROOT}/src/dynamic_placeholder_text_util.c"
    "${REPO_ROOT}/src/international_string_util.c"
    "${TEXT_ANDROID_C}"
    "${MENU_ANDROID_C}"
    "${MAIN_MENU_ANDROID_C}"
    "${FONTS_ANDROID_C}"
    "${TEXT_WINDOW_ANDROID_C}"
    "${OPTION_MENU_ANDROID_C}"
    "${STRING_UTIL_ANDROID_C}"
)
