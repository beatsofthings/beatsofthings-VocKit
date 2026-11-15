# VocKit 0.4.4 — MPC Live III release acceptance checklist

## Before testing
- Back up the last working VocKit plugin folder and MPC project.
- Install the latest successful `VocKit-MPC-install.zip` using `INSTALL_EN.md`.
- Use a *copy* of a project; never risk the original project.

## 1. Preset state round-trip on actual MPC hardware (NOT yet verified)
1. Create a project named `VocKit_Persistence_Test`.
2. Insert VocKit on a vocal track; select **Chrome Robot** from the top MPC preset selector.
3. Change and note the following values: **Mix 72**, **Formant Shift -4**, **Band 03 125**, **Band 18 55**, **Chorus Mix 23**, **Ring Mix 17**, **Band Profile Clear**, **Voice Character 31**.
4. Save the MPC project. Leave the plugin, reopen it, and verify each value.
5. Close and reload the project; verify all eight values again.
6. Restart the MPC, reload the project, and verify the values and sound again.
7. Repeat using **external JURA carrier** on a Submix and confirm Input Mode/routing persists.
8. Test preset change and undo/redo (if supported) without losing project settings.
9. Record any difference with a screenshot. **Do not claim this passes until tested on the MPC.**

## 2. UI acceptance
- VOICE: full original BeatsOfThings logo visible; no control overlaps.
- BAND1/BAND2: all 12 band faders and values visible; logo unobstructed.
- FX: three clearly separated modules: SATURATION/RING MOD, STEREO CHORUS, VOICE SHAPING.
- FILTER and OSC: titles and control labels legible, touch and Q-Link behavior correct.
- HELP: English setup instructions legible.
- No static graphic should be described as a live spectrum.

## 3. Sound acceptance
- Compare **Robot Voice** against the known-good version with all new FX at 0%.
- Internal carrier and both external L/R carrier routing modes.
- Chorus and Ring Mix 0% exact bypass; increase independently.
- Factory presets: check gain staging and intelligibility, no clipping.
- Fast rap consonants: short Release and Consonant Clarity settings.

## 4. Stability and performance
- Run continuous playback for at least 10 minutes at 44.1 and 48 kHz where supported.
- Change presets during stopped and running transport.
- Test CPU headroom with 1, 2, and 4 VocKit instances; record MPC CPU display.
- Confirm no crashes, freezes, audio dropouts, or unexpected level jumps.
- GitHub DSP smoke tests, 120-second synthetic benchmark and native host simulator must pass.
- CI performance is **not** an MPC hardware CPU measurement.

## 5. Distribution readiness
- Confirm that `INSTALL_EN.md` is present in every MPC installation ZIP.
- Verify `Plugin Skins/`, `VST/`, `plugin.json` and expected version.
- Check dependencies, third-party artwork and SDK licenses before commercial sale.
- Publish demo recordings and screenshots only from the tested hardware build.
- Keep the current release tagged only after hardware acceptance.
