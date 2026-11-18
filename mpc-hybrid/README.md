# VocKit Hybrid 0.1.0 — isolated MPC hardware test

**Does not replace VocKit FX.** Separate instrument identity BtVH; uses the existing VocKit DSP.

1. Install this experimental plugin alongside the working VocKit FX.
2. Create an MPC Plugin Instrument track and select **VocKit Hybrid**.
3. Set Input Mode to Internal, Carrier Note Mode to MIDI Notes, Sequencer Off, and Key Record Off. Play a MIDI keyboard on the selected instrument track. This checks instrument MIDI delivery.
4. Test whether an audio track with live microphone monitoring IN can route to the Hybrid instrument's input. This routing is **not guaranteed** on MPC standalone. If the instrument gets no input, the 24-band vocoder will have no speech modulator even when MIDI plays.
5. Keep the existing VocKit FX and its proven submix routing for external JURA, Mini D and other carriers.

**Interpretation:** MIDI works but no microphone = MPC instrument audio input routing limitation; do not assume the plugin can force routing. MIDI and microphone both work = promising single-plugin hybrid path.
