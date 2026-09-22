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

As of commit `2ba5937b1bbf09355af2d3b7f220c25a777e4392`, the native Android library directly includes:

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

## Current boot blocker

The Emerald engine itself is running, but Android intentionally does **not** enter the real Emerald intro/title callback yet.

In `src/main.c`, Android currently sets:

`PortGame_InitialCallback`

instead of the normal boot callback.

`PortGame_InitialCallback` is presently a minimal no-op callback used to prove that the real Emerald main loop is executing safely.

The original intro source (`src/intro.c`) is already compiled by the Android build as a portability probe, but is not linked into the APK runtime yet.

## Next milestone

Connect Android to the original copyright / intro / title boot path.

Do this incrementally:

1. Produce and inspect the unresolved-symbol list for the compiled Android `intro.c` probe.
2. Group missing symbols by subsystem rather than stubbing them blindly.
3. Link the smallest real supporting modules required for the copyright screen.
4. Keep GBA-only serial/GameCube multiboot behavior disabled or replaced on Android where appropriate.
5. Switch the Android initial callback from `PortGame_InitialCallback` to `CB2_InitCopyrightScreenAfterBootup` only after that callback's reachable dependency set links cleanly.
6. Verify the copyright screen is actually visible in the APK.
7. Continue from copyright -> intro -> title -> new game / load game.

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
