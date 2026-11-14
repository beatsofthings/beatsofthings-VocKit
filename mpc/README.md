# VocKit FX – MPC Live III
24-band FFT vocoder effect for MPC OS 3.10.1+ using the official plugin SDK.

## Factory programs and band pages
The official SDK built-in program API provides 10 presets in the MPC native preset selector: Init, Robot Voice, Clear Rap, Dark Vocoder, Saw Brass, Soft Strings, Aggressive, Retro Robot, JURA L / Voice R, JURA R / Voice L. The 24 FFT bands are individually adjustable in three pages: BANDS 1-8, BANDS 9-16 and BANDS 17-24. Presets are stored by the SDK as host programs, not custom buttons. The JURA presets only configure the plugin; physical mixer routing remains manual.

## On-device controls
- **VOICE:** Input Mode, Level, Pitch, Chord, Tone, Attack, Release, Input, Carrier
- **CARRIER:** Pitch, Octave, Chord, Tone, Detune, Sub Osc, Noise Osc, Filter Cutoff, Resonance, Synth Attack, Synth Release, Carrier
- **BANDS 1-8 / 9-16 / 17-24:** 24 individual spectral band faders
- **MIX:** Input Mode, Mix, Level, Input, Carrier

## Routing – quick help
**Speech sample or microphone + internal carrier:** Insert VocKit on the voice/audio track. For microphone input, select the actual physical input on an Audio track and enable input monitoring. Set Input Mode=Internal, Mix=100%, then adjust Pitch/Tone/Detune/Sub Osc. Use headphones for microphone monitoring.

**JURA carrier + speech:** Route the voice track to Submix 1 and pan hard RIGHT. Route JURA to the same Submix and pan hard LEFT. Insert VocKit FX on Submix 1 **only** (not on the source tracks). Set Input Mode='L Carrier / R Voice', Mix=100%. Play JURA using the MIDI keyboard. This dual-mono approach depends on the host preserving channel separation and must be checked in the MPC mixer.

**Note:** MPC FX plugins do not receive MIDI directly. The MIDI keyboard controls JURA, and JURA audio feeds VocKit. Live microphone monitoring and exact I/O depend on MPC configuration.

See repository `VOCODER_SETUP_GUIDE.md` for complete desktop, hardware and troubleshooting instructions. The standalone MPC FX has no documented direct sidechain input in the official SDK.

## Platform UI limitation
MPC standalone uses the official SDK JSON-based touchscreen pages. The Windows JUCE custom editor cannot be copied directly to MPC standalone. The audio engine and sound parameters are shared; visual parity is not yet complete. Hardware MIDI notes cannot directly enter the FX plugin, so JURA must be played as a separate MIDI instrument and routed into the effect as audio.
