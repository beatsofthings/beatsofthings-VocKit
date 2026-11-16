# VocKit 0.5.1 — Internal Carrier Synth (English)

The CARRIER tab adds six controls for the **built-in** synth used in Internal Input Mode. They do not process or modify external JURA audio.

| Control | Range | Default | Purpose |
|---|---:|---:|---|
| Pulse Width | 10–90% | 45% | Pulse duty cycle on OSC 1 |
| PWM Depth | 0–40% | 0% | Pulse width modulation amount |
| PWM Rate | 0–12 Hz | 0 Hz | Independent PWM LFO |
| OSC 2 Blend | 0–100% | 0% | Increases contribution of oscillator 2 when enabled |
| Carrier Drive | 0–100% | 0% | Optional soft saturation of internal carrier before vocoding |
| Noise Color | 0–100% | 0% | Blends white and low-passed noise in the carrier |

## First test on MPC
1. Back up your last working version and a test project.
2. Select **Internal** Input Mode and the original **Robot Voice** preset.
3. With all new controls at defaults, compare to the last known-good sound.
4. Select a pulse-based oscillator and move Pulse Width from 45% to 30%.
5. Increase PWM Depth to 15%, PWM Rate to 0.5 Hz.
6. Enable OSC 2, then test OSC 2 Blend at 20–50%.
7. Add Carrier Drive 10–25% and check the level and speech intelligibility.
8. Increase Noise Osc and test Noise Color.
9. Switch to external JURA mode; all six controls should leave the external carrier untouched.
10. Save and reload the project, check all new parameter values and sound.

This is an **experimental build**, not a release candidate. Hardware GUI and performance tests remain necessary.
