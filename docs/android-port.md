# Native Android port

This directory tracks the conversion of this pokeemerald-expansion fork from a
GBA-targeted program into an Android application.

## Non-goal: ROM-in-an-emulator packaging

The Android application is not intended to bundle `pokeemerald.gba` or an
emulator core. The long-term target is to keep as much of the existing C game
logic and data as practical while replacing the GBA hardware layer with native
platform services.

That distinction matters because simply putting the ROM in an emulator would
retain the original hardware constraints. A native port gives us a path to
larger/high-resolution assets, a resolution-independent renderer, normal
Android storage/input/audio, and later platform-specific features.

## Milestone 1: native APK bootstrap

The first milestone adds:

- a normal Android application project;
- an NDK-built C shared library;
- Android NativeActivity lifecycle and input handling;
- a native frame loop that renders directly to the Android surface;
- a platform-neutral `port_runtime` boundary;
- GitHub Actions outputting an installable debug APK.

The bootstrap deliberately renders at the Android surface resolution. It does
not force a 240x160 framebuffer and it does not read or execute a GBA ROM.

At this milestone the existing `AgbMain` is **not** called yet. Doing so on
Android would immediately dereference GBA memory-mapped addresses such as
`0x04000000`, `0x05000000`, VRAM, OAM and flash. Those accesses first need
to be moved behind a compatibility/platform layer.

## Migration sequence

1. **Platform boundary** (current)
   - Android lifecycle, frame timing, input, surface.
   - Keep the original GBA build unchanged and buildable.

2. **GBA compatibility state**
   - Replace memory-mapped register macros with host-side register state when
     `PLATFORM_ANDROID` is enabled.
   - Provide host memory for palette RAM, VRAM and OAM.
   - Replace interrupt waits/timers with platform timing.

3. **Renderer**
   - Implement the GBA tile/background/OAM behaviour required by the existing
     engine in software or a GPU-backed renderer.
   - Present the resulting scene through the Android surface.
   - Once behaviour matches, progressively expose higher-resolution native
     sprite/background paths rather than preserving GBA asset limits forever.

4. **Persistence and RTC**
   - Replace flash save calls with Android file storage.
   - Replace RTC hardware access with platform clock APIs.

5. **Audio**
   - Replace m4a/GBA sound hardware output with Android audio.
   - Preserve music/SFX data first; modernize formats later if useful.

6. **Game loop integration**
   - Split initialization currently performed by `AgbMain` into portable game
     initialization and GBA-only platform initialization.
   - Run the existing callbacks/tasks/battle/overworld logic from the Android
     frame loop.

7. **Asset liberation**
   - Keep compatibility with existing GBA assets during migration.
   - Add native asset descriptors with dimensions and formats that are not
     constrained by GBA tile sizes, 4bpp palettes, VRAM layout or ROM-space
     assumptions.
   - Migrate sprites, maps and UI incrementally so playable work is preserved.

## Build

GitHub Actions runs `.github/workflows/android-apk.yml` and uploads
`app-debug.apk` as an artifact named `pokeemerald-native-android-<commit>`.

For local builds, install JDK 17, Android SDK platform 35, Android NDK
27.2.12479018, CMake 3.22.1 and Gradle 8.9, then run:

```sh
gradle -p android :app:assembleDebug
```

The existing GBA build remains separate and unchanged.
