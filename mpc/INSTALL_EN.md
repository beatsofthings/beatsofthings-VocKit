# VocKit FX — MPC Installation & Quick Start (English)

## Requirements
- Akai MPC with MPC OS 3.10.1 or later and external plugin support enabled.
- MPC Live III / newer 64-bit hardware or supported 32-bit MPC hardware.
- SD card or USB storage with sufficient free space.
- Save your MPC project and keep a backup of the previous working VocKit before upgrading.

## Installation
1. Download the successful **VocKit-MPC-install** artifact from the GitHub Actions build and extract the outer download archive.
2. Unzip **VocKit-MPC-install.zip**. Keep the complete plugin folder intact, including `plugin.json`, `VST/`, and `Plugin Skins/`.
3. On the MPC, open **Preferences > Enhancements** and enable **Enhancements** and **External Plugin Support** (names may vary with firmware).
4. Copy the complete plugin folder into **VST2 Plugins/** at the root of the SD card or USB drive. Do not copy only the .so file. Alternatively use a compatible Plugin Manager to install the complete package.
5. Safely eject the drive and connect it to the MPC. Restart the MPC application/device if the plugin does not appear immediately.
6. On an audio track or submix, add **VocKit** from **External VST Plugins** as an insert effect.

## First sound — internal carrier
1. Put an acapella/voice sample on an audio or drum track.
2. Insert VocKit on the voice track.
3. On VOICE set **Audio Monitor = Normal**, **Input Mode = Internal**, **Mix = 100%**.
4. Load **Robot Voice** from the MPC preset menu; adjust **Carrier**, **Attack**, **Release**, **Formant Shift**, and **Consonant Clarity**.
5. Check the output level before raising the 24-band sliders.

## External carrier with JURA
1. Route the vocal track and the JURA synth track to the **same Submix**. Insert VocKit only on that Submix.
2. Pan **JURA fully LEFT**, **voice fully RIGHT**; choose **L Carrier / R Voice**.
3. Or pan **voice fully LEFT**, **JURA fully RIGHT**; choose **R Carrier / L Voice**.
4. Use a sustained JURA note/chord and voice simultaneously; start at Mix 100%.
5. If silent, select **Audio Monitor > Listen Left** and **Listen Right** to confirm both signals reach VocKit. Restore **Normal** afterwards.
6. With Mix 0% you should hear the unprocessed vocal input selected by Input Mode.

## 24-band mixer
- BAND1 controls bands 01–12; BAND2 controls 13–24.
- 100% is neutral, 0% mutes a band, 200% boosts a band.
- Lower bands affect warmth/body, middle bands vowel clarity, upper bands brightness and consonants.
- These levels shape the **processed vocoder spectrum**, not the dry voice.
- Q-Links 1–12 are intended to map sequentially to the 12 visible bands.

## Presets and troubleshooting
- The MPC preset dropdown stores complete sounds, including band gains; changing a preset can overwrite unsaved settings.
- Keep the known-good plugin version until the replacement is tested on real hardware.
- If the UI looks outdated, check for duplicate VocKit plugin folders on connected drives and restart after replacement.
- If the effect is silent, first bypass VocKit and verify the source track plays, then use Audio Monitor L/R.
- If the MPC becomes unstable, remove the third-party plugin folder and restart; external plugins run inside the MPC process.

Support and documentation: https://mpctoolkit.com
VocKit — BeatsOfThings. Unofficial third-party MPC plugin; not affiliated with Akai or Oberheim.

## FX page — new sound controls
- **Saturation:** adds harmonic warmth after the vocoder; 0% is a true bypass.
- **Chorus Mix:** controls the stereo modulation return; 0% bypasses chorus.
- **Chorus Rate / Depth:** speed and intensity of the stereo chorus.
- **Band Profile:** Manual, Warm, Clear, Radio or Airy. Profiles are applied non-destructively to the 24 saved band values.
- **Profile Amount:** blends from manual band values (0%) to the selected profile (100%).
- **Voice Character:** subtle formant-lowering and band tilt for voice coloration; 0% preserves existing sound. It is not a guaranteed voice-anonymisation feature.
- If your preset sounds different after upgrading, first set Saturation, Chorus Mix and Voice Character to 0%, and Band Profile to Manual.

## Ring Modulator (FX page)
- **Ring Mix** blends the vocoder output with a metallic, amplitude-modulated version; 0% is bypass.
- **Ring Frequency** sets the modulation oscillator from 10 to 1000 Hz. Try 50–120 Hz for a vintage robot texture, or higher values for metallic effects.
- Ring modulation is not pitch correction and can reduce speech intelligibility at high mix settings.

## Live Microphone + JURA (verified on MPC Live III)
1. Route the microphone audio track and JURA instrument track to the same submix (for example Submix 1).
2. Pan the microphone fully **left** and JURA fully **right**.
3. Insert VocKit on the submix and select **R Carrier / L Voice**.
4. On the microphone audio track, set monitoring to **IN**, not AUTO. This keeps microphone monitoring active while the JURA track is selected for pad playing.
5. Select JURA to play the pads while speaking into the microphone. Start with low output levels to avoid feedback.

## Internal Carrier Synth — 0.6.0 Preview
The internal carrier is independent of the external JURA audio. Select **Internal** under Input Mode.

- **OSC page:** choose the primary oscillator wave, pitch, chord, detune, OSC 2 wave/level/semitone, sub oscillator, noise and LFO.
- **CARRIER page:** Pulse Width, PWM Depth, PWM Rate, OSC 2 Blend, Carrier Drive and Noise Color.
- **SYNTH+ page:** Unison Voices (1–8), Unison Detune, OSC 3 Wave, OSC 3 Level, OSC 3 Semitone, Filter Mode (lowpass/highpass/bandpass), Filter Env Amount.
- **FILTER page:** Filter Cutoff and Resonance, Synth Attack/Release. The new filter envelope amount currently uses the existing Synth Attack and Synth Release controls; a dedicated four-stage filter ADSR is not yet implemented.
- **CLARITY / DYNAMICS pages:** Band Sensitivity, Voice Threshold, Spectral Smoothing, Unvoiced Mix, Carrier EQ, Band Spread, Spectral Tilt, Low/High Band Time, Dynamic Range and Stereo Width.
- **BAND1 / BAND2:** 24 individual band level controls.
- **FX:** Saturation, Ring Modulation, Stereo Chorus, Band Profile and Voice Character.

### Internal carrier sound recipes
**Classic Robot:** Saw/Pulse primary, Unison Voices 1, OSC 3 Level 0, Filter Mode Lowpass, Carrier Drive 0. Start from the original Robot Voice preset.

**Electro Funk:** Saw/Pulse, Unison Voices 3, Unison Detune 8 ct, OSC 3 Wave Pulse, OSC 3 Level 20%, OSC 3 Semitone +12, Pulse Width 35%, PWM Depth 10%, PWM Rate 0.5 Hz.

**Soft Choir:** Triangle primary, Unison Voices 4, Unison Detune 6 ct, OSC 3 Wave Sine, OSC 3 Level 20%, low Filter Resonance, moderate Chorus Mix.

**Dark Machine:** Saw primary, Unison Voices 2, Unison Detune 12 ct, OSC 3 Wave Saw, OSC 3 Level 25%, OSC 3 Semitone -12, moderate Carrier Drive.

### Compatibility and release status
All newly added parameters default to their neutral settings in the 35 factory programs. Keep a backup of your previous working version and MPC projects. This is an experimental build: verify project persistence, DSP stability, CPU load, GUI controls and Q-Link mapping on actual MPC hardware before distributing it as a release.
