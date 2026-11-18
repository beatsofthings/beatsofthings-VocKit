# VocKit Audio Pad Control — experimental 4-pad test

This test adds audio-coded note control to the EXISTING VocKit FX, not a new instrument.
The four generated WAVs are in PAD_PILOTS_TEST. Load them into a Drum Program and use **Gate / Note On** playback (not One Shot), so release stops the pilot tone. Each sample is one second long and may need looping for sustained notes.

## Required routing
- **Left channel** of the VocKit insert: live microphone (monitor IN).
- **Right channel**: audio from the Drum Program playing pilot tones.
- Both must reach **the same stereo submix**; use hard left/right panning.
- Insert VocKit FX on that submix.
- VocKit: Input Mode = Internal, Pad Audio Input = 4 Tone Pilot, Sequencer = Off.
- Pad Base Note defaults to MIDI 48 (C3); A02=48, A03=49, A04=50, A05=51.
- On a Drum Program, assign A02=750 Hz, A03=1000 Hz, A04=1250 Hz, A05=1500 Hz.
- A01 can hold a speech sample for a later test, but do NOT put it on the pilot-right channel; it should be on the microphone/voice-left side.
- If you use a live microphone, A01 does not need a sample. The live microphone is independent of the note pads.

## What is / is not supported
- This is a **four-pad proof of concept** only; banks B/C/D are not mapped yet.
- Detection uses 512-sample frequency analysis every 128 samples. Real note-on latency, note-off behavior and false triggers must be measured on MPC hardware.
- The pilot samples carry the pitch control via audio; they are not intended to be heard as musical sound.
- The internal VocKit synth and 24-band vocoder remain intact. The right pilot audio is excluded from the voice input and dry output while pilot mode is on.
- This version detects one note at a time. If two pads overlap, the strongest valid tone may win.
- The WAVs are 48 kHz mono PCM; MPC may resample them. Keep pitch/stretch disabled on those pads.
- Pilot control is an audio workaround, not direct pad ID recognition or virtual MIDI.
- Save your MPC project: Drum Program pad assignments and VocKit parameters should persist in their respective project components; verify on hardware.
- **Restore Pad Audio Input = Off** for normal VocKit routing.
