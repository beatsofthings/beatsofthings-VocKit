# VocKit — Factory Sound Guide (English)

Version 0.4.5 · BeatsOfThings · https://mpctoolkit.com

VocKit is a 24-band vocoder FX for standalone Akai MPC. Load factory programs using the MPC preset menu at the top of the plugin screen. Presets are starting points, not loudness-matched commercial masters. Keep the previous working build until you have verified the new version on your hardware.

## Preset families

### Classic robot and talkbox
- **Robot Voice** — original neutral robot sound; retained unchanged as a reference.
- **Retro Robot**, **Electro Robot**, **Analog Talker**, **Electro Robot**, **Chrome Robot**, **Robot Drive**, **Industrial Android** — increasingly metallic and driven robotic textures.
- **Neon Talkbox**, **Funky Pulse**, **Electro Funk Lead**, **Analog Brass Voice** — rhythmic, melodic electro-funk starting points.
- **Clear Rap**, **Band Clear Speech** — shorter tracking times for fast consonants.

### Choir and atmospheric
- **Soft Strings**, **Electric Choir**, **Midnight Choir**, **Cathedral Choir**, **Whispering Synth**, **Band Airy Choir**, **Warm Chorus** — softer envelopes, chorus and brighter/wider textures.
- **Analog Whisper**, **Band Warm Voice**, **Soft Alter Ego**, **Alter Ego** — softer or formant-shifted voice characters.

### Dark and experimental
- **Dark Vocoder**, **Dark Machine**, **Dark Transmission**, **Space Transmission** — darker carrier and lower spectral emphasis.
- **LoFi Broadcast**, **Band Radio Robot**, **Aggressive**, **Saw Brass** — more focused, distorted or mid-heavy effects.
- **Band Clear Speech**, **Band Warm Voice**, **Band Airy Choir**, **Band Radio Robot** demonstrate different band shapes.
- **JURA L / Voice R** and **JURA R / Voice L** are routing presets, not self-contained internal-carrier sounds.
- **Init** restores the basic sound.

## Useful starting adjustments
- **Rap intelligibility:** Attack 2–5 ms, Release 35–80 ms; keep Ring Mix low; adjust Consonant Clarity gradually.
- **Robot:** Saw/Pulse or Square carrier; Formant Shift near 0; Ring Mix 10–30%; Ring Frequency 60–150 Hz.
- **Choir:** Triangle or Sine carrier, longer Release, Chorus Mix 20–40%.
- **Lo-fi radio:** Radio Band Profile, modest Saturation and lower brightness.
- **External JURA:** Route voice and carrier to the same submix, pan fully opposite, choose the matching Input Mode.

## Gain staging and compatibility
Some more aggressive new presets have reduced Level, Saturation, Chorus Mix or Ring Mix in 0.4.5 to leave extra headroom. These are provisional design adjustments, not a measured loudness normalization. The original **Robot Voice** program remains unchanged.

If a preset is too loud, lower Level before adding additional effects. If speech is unclear, lower Ring Mix and Chorus Mix and use a brighter carrier. Do not overwrite saved projects without a backup.

## Release validation still required
- Audition all 35 presets on MPC Live III with spoken voice and fast rap.
- Check level differences, distortion, intelligibility and CPU.
- Verify project save, reload and power-cycle persistence.
- Confirm GUI text, logo, band faders and Q-Link mapping on hardware.
- Only then treat the factory bank as final for sale.
