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
