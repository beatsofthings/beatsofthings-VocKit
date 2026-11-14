# VocKit MPC effect integration (experimental)
The effect uses stereo audio input as its voice/sample **modulator** and an internal saw oscillator as **carrier**. The official MPC SDK host contract says effects receive no MIDI. Live microphone routing depends on MPC track monitoring.

Implemented controls: Level, Pitch, Attack, Release, Mix, Input and Carrier. Chord is reserved in the parameter list but not yet active; it is deliberately hidden in the UI. This is an initial integration, not a validated hardware release.

Build via the official SDK and GitHub Actions. Enable Enhancements and External Plugin Support in MPC OS 3.10.1+. Do not install until build and host-simulator tests pass. No direct pad-sample access or external carrier input is claimed.
