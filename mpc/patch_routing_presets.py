#!/usr/bin/env python3
"""Keep the VocKit FX stereo routing independent of factory sound programs.

The official MPC SDK's Plugin::setProgram() resets every parameter to the
program value. Routing must be an instance/project setting, not a sound preset.
This narrowly scoped build-time patch applies only to the VocKit FX SDK copy.
Saved project state still restores the Input Mode normally.
"""
from pathlib import Path
import sys

path = Path(sys.argv[1])
source = path.read_text()
old = "for (int k = 0; k < numParams() && k < (int)p.plainValues.size(); ++k) set(k, p.plainValues[(size_t)k]);"
new = """for (int k = 0; k < numParams() && k < (int)p.plainValues.size(); ++k) {
            // VocKit FX: sound presets must not change stereo carrier/voice routing.
            if (params_[(size_t)k].id == "mode") continue;
            set(k, p.plainValues[(size_t)k]);
        }"""
assert source.count(old) == 1, "Official SDK setProgram implementation changed; review required"
source = source.replace(old, new)
path.write_text(source)
print("VocKit FX Input Mode is preserved when switching factory programs")
