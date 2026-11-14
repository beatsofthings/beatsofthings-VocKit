# SDK and MPC routing verification checklist

- [ ] Pin official SDK version and commit.
- [ ] Inspect official minimal FX and instrument examples and build target names.
- [ ] Confirm how many input/output buses can be declared.
- [ ] Confirm whether FX plugin instances receive MIDI note on/off and pitch bend.
- [ ] Confirm whether instrument instances can access incoming live audio.
- [ ] Confirm whether a separate sidechain carrier bus is supported by both SDK and MPC OS routing.
- [ ] Confirm direct input/microphone monitoring to plugin track / audio track.
- [ ] Confirm access to pads / samples: host API vs explicit WAV import. Do not assume direct pad access.
- [ ] Confirm JSON UI skin controls, touch input, parameter/visualizer update pathways, Q-Links.
- [ ] Check plugin ID uniqueness and preset save/restore behavior.
- [ ] Build, validate, and test in simulator, then on real hardware.

All unchecked items are unknown; these are not confirmed MPC capabilities.
