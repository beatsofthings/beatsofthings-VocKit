# VocKit FX – MPC routing and live input guide

VocKit is an **audio effect**, not a MIDI instrument. Its own internal carrier does not receive MIDI notes in the FX format. The internal pitch is adjusted with **Pitch**. To play notes on a keyboard, use JURA (or another MIDI instrument) as the external carrier.

## A. Sample or microphone + internal carrier
1. Put a speech sample on a Drum pad, or connect a microphone to an audio interface / MPC audio input.
2. For live microphone input, create an Audio track, select the actual physical input, enable input monitoring and verify its input meter. Use headphones to avoid feedback.
3. Insert VocKit FX on that voice/audio track.
4. Set **Input Mode = Internal**, **Mix = 100%**. Start with Input 100%, Carrier 70–100%, Pitch 48, Attack 5 ms, Release 70 ms.
5. Adjust **Tone** (saw/pulse), **Detune** (0–35 cents), **Sub Osc** (0–100%). Start Detune 6 cents and Sub Osc 15% for a fuller sound.

## B. Sample or microphone + JURA (dual-mono submix)
1. Voice/Drum or monitored microphone track -> **Submix 1**, pan fully **right**.
2. JURA instrument track -> **Submix 1**, pan fully **left**. Play a bright saw/brass patch with MIDI notes.
3. Remove/bypass VocKit from the source tracks. Insert **one** VocKit FX on Submix 1.
4. Set **Input Mode = L Carrier / R Voice** and Mix 100%. Submix 1 -> Out 1/2.
5. Confirm that JURA only sounds on the left channel and voice only on the right **before** VocKit. Some MPC mixer paths or effects can collapse or modify this separation; this setup requires verification on the actual host.
6. Avoid sending the Submix output back to itself; avoid direct monitoring the dry microphone and processed microphone simultaneously.

## C. Windows VST3 sidechain
- Main stereo input: voice or microphone audio.
- Optional VST3 sidechain: JURA stereo output.
- Set **Carrier Mode = Sidechain** and route JURA to the plugin's sidechain **only if the DAW exposes that routing**.
- MPC Desktop sidechain support for this VST3 has not been confirmed. Use the dual-mono method when necessary.

## Signal diagnosis
- Mix 0% = dry voice must be clearly audible.
- Internal, Mix 100% = synthesized voice.
- Dual-mono, Mix 100% = synthesized voice only when external carrier is present.
- If no sound: check input monitoring, channel pan, sends, and carrier playback.
- If direct dry voice is heard at Mix 100%: check other effects, monitoring, and parallel audio routing.
- VocKit FX does not receive MIDI directly. The JURA track receives MIDI and supplies audio.
- The FFT engine has inherent processing latency; for live microphone use, choose the lowest stable audio buffer size and monitor through VocKit only.

## Platform controls
MPC hardware: VOICE, CARRIER, MIX pages.
Windows MPC Desktop: stable JUCE generic editor. A custom GUI was previously found to crash MPC Desktop, so the standard editor remains intentionally enabled.
