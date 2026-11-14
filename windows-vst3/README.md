# VocKit FX for Windows (VST3)

Native Windows x64 VST3 plugin built with JUCE 8 and the shared 24-band Vocoder.hpp engine. The Windows build is independent of the Akai ARM plugin.

## Install
Download the artifact **VocKit-FX-Windows-VST3** from the Windows GitHub Actions run. Unzip twice if GitHub wraps the archive. Copy the `VocKit FX.vst3` bundle into:
`C:\\Program Files\\Common Files\\VST3\\`
Then rescan VST3 plugins in your DAW.

## Modes
- **Internal:** input voice drives the internal synth carrier.
- **External Sidechain (JURA):** main stereo input is voice, sidechain stereo input is the carrier. DAW must support routing a plugin/instrument track into the effect's sidechain.
- **Left Carrier / Right Voice:** main left input is carrier, right is voice.
- **Right Carrier / Left Voice:** reverse.

## MPC Desktop
MPC Desktop may not expose VST3 sidechain inputs or support loading a VST3 effect in every track context. Confirm plugin discovery and audio routing on your PC. If the DAW cannot route a sidechain, try the dual-mono mode with a stereo audio file containing carrier on left and voice on right. The Windows VST3 has **not yet been tested in MPC Desktop or FL Studio**.

## Building locally
Use CMake 3.22+, Visual Studio 2022 C++ toolchain and internet access for JUCE:
`cmake -S windows-vst3 -B windows-vst3/build -G "Visual Studio 17 2022" -A x64`
`cmake --build windows-vst3/build --config Release --target VocKitFX_VST3`
