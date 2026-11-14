# VocKit — development instructions

Target: Akai MPC Live III, MPC OS 3.10.1+, official https://github.com/inmusic-software/mpc-plugin-builder-sdk.

Read README.md and docs/architecture.md, docs/sdk-checklist.md, docs/roadmap.md before implementation. Read upstream SDK AGENTS.md, host-contract and UI references before choosing the wrapper or skin schema.

## First implementation
1. Pin a reviewed official SDK commit (do not vendor the SDK).
2. Check the real SDK plugin type and host APIs for effect audio input, note events, additional sidechain buses, program state and skin.
3. Build the standalone C++ DSP proof of concept and run the local offline test.
4. Adapt to the official SDK's actual plugin API, plugin manifest, skin and GitHub Actions workflow.
5. Validate in the host simulator; report commands and logs. Never claim hardware testing without evidence.

## DSP constraints
Do not allocate memory, lock, read files or log inside process(). Sample-rate changes, audio block sizes and full silence must work. Guard input levels and numerical stability. Keep analysis and carrier synthesis separated. Modulator = voice/sample, carrier = synthesizer/sound. No implicit access to audio from other plugin tracks.

Do not copy code from other repositories unless licensing and provenance are checked. Plugin branding: VocKit, Beats of Things. Select a unique 4-character SDK UID after checking collisions.
