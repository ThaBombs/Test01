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

# Emerald's shared encoded strings. Link the real data rather than Android
# placeholder copies so battle/menu text continues to use the original engine data.
set(STRINGS_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/strings_android.c")
add_custom_command(
    OUTPUT "${STRINGS_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 '${REPO_ROOT}/src/strings.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${REPO_ROOT}/src/strings.c' '${REPO_ROOT}/charmap.txt' > '${STRINGS_ANDROID_C}'"
    DEPENDS
        "${REPO_ROOT}/src/strings.c"
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
        "cd '${REPO_ROOT}' && mkdir -p build/assets/graphics/object_events/pics/people/brendan build/assets/graphics/object_events/palettes build/assets/graphics/field_effects/pics build/assets/graphics/field_effects/palettes && tools/gbagfx/gbagfx graphics/object_events/pics/people/brendan/walking.png build/assets/graphics/object_events/pics/people/brendan/walking.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/palettes/brendan.pal build/assets/graphics/object_events/palettes/brendan.pal.gbapal && tools/gbagfx/gbagfx graphics/field_effects/pics/tall_grass.png build/assets/graphics/field_effects/pics/tall_grass.png_mwidth_2__mheight_2.4bpp -mwidth 2 -mheight 2 && tools/gbagfx/gbagfx graphics/field_effects/palettes/general_1.pal build/assets/graphics/field_effects/palettes/general_1.pal.gbapal && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 -DPORT_BOOTSTRAP_NO_OVERWORLD=1 '${PORT_ROOT}/src/port_test_player.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${PORT_ROOT}/src/port_test_player.c' '${REPO_ROOT}/charmap.txt' > '${TEST_PLAYER_ANDROID_C}'"
    DEPENDS
        "${PORT_ROOT}/src/port_test_player.c"
        "${REPO_ROOT}/graphics/object_events/pics/people/brendan/walking.png"
        "${REPO_ROOT}/graphics/object_events/palettes/brendan.pal"
        "${REPO_ROOT}/graphics/field_effects/pics/tall_grass.png"
        "${REPO_ROOT}/graphics/field_effects/palettes/general_1.pal"
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
        "cd '${REPO_ROOT}' && mkdir -p build/assets/graphics/object_events/pics/people build/assets/graphics/object_events/palettes && tools/gbagfx/gbagfx graphics/object_events/pics/people/twin.png build/assets/graphics/object_events/pics/people/twin.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/pics/people/fat_man.png build/assets/graphics/object_events/pics/people/fat_man.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/pics/people/boy_2.png build/assets/graphics/object_events/pics/people/boy_2.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/pics/people/scientist_1.png build/assets/graphics/object_events/pics/people/scientist_1.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/pics/people/youngster.png build/assets/graphics/object_events/pics/people/youngster.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/pics/people/prof_birch.png build/assets/graphics/object_events/pics/people/prof_birch.png_mwidth_2__mheight_4.4bpp -mwidth 2 -mheight 4 && mkdir -p build/assets/graphics/object_events/pics/misc build/assets/graphics/object_events/pics/pokemon_old && tools/gbagfx/gbagfx graphics/object_events/pics/misc/birchs_bag.png build/assets/graphics/object_events/pics/misc/birchs_bag.png_mwidth_2__mheight_2.4bpp -mwidth 2 -mheight 2 && tools/gbagfx/gbagfx graphics/object_events/pics/pokemon_old/enemy_zigzagoon.png build/assets/graphics/object_events/pics/pokemon_old/enemy_zigzagoon.png_mwidth_4__mheight_4.4bpp -mwidth 4 -mheight 4 && tools/gbagfx/gbagfx graphics/object_events/palettes/npc_1.pal build/assets/graphics/object_events/palettes/npc_1.pal.gbapal && tools/gbagfx/gbagfx graphics/object_events/palettes/npc_2.pal build/assets/graphics/object_events/palettes/npc_2.pal.gbapal && tools/gbagfx/gbagfx graphics/object_events/palettes/npc_3.pal build/assets/graphics/object_events/palettes/npc_3.pal.gbapal && tools/gbagfx/gbagfx graphics/object_events/palettes/enemy_zigzagoon.pal build/assets/graphics/object_events/palettes/enemy_zigzagoon.pal.gbapal && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 -DPORT_BOOTSTRAP_NO_OVERWORLD=1 '${PORT_ROOT}/src/port_test_npc.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${PORT_ROOT}/src/port_test_npc.c' '${REPO_ROOT}/charmap.txt' > '${TEST_NPC_ANDROID_C}'"
    DEPENDS
        "${PORT_ROOT}/src/port_test_npc.c"
        "${PORT_ROOT}/include/port_test_npc.h"
        "${REPO_ROOT}/graphics/object_events/pics/people/twin.png"
        "${REPO_ROOT}/graphics/object_events/pics/people/fat_man.png"
        "${REPO_ROOT}/graphics/object_events/pics/people/boy_2.png"
        "${REPO_ROOT}/graphics/object_events/pics/people/scientist_1.png"
        "${REPO_ROOT}/graphics/object_events/pics/people/youngster.png"
        "${REPO_ROOT}/graphics/object_events/pics/people/prof_birch.png"
        "${REPO_ROOT}/graphics/object_events/pics/misc/birchs_bag.png"
        "${REPO_ROOT}/graphics/object_events/pics/pokemon_old/enemy_zigzagoon.png"
        "${REPO_ROOT}/graphics/object_events/palettes/npc_1.pal"
        "${REPO_ROOT}/graphics/object_events/palettes/npc_2.pal"
        "${REPO_ROOT}/graphics/object_events/palettes/npc_3.pal"
        "${REPO_ROOT}/graphics/object_events/palettes/enemy_zigzagoon.pal"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/charmap.txt"
    VERBATIM
)

# Playable Android first-battle slice using the original starter / Zigzagoon art.
set(TEST_BATTLE_ANDROID_C "${CMAKE_CURRENT_BINARY_DIR}/generated/test_battle_android.c")
add_custom_command(
    OUTPUT "${TEST_BATTLE_ANDROID_C}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && mkdir -p build/assets/graphics/pokemon/treecko build/assets/graphics/pokemon/torchic build/assets/graphics/pokemon/mudkip build/assets/graphics/pokemon/zigzagoon && tools/gbagfx/gbagfx graphics/pokemon/treecko/back_gba.png build/assets/graphics/pokemon/treecko/back_gba.png.4bpp && tools/gbagfx/gbagfx graphics/pokemon/torchic/back_gba.png build/assets/graphics/pokemon/torchic/back_gba.png.4bpp && tools/gbagfx/gbagfx graphics/pokemon/mudkip/back_gba.png build/assets/graphics/pokemon/mudkip/back_gba.png.4bpp && tools/gbagfx/gbagfx graphics/pokemon/zigzagoon/anim_front_gba.png build/assets/graphics/pokemon/zigzagoon/anim_front_gba.png.4bpp && tools/gbagfx/gbagfx graphics/pokemon/treecko/normal_gba.pal build/assets/graphics/pokemon/treecko/normal_gba.pal.gbapal && tools/gbagfx/gbagfx graphics/pokemon/torchic/normal_gba.pal build/assets/graphics/pokemon/torchic/normal_gba.pal.gbapal && tools/gbagfx/gbagfx graphics/pokemon/mudkip/normal_gba.pal build/assets/graphics/pokemon/mudkip/normal_gba.pal.gbapal && tools/gbagfx/gbagfx graphics/pokemon/zigzagoon/normal_gba.pal build/assets/graphics/pokemon/zigzagoon/normal_gba.pal.gbapal && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DPORT_BOOTSTRAP_MENU_ONLY=1 -DPORT_BOOTSTRAP_NO_OVERWORLD=1 '${PORT_ROOT}/src/port_test_battle.c' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${PORT_ROOT}/src/port_test_battle.c' '${REPO_ROOT}/charmap.txt' > '${TEST_BATTLE_ANDROID_C}'"
    DEPENDS
        "${PORT_ROOT}/src/port_test_battle.c"
        "${PORT_ROOT}/include/port_test_battle.h"
        "${REPO_ROOT}/graphics/pokemon/treecko/back_gba.png"
        "${REPO_ROOT}/graphics/pokemon/treecko/normal_gba.pal"
        "${REPO_ROOT}/graphics/pokemon/torchic/back_gba.png"
        "${REPO_ROOT}/graphics/pokemon/torchic/normal_gba.pal"
        "${REPO_ROOT}/graphics/pokemon/mudkip/back_gba.png"
        "${REPO_ROOT}/graphics/pokemon/mudkip/normal_gba.pal"
        "${REPO_ROOT}/graphics/pokemon/zigzagoon/anim_front_gba.png"
        "${REPO_ROOT}/graphics/pokemon/zigzagoon/normal_gba.pal"
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

# Prepare graphics embedded directly by Emerald's real battle interface.
# The normal GBA build discovers these through scaninc/make dependencies; the
# Android CMake probe runs the preprocessor directly, so mirror those asset
# conversions explicitly instead of replacing the interface graphics.
set(REAL_BATTLE_INTERFACE_ASSET_STAMP "${CMAKE_CURRENT_BINARY_DIR}/generated/real_battle_interface_assets.stamp")
add_custom_command(
    OUTPUT "${REAL_BATTLE_INTERFACE_ASSET_STAMP}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${REPO_ROOT}/build/assets/graphics/battle_interface"
    COMMAND "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/ability_pop_up.png"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/ability_pop_up.png_mwidth_8__mheight_4.4bpp"
        -mwidth 8 -mheight 4
    COMMAND "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/ability_pop_up.pal"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/ability_pop_up.pal.gbapal"
    COMMAND "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/last_used_ball_r_cycle.png"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/last_used_ball_r_cycle.png.4bpp"
    COMMAND "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/last_used_ball_l_cycle.png"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/last_used_ball_l_cycle.png.4bpp"
    COMMAND "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/last_used_ball_r.png"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/last_used_ball_r.png.4bpp"
    COMMAND "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/last_used_ball_l.png"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/last_used_ball_l.png.4bpp"
    COMMAND "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/move_info_window_r.png"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/move_info_window_r.png.4bpp"
    COMMAND "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/move_info_window_l.png"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/move_info_window_l.png.4bpp"
    COMMAND "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/level_up_banner.png"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/level_up_banner.png.gbapal"
    COMMAND "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/level_up_banner.png"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/level_up_banner.png.4bpp"
    COMMAND "${REPO_ROOT}/tools/compresSmol/compresSmol"
        -w
        "${REPO_ROOT}/build/assets/graphics/battle_interface/level_up_banner.png.4bpp"
        "${REPO_ROOT}/build/assets/graphics/battle_interface/level_up_banner.png.4bpp.smol"
    COMMAND "${CMAKE_COMMAND}" -E touch "${REAL_BATTLE_INTERFACE_ASSET_STAMP}"
    DEPENDS
        "${REPO_ROOT}/tools/gbagfx/gbagfx"
        "${REPO_ROOT}/graphics/battle_interface/ability_pop_up.png"
        "${REPO_ROOT}/graphics/battle_interface/ability_pop_up.pal"
        "${REPO_ROOT}/graphics/battle_interface/last_used_ball_r_cycle.png"
        "${REPO_ROOT}/graphics/battle_interface/last_used_ball_l_cycle.png"
        "${REPO_ROOT}/graphics/battle_interface/last_used_ball_r.png"
        "${REPO_ROOT}/graphics/battle_interface/last_used_ball_l.png"
        "${REPO_ROOT}/graphics/battle_interface/move_info_window_r.png"
        "${REPO_ROOT}/graphics/battle_interface/move_info_window_l.png"
        "${REPO_ROOT}/graphics/battle_interface/level_up_banner.png"
        "${REPO_ROOT}/tools/compresSmol/compresSmol"
    VERBATIM
)
add_custom_target(
    pokeemerald_real_battle_interface_assets
    DEPENDS "${REAL_BATTLE_INTERFACE_ASSET_STAMP}"
)

# Compile the real shared Emerald battle engine under the Android toolchain.
# These sources use pokeemerald's encoded-string macros, so run them through
# the repository preprocessor exactly like the sources that ship in the APK.
set(REAL_BATTLE_PROBE_SOURCE_NAMES
    data
    math_util
    money
    line_break
    item_icon
    pokemon_spots
    battle_main
    battle_setup
    battle_controllers
    battle_controller_player
    battle_controller_opponent
    secret_base
    sound
    berry
    field_player_avatar
    metatile_behavior
    field_weather
    party_menu
    pokeball
    battle_tv
    battle_arena
    battle_controller_wally
    battle_controller_safari
    battle_controller_recorded_player
    battle_controller_recorded_partner
    battle_controller_recorded_opponent
    battle_controller_player_partner
    battle_controller_oak_old_man
    battle_controller_link_partner
    battle_controller_link_opponent
    battle_message
    battle_bg
    battle_interface
    battle_script_commands
    battle_gfx_sfx_util
    battle_ai_main
    battle_ai_switch
    battle_ai_items
    battle_ai_util
    battle_ai_field_statuses
    battle_anim
    battle_anim_bug
    battle_anim_dark
    battle_anim_dragon
    battle_anim_effects_1
    battle_anim_effects_2
    battle_anim_effects_3
    battle_anim_electric
    battle_anim_fight
    battle_anim_fire
    battle_anim_flying
    battle_anim_ghost
    battle_anim_ground
    battle_anim_ice
    battle_anim_mon_movement
    battle_anim_mons
    battle_anim_new
    battle_anim_normal
    battle_anim_poison
    battle_anim_psychic
    battle_anim_rock
    battle_anim_smokescreen
    battle_anim_sound_tasks
    battle_anim_status_effects
    battle_anim_throw
    battle_anim_utility_funcs
    battle_anim_water
    battle_intro
    recorded_battle
    graphics
    battle_util
    battle_util2
    battle_end_turn
    battle_move_resolution
    battle_stat_change
    battle_switch_in
    battle_gimmick
    battle_hold_effects
    pokemon
    move
    item
    script_pokemon_util
    starter_choose
    wild_encounter
    malloc
    scanline_effect
    battle_partner
    trainer_pools
    trainer
    trainer_slide
    dexnav
    roamer
    pokerus
    field_weather_effect
    fishing
    pokemon_animation
    battle_terastal
    rtc
    caps
    battle_set_effect
    battle_ai_record
    battle_z_move
    battle_dynamax
    ow_abilities
    difficulty
    event_data
)

set(REAL_BATTLE_ANDROID_SOURCES)
foreach(REAL_BATTLE_NAME IN LISTS REAL_BATTLE_PROBE_SOURCE_NAMES)
    set(REAL_BATTLE_INPUT "${REPO_ROOT}/src/${REAL_BATTLE_NAME}.c")
    set(REAL_BATTLE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/real_battle_${REAL_BATTLE_NAME}.c")
    add_custom_command(
        OUTPUT "${REAL_BATTLE_OUTPUT}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
        COMMAND /bin/bash -o pipefail -c
            "cd '${REPO_ROOT}' && '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -std=gnu17 -iquote'${REPO_ROOT}/include' -iquote'${PORT_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DTESTING=0 '${REAL_BATTLE_INPUT}' | '${REPO_ROOT}/tools/preproc/preproc' -i -g '${REPO_ROOT}/build/assets' '${REAL_BATTLE_INPUT}' '${REPO_ROOT}/charmap.txt' > '${REAL_BATTLE_OUTPUT}'"
        DEPENDS
            "${REAL_BATTLE_INPUT}"
            "${REPO_ROOT}/tools/preproc/preproc"
            "${REPO_ROOT}/charmap.txt"
        VERBATIM
    )
    list(APPEND REAL_BATTLE_ANDROID_SOURCES "${REAL_BATTLE_OUTPUT}")
endforeach()

add_library(
    pokeemerald_real_battle_probe
    STATIC
    ${REAL_BATTLE_ANDROID_SOURCES}
)
target_include_directories(
    pokeemerald_real_battle_probe
    PRIVATE
    "${PORT_ROOT}/include"
)
target_compile_features(pokeemerald_real_battle_probe PRIVATE c_std_17)
target_compile_definitions(
    pokeemerald_real_battle_probe
    PRIVATE
    PLATFORM_ANDROID=1
    MODERN=1
    TESTING=0
)
# The upstream project intentionally relies on a large number of GNU C
# extensions. Keep this probe focused on actual portability errors rather than
# flooding CI with non-actionable warning output.
target_compile_options(
    pokeemerald_real_battle_probe
    PRIVATE
    -w
    -ffunction-sections
    -fdata-sections
    -fvisibility=hidden
    "-iquote${REPO_ROOT}/include"
)
add_dependencies(pokeemerald_real_battle_probe pokeemerald_real_battle_interface_assets)

# Compile the Android-to-Emerald first-battle handoff separately. This keeps
# the adapter tiny and verifies that it only depends on the real battle engine
# plus the existing Android overworld return boundary.
add_library(
    pokeemerald_real_battle_bridge_probe
    STATIC
    "${PORT_ROOT}/src/port_real_battle_bridge.c"
)
target_include_directories(
    pokeemerald_real_battle_bridge_probe
    PRIVATE
    "${PORT_ROOT}/include"
)
target_compile_features(pokeemerald_real_battle_bridge_probe PRIVATE c_std_17)
target_compile_definitions(
    pokeemerald_real_battle_bridge_probe
    PRIVATE
    PLATFORM_ANDROID=1
    MODERN=1
    TESTING=0
)
target_compile_options(
    pokeemerald_real_battle_bridge_probe
    PRIVATE
    -w
    -ffunction-sections
    -fdata-sections
    -fvisibility=hidden
    "-iquote${REPO_ROOT}/include"
)
add_dependencies(pokeemerald_native pokeemerald_real_battle_probe pokeemerald_real_battle_bridge_probe)

# Runtime milestone: the playable Birch encounter now calls the bridge above.
# Link the real Emerald battle archive into the APK. Keeping the archive hidden
# and sectioned lets lld pull only the runtime-reachable battle code instead of
# treating the entire compile probe as exported shared-library API.
target_link_libraries(
    pokeemerald_native
    PRIVATE
    pokeemerald_real_battle_bridge_probe
    pokeemerald_real_battle_probe
)

# Native-width battle-script data probe. The script command parser uses packed
# structs containing native pointers, so Android script data must emit 64-bit
# pointer operands while ordinary u32 command operands remain 32-bit.
set(REAL_BATTLE_SCRIPT_OBJECTS)
foreach(REAL_BATTLE_SCRIPT_NAME IN ITEMS battle_scripts_1 battle_scripts_2)
    set(REAL_BATTLE_SCRIPT_INPUT "${REPO_ROOT}/data/${REAL_BATTLE_SCRIPT_NAME}.s")
    set(REAL_BATTLE_SCRIPT_PREPROCESSED "${CMAKE_CURRENT_BINARY_DIR}/generated/${REAL_BATTLE_SCRIPT_NAME}_android.pre.s")
    set(REAL_BATTLE_SCRIPT_NORMALIZED "${CMAKE_CURRENT_BINARY_DIR}/generated/${REAL_BATTLE_SCRIPT_NAME}_android.S")
    set(REAL_BATTLE_SCRIPT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/generated/${REAL_BATTLE_SCRIPT_NAME}_android.o")
    set(REAL_BATTLE_SCRIPT_ABSOLUTE_SYMBOLS "${CMAKE_CURRENT_BINARY_DIR}/generated/${REAL_BATTLE_SCRIPT_NAME}_absolute_symbols.txt")
    add_custom_command(
        OUTPUT "${REAL_BATTLE_SCRIPT_OBJECT}"
        BYPRODUCTS
            "${REAL_BATTLE_SCRIPT_PREPROCESSED}"
            "${REAL_BATTLE_SCRIPT_NORMALIZED}"
            "${REAL_BATTLE_SCRIPT_ABSOLUTE_SYMBOLS}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
        # Match pokeemerald's upstream DATA_ASM build pipeline:
        # preproc -s -> C preprocessor -> preproc -ie.
        # The second preproc pass converts C enum declarations included by
        # battle scripts into assembler constants before LLVM ever sees them.
        COMMAND /bin/bash -o pipefail -c
            "cd '${REPO_ROOT}' && tools/preproc/preproc -s 'data/${REAL_BATTLE_SCRIPT_NAME}.s' charmap.txt | '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -x assembler-with-cpp -iquote'${REPO_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DTESTING=0 - | tools/preproc/preproc -ie 'data/${REAL_BATTLE_SCRIPT_NAME}.s' charmap.txt > '${REAL_BATTLE_SCRIPT_PREPROCESSED}'"
        # The upstream preprocessing has already expanded includes/enums.
        # This pass only removes ARM-GAS syntax LLVM does not accept and keeps
        # the Android pointer-width changes in battle_script.inc intact.
        COMMAND python3
            "${PORT_ROOT}/tools/normalize_battle_asm.py"
            "${REAL_BATTLE_SCRIPT_PREPROCESSED}"
            "${REAL_BATTLE_SCRIPT_NORMALIZED}"
            --repo-root "${REPO_ROOT}"
        COMMAND "${CMAKE_C_COMPILER}"
            "--target=${CMAKE_C_COMPILER_TARGET}"
            "--sysroot=${CMAKE_SYSROOT}"
            -c
            -x
            assembler-with-cpp
            -DPLATFORM_ANDROID=1
            "${REAL_BATTLE_SCRIPT_NORMALIZED}"
            -o
            "${REAL_BATTLE_SCRIPT_OBJECT}"
        # preproc -ie emits C enum values as absolute ELF symbols. They are
        # assembler constants, not linkable game data. Localize every absolute
        # symbol after assembly so both full battle-script objects can coexist
        # under lld without weakening duplicate checks for real labels/data.
        COMMAND /bin/bash -o pipefail -c
            "'${CMAKE_NM}' --defined-only '${REAL_BATTLE_SCRIPT_OBJECT}' | awk '$2 ~ /^[Aa]$/ {print $3}' > '${REAL_BATTLE_SCRIPT_ABSOLUTE_SYMBOLS}' && if [ -s '${REAL_BATTLE_SCRIPT_ABSOLUTE_SYMBOLS}' ]; then '${CMAKE_OBJCOPY}' --localize-symbols='${REAL_BATTLE_SCRIPT_ABSOLUTE_SYMBOLS}' '${REAL_BATTLE_SCRIPT_OBJECT}'; fi"
        DEPENDS
            "${REAL_BATTLE_SCRIPT_INPUT}"
            "${PORT_ROOT}/tools/normalize_battle_asm.py"
            "${REPO_ROOT}/tools/preproc/preproc"
            "${REPO_ROOT}/charmap.txt"
            "${REPO_ROOT}/asm/macros.inc"
            "${REPO_ROOT}/asm/macros/battle_script.inc"
            "${REPO_ROOT}/constants/constants.inc"
        VERBATIM
    )
    list(APPEND REAL_BATTLE_SCRIPT_OBJECTS "${REAL_BATTLE_SCRIPT_OBJECT}")
endforeach()

add_custom_target(
    pokeemerald_real_battle_scripts_probe
    DEPENDS ${REAL_BATTLE_SCRIPT_OBJECTS}
)

# The battle bridge now enters CB2_InitBattle in the APK, so the interpreter's
# native-width Emerald script data must be present at runtime as well.
set_source_files_properties(
    ${REAL_BATTLE_SCRIPT_OBJECTS}
    PROPERTIES
    GENERATED TRUE
    EXTERNAL_OBJECT TRUE
)
target_sources(
    pokeemerald_native
    PRIVATE
    ${REAL_BATTLE_SCRIPT_OBJECTS}
)
add_dependencies(pokeemerald_native pokeemerald_real_battle_scripts_probe)

# Native-width battle-animation script data. battle_anim.c references the
# complete Emerald move-animation table, so link the original animation
# bytecode rather than substituting per-move Android animations.
set(REAL_BATTLE_ANIM_SCRIPT_INPUT "${REPO_ROOT}/data/battle_anim_scripts.s")
set(REAL_BATTLE_ANIM_SCRIPT_PREPROCESSED "${CMAKE_CURRENT_BINARY_DIR}/generated/battle_anim_scripts_android.pre.s")
set(REAL_BATTLE_ANIM_SCRIPT_NORMALIZED "${CMAKE_CURRENT_BINARY_DIR}/generated/battle_anim_scripts_android.S")
set(REAL_BATTLE_ANIM_SCRIPT_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/generated/battle_anim_scripts_android.o")
set(REAL_BATTLE_ANIM_SCRIPT_ABSOLUTE_SYMBOLS "${CMAKE_CURRENT_BINARY_DIR}/generated/battle_anim_scripts_absolute_symbols.txt")
add_custom_command(
    OUTPUT "${REAL_BATTLE_ANIM_SCRIPT_OBJECT}"
    BYPRODUCTS
        "${REAL_BATTLE_ANIM_SCRIPT_PREPROCESSED}"
        "${REAL_BATTLE_ANIM_SCRIPT_NORMALIZED}"
        "${REAL_BATTLE_ANIM_SCRIPT_ABSOLUTE_SYMBOLS}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND /bin/bash -o pipefail -c
        "cd '${REPO_ROOT}' && tools/preproc/preproc -s 'data/battle_anim_scripts.s' charmap.txt | '${CMAKE_C_COMPILER}' --target=${CMAKE_C_COMPILER_TARGET} --sysroot='${CMAKE_SYSROOT}' -E -x assembler-with-cpp -iquote'${REPO_ROOT}/include' -DPLATFORM_ANDROID=1 -DMODERN=1 -DTESTING=0 - | tools/preproc/preproc -ie 'data/battle_anim_scripts.s' charmap.txt > '${REAL_BATTLE_ANIM_SCRIPT_PREPROCESSED}'"
    COMMAND python3
        "${PORT_ROOT}/tools/normalize_battle_asm.py"
        "${REAL_BATTLE_ANIM_SCRIPT_PREPROCESSED}"
        "${REAL_BATTLE_ANIM_SCRIPT_NORMALIZED}"
        --repo-root "${REPO_ROOT}"
    COMMAND "${CMAKE_C_COMPILER}"
        "--target=${CMAKE_C_COMPILER_TARGET}"
        "--sysroot=${CMAKE_SYSROOT}"
        -c
        -x
        assembler-with-cpp
        -DPLATFORM_ANDROID=1
        "${REAL_BATTLE_ANIM_SCRIPT_NORMALIZED}"
        -o
        "${REAL_BATTLE_ANIM_SCRIPT_OBJECT}"
    COMMAND /bin/bash -o pipefail -c
        "'${CMAKE_NM}' --defined-only '${REAL_BATTLE_ANIM_SCRIPT_OBJECT}' | awk '$2 ~ /^[Aa]$/ {print $3}' > '${REAL_BATTLE_ANIM_SCRIPT_ABSOLUTE_SYMBOLS}' && if [ -s '${REAL_BATTLE_ANIM_SCRIPT_ABSOLUTE_SYMBOLS}' ]; then '${CMAKE_OBJCOPY}' --localize-symbols='${REAL_BATTLE_ANIM_SCRIPT_ABSOLUTE_SYMBOLS}' '${REAL_BATTLE_ANIM_SCRIPT_OBJECT}'; fi"
    DEPENDS
        "${REAL_BATTLE_ANIM_SCRIPT_INPUT}"
        "${PORT_ROOT}/tools/normalize_battle_asm.py"
        "${REPO_ROOT}/tools/preproc/preproc"
        "${REPO_ROOT}/charmap.txt"
        "${REPO_ROOT}/asm/macros.inc"
        "${REPO_ROOT}/asm/macros/battle_anim_script.inc"
        "${REPO_ROOT}/constants/constants.inc"
    VERBATIM
)
add_custom_target(
    pokeemerald_real_battle_anim_scripts_probe
    DEPENDS "${REAL_BATTLE_ANIM_SCRIPT_OBJECT}"
)
set_source_files_properties(
    "${REAL_BATTLE_ANIM_SCRIPT_OBJECT}"
    PROPERTIES
    GENERATED TRUE
    EXTERNAL_OBJECT TRUE
)
target_sources(
    pokeemerald_native
    PRIVATE
    "${REAL_BATTLE_ANIM_SCRIPT_OBJECT}"
)
add_dependencies(pokeemerald_native pokeemerald_real_battle_anim_scripts_probe)

# Compute dependency closure only after both the real Emerald C battle engine
# and the complete native-width battle scripts exist. This prevents hundreds
# of BattleScript_* references from being misreported as external deps.
set(REAL_BATTLE_COMBINED_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/generated/real_battle_combined.o")
add_custom_command(
    OUTPUT "${REAL_BATTLE_COMBINED_OBJECT}"
    COMMAND "${CMAKE_LINKER}"
        -r
        --whole-archive
        "$<TARGET_FILE:pokeemerald_real_battle_probe>"
        "$<TARGET_FILE:pokeemerald_real_battle_bridge_probe>"
        --no-whole-archive
        ${REAL_BATTLE_SCRIPT_OBJECTS}
        "${REAL_BATTLE_ANIM_SCRIPT_OBJECT}"
        -o "${REAL_BATTLE_COMBINED_OBJECT}"
    COMMAND "${CMAKE_NM}" -u "${REAL_BATTLE_COMBINED_OBJECT}" > "${REPO_ROOT}/android/real-battle-unresolved.txt"
    DEPENDS
        pokeemerald_real_battle_probe
        pokeemerald_real_battle_bridge_probe
        pokeemerald_real_battle_anim_scripts_probe
        ${REAL_BATTLE_SCRIPT_OBJECTS}
        "${REAL_BATTLE_ANIM_SCRIPT_OBJECT}"
    COMMENT "Real Emerald battle engine + scripts dependency closure"
    VERBATIM
)
add_custom_target(
    pokeemerald_real_battle_closure
    DEPENDS "${REAL_BATTLE_COMBINED_OBJECT}"
)
add_dependencies(pokeemerald_real_battle_closure pokeemerald_real_battle_probe pokeemerald_real_battle_bridge_probe pokeemerald_real_battle_scripts_probe pokeemerald_real_battle_anim_scripts_probe)
add_dependencies(pokeemerald_native pokeemerald_real_battle_closure)


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
    "${TEST_BATTLE_ANDROID_C}"
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
    "${PORT_ROOT}/src/port_battle_runtime_stubs.c"
    "${PORT_ROOT}/src/port_test_overworld.c"
    "${REPO_ROOT}/src/fieldmap.c"
    "${REPO_ROOT}/src/field_camera.c"
    "${LITTLEROOT_ANDROID_C}"
    "${TEST_PLAYER_ANDROID_C}"
    "${TEST_NPC_ANDROID_C}"
    "${TEST_BATTLE_ANDROID_C}"
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
    "${STRINGS_ANDROID_C}"
)
