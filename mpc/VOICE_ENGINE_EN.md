# VocKit 0.5.0 — Voice Engine Preview (English)

This preview adds 11 controls to the **vocoder processing engine**, not to the internal carrier synthesizer. New parameters have neutral defaults so the existing 35 factory presets retain their previous intended sound.

## CLARITY
- **Band Sensitivity (25–400%, default 100%)**: adjusts the 24-band speech envelope response globally.
- **Voice Threshold (-80 to -10 dB, default -80 dB)**: optional modulator gate; -80 dB disables it.
- **Spectral Smoothing (0–100%, default 0%)**: slows analysis envelope response to reduce rapid spectral fluctuations.
- **Unvoiced Mix (0–100%, default 0%)**: blends a high-passed speech path for consonant articulation.
- **Carrier EQ (-100 to +100%, default 0%)**: spectral tilt applied to the carrier gain after vocoding; not a standalone JURA EQ.
- **Band Spread (0–100%, default 0%)**: blends neighboring analysis bands to reduce narrow-band fluctuations.

## DYNAMICS
- **Spectral Tilt (-100 to +100%, default 0%)**: tilts the spectral envelope across the frequency bands.
- **Low Band Time (25–400%, default 100%)**: scales attack/release times in the lowest eight bands.
- **High Band Time (25–400%, default 100%)**: scales attack/release times in the highest eight bands.
- **Dynamic Range (50–150%, default 100%)**: power curve on vocoder band gains.
- **Stereo Width (0–200%, default 100%)**: adjusts the stereo side signal after the FX chain. This does not create stereo from a mono source.

## Hardware test sequence
1. Back up the last known-good plugin and MPC project before installing.
2. Load the original **Robot Voice** factory preset; leave all new controls at defaults.
3. Confirm the previous sound remains the same and that the two new pages display correctly.
4. With JURA panned right and microphone panned left, select **R Carrier / L Voice**.
5. Test **Band Sensitivity 125%**, **Spectral Smoothing 20%**, **Unvoiced Mix 15%** independently, resetting each to default.
6. Try **Voice Threshold -45 dB** only if the microphone noise floor is audible; increase toward -30 dB cautiously.
7. Try **Carrier EQ +20%** for brighter JURA carriers and **Band Spread 20%** for smoother response.
8. Check that project save, restart and reload preserve the 11 new settings.
9. Compare CPU load and clipping against 0.4.5.

**Status:** Experimental feature preview. Code and UI are not equivalent to a passed hardware test. No live FFT spectrum display has been added.
