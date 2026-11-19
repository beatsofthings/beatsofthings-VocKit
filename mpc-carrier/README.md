# VocKit Synth 0.2.0 — original VocKit carrier as an MPC instrument

**Two-plugin product:**
- **VocKit FX** is the existing 24-band vocoder with microphone/sample audio input and external carrier mode.
- **VocKit Synth** is the original internal carrier synthesis code from `src/Vocoder.hpp`, now played by MPC MIDI pads/keyboard/sequencer.

The synth uses the same carrier oscillator, chord, unison, three-oscillator, PWM, noise, LFO, filter and drive DSP as VocKit FX. The instrument is currently monophonic for MIDI note selection; unison and chord modes can sound multiple oscillator pitches but do not provide independently voiced polyphony.

## Installation
1. Back up your working VocKit FX plugin and MPC project.
2. Download **VocKit-Carrier-MPC-install** from the successful **Build VocKit Carrier MIDI instrument** GitHub Actions run.
3. Extract the complete plugin directory into the MPC's supported VST2 Plugins location, and restart.
4. Select **VocKit Synth** as a **Plugin Instrument** on a track. VocKit FX remains a separate **Insert Effect**.

## Live microphone + MIDI melody
1. On an Audio Track select the microphone input, set monitoring to **IN**, pan hard **LEFT**.
2. On a Plugin Instrument Track select **VocKit Synth**, pan hard **RIGHT**, and play it from MPC pads or MIDI keyboard.
3. Send both tracks to **Submix 1**.
4. Insert the existing **VocKit FX** on Submix 1, set **Input Mode = R Carrier / L Voice**, **Audio Monitor = Normal**, **Mix = 100%**.
5. Speak into the microphone while holding a note on VocKit Synth. Adjust VocKit FX Carrier/Input/Level as needed.
6. For a recorded voice sample instead of live mic, route a Drum Track playing that sample to Submix 1, panned left; keep the Synth right.
7. Save the MPC project: MIDI notes live on the Synth's instrument track, and plugin settings are stored in each plugin's project state.

## Other carrier plugins
Replace VocKit Synth on the carrier track with JURA, OPx-4, Mini D or any other instrument. VocKit FX routing stays the same.

## Important
- The old simplified four-wave VocKit Carrier prototype is replaced by this shared-engine instrument; use the 0.2.0 package.
- The existing VocKit FX is NOT replaced and is still necessary for vocoding.
- A successful CI build does not prove hardware operation. Test MIDI note-on/off, microphone/submix routing, project save/reload and output levels.
