# VocKit 0.7.3: Touch Keys + REC (experimental)

This remains the original VocKit **effect** plugin, not a separate instrument.

## Capture key presses to the internal pattern
1. Load VocKit on your live microphone track (Input Mode **Internal**) and open **KEYS**.
2. Set **Key Record = Arm**. Set **Sequencer = On** under SEQ, choose Seq Rate and Seq Length (1–16).
3. Press the MPC's transport **REC** and **PLAY**. Touch one of the twelve Key switches on KEYS. The first key-on edge is quantized to the closest step of the active pattern.
4. The recorded note is stored in a **Step 01–16 Note** parameter. Those parameters are part of the VocKit instance's regular MPC project state. Save the MPC project.
5. Set **Key Record = Off** and turn the keyboard switches off, then play the MPC transport to hear the saved sequence.
6. Reopen the project and check the STEP1 / STEP2 values and playback.

## Important limitations
- Recording is into VocKit's **own 16-step pattern**, NOT a MIDI track or the MPC piano roll. A VST2 insert effect cannot create notes in an MPC MIDI track through this SDK.
- This initial version has twelve on/off touch controls, not a fully drawn piano keyboard. A switch must be turned off before the same note can retrigger; simultaneous touches use highest-note priority.
- REC must be active in the host transport AND Key Record must be armed. The plugin checks `transport.recording` and `transport.ppqPos`. If MPC does not expose these values to insert FX, the capture will not occur.
- Notes are quantized to one of 16 steps; velocity, variable gate lengths, and full polyphony are not yet captured.
- In the first build, pattern steps are changed by the audio processing thread through atomic plugin parameter storage; no file access is performed. Hardware project save/reload and touch responsiveness require verification.
- This is not yet a native momentary keyboard. Do not treat it as finished live-performance UX.
