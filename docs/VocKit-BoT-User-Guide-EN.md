# VocKit BoT for Akai MPC — User Guide (English)

## Overview
VocKit BoT includes two independent plugins built with the Akai MPC Plugin SDK:
- **VocKit FX:** a 24-band spectral vocoder effect. Uses an external audio carrier and a voice/modulator signal routed to the left and right channels of the same submix.
- **VocKit Synth:** a MIDI-playable synthesizer using the original VocKit carrier oscillators, with up to eight voices, amplitude and filter ADSR, an arpeggiator, chorus, and delay.

## Installation
1. Download the latest successful **VocKit-BoT-MPC** artifact from GitHub Actions.
2. Unzip the artifact and then the contained VocKit-BoT-MPC.zip archive, if nested.
3. Copy **both plugin directories** from `VST2 Plugins` into the `VST2 Plugins` directory on MPC USB/SD storage. Keep each plugin.json and VST subdirectory intact.
4. Restart the MPC, then scan/select VocKit FX as an effect and VocKit Synth as an instrument.
5. If the old plugin remains visible, verify there is only one active copy of each plugin on the attached storage.

## VocKit Synth
Load VocKit Synth on a plugin track and play it with pads or a MIDI keyboard. OSC, OSC2 and OSC3 select oscillator shape, levels and pitch. FILTER controls cutoff, resonance and envelope depth.
- **AMP:** Synth Attack, Amp Decay, Amp Sustain and Synth Release. Poly Voices selects 1–8 voices.
- **FENV:** Filter Attack, Filter Decay, Filter Sustain, Filter Release and Filter Env Amount.
- **ARP:** Off, Up, Down and UpDown. Rate divisions: quarter, eighth, sixteenth and thirty-second notes; Gate adjusts note duration. Uses MPC transport tempo when supplied, otherwise 120 BPM.
- **SYNFX:** Synth Chorus, Chorus Speed, Synth Delay, Delay Time and Delay Feedback.
- **Presets:** Use the MPC plugin preset menu. The factory patches are starting points; check cutoff, resonance, attack and release after loading. Polyphony and long releases increase CPU usage.

## VocKit FX — routing
1. Put the vocal/modulator signal on one side of a stereo submix (for example right).
2. Put a carrier instrument such as JURA, OPx-4 or VocKit Synth on the other side (for example left).
3. Insert **VocKit FX** on that stereo submix.
4. Choose **L Carrier / R Voice** when the carrier is left and voice is right, or reverse it for **R Carrier / L Voice**.
5. Speak/play the vocal while playing sustained carrier notes. Raise Input, Carrier and Mix, and verify that both submix channels receive audio.
6. Use VOICE for input routing, BAND1/BAND2 for 24 band gains, DESIGN for voice character and spectral freeze, and the other pages for clarity, dynamics and effects.

## Voice Designer
The DESIGN page offers Neutral, Robot, Deep, Whisper, Radio and Choir. These modify spectral formants, frequency shaping and/or consonant processing. Individual manual settings still influence the result.

## Spectral Freeze
Set **Spectral Freeze = Hold** after speaking into the modulator. The vocoder holds its most recently analyzed 24-band voice envelope while the external carrier continues to play. Set **Off** to resume live voice analysis. Capture a clearly voiced sound before engaging Hold.

## Band Q-Links (MPC Live III)
The custom band pages have been mapped to the Live III four-knob/four-position physical layout:
- Switch position 1: knobs 1–4 control bands 1–4 (or 13–16).
- Switch position 2: knobs 1–4 control bands 5–8 (or 17–20).
- Switch position 3: knobs 1–4 control bands 9–12 (or 21–24).
- Switch position 4: unassigned.
Hardware verification is still recommended.

## Troubleshooting
- **Only dry voice:** check submix stereo routing, Input Mode and wet Mix.
- **No vocoder sound:** verify a sustained carrier on one channel and voice on the other; inspect levels.
- **Muffled synth:** increase Filter Cutoff and lower Resonance; inspect Filter Env Amount.
- **No MIDI sound:** use VocKit Synth on a plugin track, check track MIDI input and Level.
- **No new presets/pages:** remove older plugin copies and restart the MPC.
- **High CPU usage:** reduce Poly Voices, shorten releases and disable unused effects.

## Project and support
Project: https://github.com/beatsofthings/beatsofthings-VocKit
More MPC tools: https://mpctoolkit.com

**Status:** The software is under active development. GitHub CI success does not substitute for testing on MPC hardware.
