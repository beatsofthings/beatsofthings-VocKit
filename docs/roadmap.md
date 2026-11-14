# Roadmap and acceptance criteria

## v0.1: audible proof of concept
- 24 log-spaced filter bands approximately 120 Hz–8 kHz
- Fixed internally generated sawtooth carrier, 1–4-voice notes after MIDI verification
- Attack / release and wet/dry
- Offline tests: silence remains silent; input speech/noise produces finite output; parameter edge cases; varying block sizes; sample rates 44.1 and 48 kHz
- No plugin binary/hardware validation claimed until actual SDK build.

## v0.2: instrument and live input
- MPC plugin wrapper selected using SDK host contract, UI and parameter bridge
- Live microphone and track input where MPC routing supports
- MIDI note handling where chosen plugin type permits
- Host simulation and manual real MPC validation

## v0.3: intelligibility
- Sibilant/unvoiced path, envelope controls, band-shift, carrier tone shaping, calibrated levels
- Touch UI spectrum and Q-Link mappings

## v0.4+: creative
- User WAV import, carrier sample, spectral freeze, presets, optional second audio carrier bus

## Open architectural decision
Effect-first is preferred, but an instrument build may be necessary for MIDI control. The SDK and MPC host must be checked; do not model the plug-in as a general-purpose audio router.
