# VocKit FX – MPC Live III
24-band FFT vocoder effect for MPC OS 3.10.1+ using the official plugin SDK.

## On-device controls
- **VOICE:** Input Mode, Level, Pitch, Chord, Tone, Attack, Release, Input, Carrier
- **CARRIER:** Pitch, Chord, Tone, Detune (cents), Sub Osc (%), Carrier
- **MIX:** Input Mode, Mix, Level, Input, Carrier

## Routing – quick help
**Speech sample or microphone + internal carrier:** Insert VocKit on the voice/audio track. For microphone input, select the actual physical input on an Audio track and enable input monitoring. Set Input Mode=Internal, Mix=100%, then adjust Pitch/Tone/Detune/Sub Osc. Use headphones for microphone monitoring.

**JURA carrier + speech:** Route the voice track to Submix 1 and pan hard RIGHT. Route JURA to the same Submix and pan hard LEFT. Insert VocKit FX on Submix 1 **only** (not on the source tracks). Set Input Mode='L Carrier / R Voice', Mix=100%. Play JURA using the MIDI keyboard. This dual-mono approach depends on the host preserving channel separation and must be checked in the MPC mixer.

**Note:** MPC FX plugins do not receive MIDI directly. The MIDI keyboard controls JURA, and JURA audio feeds VocKit. Live microphone monitoring and exact I/O depend on MPC configuration.

See repository `VOCODER_SETUP_GUIDE.md` for complete desktop, hardware and troubleshooting instructions. The standalone MPC FX has no documented direct sidechain input in the official SDK.
