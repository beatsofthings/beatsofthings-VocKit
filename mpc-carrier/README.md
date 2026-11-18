# VocKit Carrier — Standalone MIDI Instrument (Experimental)

VocKit Carrier is a **separate MPC instrument plugin** designed to supply a MIDI-playable audio carrier to **VocKit FX**. It has its own plugin ID (`BtVC`) and does not replace the VocKit FX (`BtVK`).

## Install
1. Keep your working VocKit FX installation and MPC project backed up.
2. Download the **VocKit-Carrier-MPC-install** artifact from the successful **Build VocKit Carrier MIDI instrument** workflow.
3. Extract the complete plugin folder into the same supported VST2 Plugins directory used for other third-party MPC plugins.
4. Restart MPC and look for **VocKit Carrier** in the **Plugin Instrument** selector, not in the FX selector.

## Live microphone + melodic carrier
1. Create an **Audio Track** with your microphone input, set monitoring to **IN** (not AUTO), and pan fully **left**.
2. Create a **Plugin Instrument Track**, choose **VocKit Carrier**, and pan its output fully **right**.
3. Route both tracks to **Submix 1**.
4. Insert **VocKit FX** on Submix 1, select **R Carrier / L Voice**.
5. Select the VocKit Carrier instrument track and play pads or an external MIDI keyboard while speaking into the mic.
6. To sequence melodies, record MIDI notes on the instrument track. The plugin is currently **monophonic with last-note priority**; chords are not independently polyphonic yet.
7. If the signal sounds too hot, reduce Carrier Level and VocKit input gain before adding saturation.

## Included synth controls
Saw/Pulse/Triangle/Sine waveform, Level, Attack/Release, lowpass Cutoff, Resonance, 1–8 Unison, Detune, Sub oscillator, Drive, Octave; four starting presets: Clean Saw, Electro Pulse, Soft Choir, Dark Robot.

## Known limitations
- This first instrument version uses a simple oscillator and lowpass architecture, not the full VocKit FX internal synthesizer; it is a practical separate MIDI carrier prototype.
- It does not receive microphone audio. VocKit FX handles the 24-band vocoding.
- MIDI note-off uses a release envelope; note tracking is monophonic.
- Build/host simulator success is not a hardware MIDI-routing or sound-quality certification.
- Keep the previous working VocKit FX until the MPC Live III test succeeds.
