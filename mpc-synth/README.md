# VocKit Synth — MPC instrument (experimental)

VocKit Synth is a MIDI-playable 24-band vocoder instrument. It receives MIDI notes from MPC hardware pads, but does **not** receive live mic/audio inputs because the MPC host silences instrument audio inputs.

## Load your own WAV files (new)

1. Install the latest **VocKit-Synth-install** artifact from GitHub Actions.
2. Open the installed plugin folder: `Beats of Things - VST - VocKit Synth/Content/User Samples/`.
3. Copy **up to eight WAV files** into this folder. They may have any filename ending in `.wav` (case-insensitive).
4. Reload the plugin (or restart the MPC). Files are scanned during `prepare()` and assigned **alphabetically** to the eight on-screen pads.
5. Tap a sample pad to select and audition it. Play MIDI notes on the MPC hardware pads to vocode it.

If fewer than eight custom files are present, the remaining slots use the eight bundled **synthetic vowel demo WAVs**. WAV decoding supports PCM 8/16/24/32-bit and float32; WAV only, not MP3. The preview does not retrigger when tapping the already-selected pad.

The MPC SDK's scripted plugin skin does not expose a native file-picker API, so importing is done by copying WAV files into the `User Samples` folder. The decorative waveforms on the eight buttons are **not** derived from sample audio. An in-plugin file picker and real waveform visualization remain unsupported in this version.

## UI

- **SAMPLES**: eight touch-selectable WAV pads, Speed, Loop, Preview and Level.
- **CARRIER**: Transpose, Saw/Pulse Blend, Attack, Release, Mix and Input.
- **VOCODER**: 24 **vertical** spectral band gains (0–200%), Formant Shift, Attack and Release.

The bars are editable controls, **not live level meters**. The engine is currently monophonic, last-note priority. Sample playback is retriggered by MIDI note-on. Speed changes pitch and duration (not time stretching). WAV loading occurs outside the realtime audio callback.

## Audio In

Live microphone input belongs to the separate **VocKit FX** plugin. The official MPC host does not provide reliable live audio inputs to an instrument plugin, and does not deliver MIDI notes to effect plugins.

## Experimental drive-wide WAV selection

On the **FILES** page there are eight `Pad N WAV` parameters. At plugin construction the scanner reads mounted filesystems listed in `/proc/mounts` (plus `Content/User Samples`) and indexes up to 128 WAV files, at most five directory levels deep. It does not scan the root OS filesystem. Drives not exposed as a `/dev/` mount, including some USB/SD mounts, may not be discovered. Filename-only labels can be ambiguous; this is a prototype, not a full folder browser.

**New:** changing a WAV selector now requests an asynchronous load on a background worker; playback uses the new WAV once decoding finishes, without restarting the plugin. The scan still runs at plugin creation, so newly attached drives require a plugin reload. The FILES page is a flat, capped list rather than a folder-by-folder browser. It cannot open the MPC's native file picker. A full navigable browser is still not supported by the documented MPC scripted UI API.
