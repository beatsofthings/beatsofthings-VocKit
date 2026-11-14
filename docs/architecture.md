# VocKit architecture

## Intended product
A low-latency, touch-friendly filterbank vocoder for MPC Live III. **Modulator** = microphone, audio track or sample. **Carrier** = integrated oscillator-based synth, later optional external audio. MIDI chords control internal carrier pitch if the host delivers notes to the chosen plugin type.

## Signal graph
Audio input -> high-pass / gate -> analysis bandpass filter bank -> envelope followers -> smoothing / shaping -> per-band modulation of the internally generated carrier -> band sum -> optional sibilance/noise enhancement -> wet/dry/output.

The internal carrier uses polyphonic saw/pulse oscillators (plus optional noise), envelope and voice allocator. Set frequencies and envelopes off the audio callback where possible; audio-rate updates only where necessary.

## Supported formats and routing
Host integration is **not yet verified**. Treat input audio, MIDI, additional sidechain inputs and audio sample access as distinct capabilities. If effect plugins cannot receive MIDI, investigate two variants: FX (live modulator, fixed carrier pitch / host-supported note input) and Instrument (MIDI + embedded WAV voice source, if SDK supports it). Do not promise an arbitrary external plugin's audio as carrier without an actual MPC routing path.

## Prototype phases
- v0.1: 24-band analysis/synthesis, internal basic carrier, dry/wet, output, regression tests
- v0.2: host MIDI, polyphony, basic MPC touch UI after SDK verification
- v0.3: noise/unvoiced handling, attack/release, formant and input conditioning
- v0.4: WAV source, preset state, freeze and 8/16/32/48 bands
- optional: external carrier if host audio routing permits

## DSP practices
Use allocation-free callback, denormal protection, stable state-variable filters or equivalent, bounded gains, per-sample envelope follower, meaningful spectral spread and normalization. Latency, CPU and intelligibility must be measured in simulator/hardware.
