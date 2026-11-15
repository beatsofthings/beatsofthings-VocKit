# Real-time 24-band spectrum on Akai MPC: feasibility and safety gate

## What already works
`SpectralVocoder::spectrum()` exposes 24 real measured FFT-band energies. `Vocoder::spectrum()` forwards them. `tests/spectrum_smoke.cpp` verifies nonnegative finite nonzero readings. These readings are NOT yet displayed on the MPC screen.

## Official SDK review (2026-11-15)
- `docs/ui-reference.md` section 6.16: `Plotter` draws **one parameter** and optional history, not a 24-element data array.
- Section 6.15: `Meter` also binds to plugin **parameters**.
- Section 6.13: `Envelope` can bind several editable parameters but is not a generic spectrum widget.
- `framework/mpc_plugin.h` parameter values are atomic; `set()` updates a normal host-visible parameter. Its VST2 state serialization includes these parameters. There is no documented read-only/nonpersistent parameter channel, shared spectrum buffer, custom drawing callback, or push event API in the reviewed public interface.
- A previous prototype wrote continuously varying levels to host parameters from the audio callback; the SDK host_sim state round-trip check failed. Do NOT repeat this as a release path.

## Required before implementing a live on-screen spectrum
1. Obtain SDK confirmation or demonstrate a supported **non-persistent, non-automatable telemetry channel** to the TUI, or a supported custom UI renderer that can read a plugin-owned snapshot without host parameter serialization.
2. Keep spectrum values inside the DSP and use lock-free single-producer/single-consumer snapshots; update UI no more than 15–30 fps, never every audio sample.
3. Draw 24 actual bars only if UI reads real 24-bin values. Do not show static fake bars as a live spectrum.
4. Verify on MPC Live III: continuous playback 10+ minutes, preset load/save and project reopen, no crashes, no changing saved parameters, no audio glitches, and acceptable CPU.
5. Keep this work isolated until the interface is proven; existing FX and preset code remains the stable release.

## Safe fallback
A separate offline diagnostics tool can render 24 bands from captured test audio, but that is not an MPC live spectrum and should not be advertised as such.
