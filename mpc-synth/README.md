# VocKit Synth — experimental instrument

This is a separate **instrument** with UID `BtVS`, alongside VocKit FX (`BtVK`). It accepts MPC MIDI pad notes and uses an internal oscillator as the carrier. Modulator source is one of four WAV slots in the plugin's `Content/Samples/` folder: `sample1.wav` through `sample4.wav`. Missing files produce silence, not a generated voice. Add WAVs to `mpc-synth/Content/Samples/` before the GitHub build, or replace files in the installed plugin's Content folder and restart.

Current limitations: **monophonic last-note-priority**, no host microphone input (MPC instrument plugins have no guaranteed audio input), no direct browsing of MPC pad sample pool, no in-plugin file browser, no sample recording, and no external carrier routing. WAV playback retriggers on MIDI note-on. `Speed` changes playback rate and duration, not pitch-independent time stretching. Samples load only during prepare, never on the realtime audio thread.

Use this as an experimental test build, not a production-ready release. Test on simulator before installing.
