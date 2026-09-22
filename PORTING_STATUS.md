# Android Porting Status

Last updated: 2026-09-22

## Goal

Turn the existing pokeemerald-expansion / Emerald-derived game code into a native Android APK base for the new fan game.

The Android build is **not** intended to be a GBA emulator wrapper. Android owns the application lifecycle, frame loop, rendering surface, input, storage, and eventually higher-resolution assets. The original game code is being ported subsystem-by-subsystem so GBA-specific limits can later be removed rather than emulated forever.

## Safety / reference point

- Original pre-Android backup branch: `backup-gba-pre-android-port-2026-09-21`
- Backup commit: `10972b36f396036a039be01c1855a768d11a0318`
- Current working branch: `master`

Do not rewrite or delete the backup branch.

## Current native Android architecture

Android entry point:
- `android/app/src/main/cpp/native_main.c`

Portability layer:
- `port/include/`
- `port/src/`

Android currently owns:
- NativeActivity lifecycle
- outer frame timing
- Android input collection
- native window rendering
- host-side GBA memory compatibility
- BIOS-style memory operations
- DMA compatibility
- VBlank timing compatibility
- RTC compatibility
- flash/save persistence
- audio compatibility scaffolding
- GBA-style framebuffer compatibility rendering

The original Emerald engine now participates in the Android frame loop:
- `Game_Init()`
- `Game_RunFrame()`
- `Game_VBlank()`

No GBA ROM or emulator core is embedded in the native runtime.

## Ported / linked engine pieces

The core rendering/engine integration reached commit `2ba5937b1bbf09355af2d3b7f220c25a777e4392`. Current Android-port head after dependency cleanup is `6d02cf342a6ff6d8cc4a11cec9a0d4549fdb86c9`.

The native Android library directly includes:

- `src/main.c`
- `src/random.c`
- `src/gpu_regs.c`
- `src/dma3_manager.c`
- `src/task.c`
- `src/bg.c`
- `src/palette.c`
- `src/sprite.c`
- `src/trig.c`
- `src/decompress.c`
- `src/decompress_error_handler.c`
- `src/util.c`

Android-specific changes already remove or bypass several GBA-only assumptions, including:
- RFU/link boot dependency
- cartridge flash error boot screen
- GBA timer interrupt dependency
- IWRAM self-copy execution in decompression
- GBA-only RNG implementation path
- GameCube multiboot / serial copyright-screen behavior on Android

## Rendering status

The compatibility renderer currently supports:
- mode-0 text backgrounds
- mode-1 text BG0/BG1
- 4bpp and 8bpp text tiles
- BG palettes
- regular OBJ sprites
- 1D and 2D OBJ tile mapping
- sprite palettes
- basic priority ordering
- nearest-neighbour scaling to the Android window

Still incomplete:
- affine sprite transforms
- affine backgrounds
- bitmap display modes
- blending/window/mosaic fidelity
- final high-resolution/native asset pipeline

The GBA-compatible renderer is a bootstrap bridge, not the desired long-term graphics ceiling.

## Android boot target

Android deliberately skips all three legacy presentation layers:

- copyright screen
- Emerald intro movie
- "Press Start" title screen

The desired Android launch path is:

`Game_Init() -> Android startup/save initialization -> CB2_InitMainMenu()`

`CB2_InitMainMenu()` is the existing Emerald menu containing CONTINUE / NEW GAME / OPTION.

The old copyright callback also performed important non-visual startup work after its animation completed. That work must be preserved even though the screen itself is skipped:

- establish save-block pointers
- reset menu/mon globals
- reset save counters
- load the normal save
- initialize defaults for empty/corrupt saves
- apply saved audio options
- reinitialize the game heap

Do not route Android through the copyright or title callbacks just to obtain those side effects. Move/reuse the initialization directly in the Android startup path.

## Current boot blocker

The original main menu source (`src/main_menu.c`) is not linked into the Android runtime yet. The Android build now compiles it as the active portability probe and reports unresolved symbols.

The temporary `PortGame_InitialCallback` remains in place only until the real main-menu dependency set is linked cleanly. Once that is true, its job is to perform the non-visual startup/save initialization above and transfer control to `CB2_InitMainMenu()`.

## Next milestone

1. Keep the Android main-menu probe compiling on every CI build.
2. Use its unresolved-symbol report to link only the real subsystems needed by `CB2_InitMainMenu()`.
3. Preserve save initialization independently of the skipped copyright flow.
4. Replace the temporary Android callback with the direct startup -> `CB2_InitMainMenu()` handoff.
5. Verify that the APK opens directly on CONTINUE / NEW GAME / OPTION.
6. Verify NEW GAME enters the normal new-game path and CONTINUE detects an Android-persisted save.

The intro/title sources are no longer Android-port milestones.

## Working rules

For every Android-port step:

1. Make one small, atomic change.
2. Build or use the Android CI probe immediately.
3. Fix the first real compiler/linker/runtime blocker.
4. Commit the working checkpoint.
5. Update this file when a milestone changes.

Do not replace real game systems with broad permanent stubs merely to make the linker green. Temporary bootstrap stubs must be clearly isolated under `port/` and documented here.

Do not reintroduce an emulator core as a shortcut.

## Longer-term direction

Once normal gameplay is running natively, gradually replace the compatibility renderer and GBA asset assumptions with Android-native systems. This is the stage where constraints such as 64x64 sprite conventions, GBA palette limits, VRAM layout, and other hardware-era restrictions can be relaxed for the new game.
