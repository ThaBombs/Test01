# Dicebound Prototype

A first-pass offline Android combat prototype built around an original character/dice duel system.

## Current vertical slice

- Native Android / Kotlin / Jetpack Compose.
- 8-character player squad and 8-character AI squad.
- Choose exactly 3 opening active characters; the other 5 begin in reserve.
- Alternating first-commit each round. When the AI commits first its choice is revealed before the player responds.
- 6 dice per duel:
  - Blade: +1 Attack.
  - Guard: +1 Guard.
  - Speed: +1 Speed.
  - Power: increases ability potency.
  - Focus: contributes to precision/critical attack and healing.
  - Wild: substitutes for ability requirements but has no inherent stat bonus.
- Tap dice to lock them and use up to two rerolls.
- Three abilities per character, resolved tier-by-tier with the faster fighter acting first.
- Dice simultaneously provide stats and satisfy ability patterns.
- One optional tactical action per duel:
  - Power Surge: spend 2 Power.
  - Brace: spend 2 Guard.
  - Quickstep: spend 2 Speed.
  - Fated Strike: spend 2 Focus.
- Spent dice no longer provide stats and cannot activate abilities.
- Persistent random location per match.
- Random event every round.
- AI chooses matchups, rerolls toward an ability pattern, and can spend dice on a tactic.
- Persistent HP, knockouts, automatic reserve promotion, victory/defeat, and restart.
- On-screen combat log showing resolution order and damage.

## Design intent

The key v0.1 experiment is whether one die can support several meaningful decisions:

1. Keep it for its raw stat.
2. Keep it because it completes one or more character abilities.
3. Spend it on a universal tactical action and sacrifice both of the above.

The event/location layer adds changing context without replacing player decisions.

## Build

The branch includes a GitHub Actions workflow that builds a debug APK using JDK 17, Android SDK 35 and Gradle.

Local command (with Android SDK configured):

    gradle :app:assembleDebug

APK output:

    app/build/outputs/apk/debug/app-debug.apk

## Prototype limitations / next candidates

- Reserve replacement is automatic after a knockout; a later version should allow the player to choose which reserve is promoted.
- Character art is intentionally absent in this mechanical prototype.
- No collection/deck editor yet; the roster is currently fixed data.
- No pass-and-play or LAN/Bluetooth P2P yet.
- No save/progression/economy yet.
- Balance values are deliberately provisional.
- Animations/audio are minimal; this pass prioritizes validating the combat loop.
