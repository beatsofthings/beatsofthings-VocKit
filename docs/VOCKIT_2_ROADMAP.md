# VocKit 2.0 roadmap

## Stability gates
Keep the working 512-point FFT/128-hop vocoder, robot presets, and the known-good MPC package. New controls must default to neutral. Validate native, arm32, arm64, TUI, Q-Links, preset round-trip, and audio host simulation. Avoid a complete custom TUI replacement (previously black screen). Do not write continuously changing audio telemetry to saved host parameters (previously broke state round-trip).

## Stage 1 — DSP (in progress)
- [x] OSC 2: Saw/Triangle/Sine/Square, independent level, semitone and cents; default off.
- [x] Optional high-pass consonant clarity; default off; listening tests pending.
- [ ] Formant Shift in the **active FFT spectral envelope**, not the unused legacy bandpass path. Test ±12 semitones, intelligibility and numerical stability.
- [ ] Regression tests for neutral output and OSC2 audibility.

## Stage 2 — MPC visuals
- [ ] New dark amber background, visually separated VOICE/OSC/FILTER modules, waveform icons, clear OSC1/OSC2 labels.
- [ ] Preserve 12 tall vertical band faders per page and sequential Q-Link mappings.
- [ ] True combined Cutoff+Resonance and Attack+Release displays; do not mislabel static diagrams as dynamic.
- [ ] Incremental SDK-generated skin patching and screenshot review.

## Stage 3 — Live spectrum
- [ ] Evaluate MPC SDK transport for 24 FFT-band levels and safe UI refresh. Plotter handles a single parameter; it is not a ready-made multiband spectrum.
- [ ] Prototype isolated from saved preset state; test MPC CPU load and UI stability.
- [ ] No allocation/locking/host notification inside audio callback.

## Stage 4 — FX
- [ ] Optional saturation with level-matched bypass.
- [ ] Stereo chorus with conservative CPU and mono compatibility.
- [ ] Confirm vocoder -> saturation -> chorus ordering and dry path.

## Stage 5 — Presets
- [ ] Electro Robot, Midnight Choir, Analog Talker, Funk Transmission, Dark Space, Retro Machine and Wide Synth Voice.
- [ ] Full parameter vectors and automated preset/state tests.
- [ ] Preserve existing Robot Voice, Retro Robot and external JURA routing presets.

Release only after green MPC CI plus hardware audio/UI checks.
