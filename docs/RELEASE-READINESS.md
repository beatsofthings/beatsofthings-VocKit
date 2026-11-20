# VocKit BoT — Public Release Acceptance Checklist

**Release status: NOT YET APPROVED FOR SALE.** Passing GitHub Actions is not equivalent to passing MPC Live III hardware tests.

## Mandatory hardware regression tests (MPC OS 3.10.1+)
- [ ] Both plugins install cleanly from a fresh USB/SD `VST2 Plugins` directory and are recognized after reboot.
- [ ] FX routes L Carrier / R Voice and R Carrier / L Voice with no dry-voice leakage when fully wet.
- [ ] FX BAND1/BAND2 Q-Links: positions 1–3, knobs 1–4 address the twelve corresponding bands in order. Position 4 unassigned.
- [ ] FX presets visibly update the corresponding on-screen parameters; project save/reopen retains user changes without changing factory programs.
- [ ] Synth MIDI note-on/note-off, overlapping chords (up to eight voices), voice stealing and All Notes Off work without stuck notes.
- [ ] Synth AMP and FILTER ADSR controls respond across all stages and don't click on note release.
- [ ] Synth arpeggiator Up/Down/UpDown runs at MPC tempo, stops cleanly and responds to changing held notes.
- [ ] Chorus and delay work independently, with feedback below runaway level.
- [ ] Spectral Freeze holds an audible spectral envelope and resumes normal tracking after release.
- [ ] Factory presets sound usable at 44.1/48 kHz with no unexpectedly closed filters, excessive resonance or output clipping.
- [ ] At least 20 minutes of stress playback: rapid preset changes, dense chords, FX bypass, transport start/stop, load/save/reload, no crash.
- [ ] Record peak CPU usage for 1, 4 and 8 synth voices on MPC Live III.
- [ ] Document user-visible known limitations and verify install guide against the shipping ZIP.

## Distribution and support
- [ ] Use versioned release archive and immutable tag; publish changelog and known issues.
- [ ] Verify plugin IDs are unique, version fields correct and licenses/third-party notices included.
- [ ] Provide English documentation, support contact, purchase terms, refund policy and update policy.
- [ ] If commercial licensing is added, test activation, offline use, device migration and recovery before charging users.
- [ ] Do not claim support for MPC models not tested on hardware.

## Current engineering risks
- The migrated factory presets were remapped from a previous FX architecture and need listening tests.
- The Synth runs multiple instances of the original vocoder carrier DSP; performance is unverified on target hardware.
- Arpeggiator behavior and plugin state persistence must be validated on MPC hardware.
- The package workflow downloads the latest *successful* FX and Synth artifacts independently, so their source revisions may differ.

The release owner must sign off the above items before describing VocKit BoT as production-ready.
