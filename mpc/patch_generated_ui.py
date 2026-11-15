#!/usr/bin/env python3
"""Patch the proven SDK generator to add compact vertical spectral faders and a legible HELP tab.
The original generated structure and overlays remain intact.
"""
from pathlib import Path
import sys
p=Path(sys.argv[1])
s=p.read_text()
needle='    page_data = obj(\n'
assert s.count(needle)==1, "SDK generator layout changed: stop rather than ship broken UI"
injection=r'''
    # VocKit-specific safe extension: retain the SDK's valid TUI structure.
    def kv(o, key):
        return next(v for k, v in o if k == key)
    def put(o, key, value):
        for idx, (k, _) in enumerate(o):
            if k == key:
                o[idx] = (k, value)
                return
        raise KeyError(key)

    # A real vertical ColourBar slider with parameter name/value labels.
    vertical = slider_component(style, "knob overlay")
    vertical_children = kv(vertical, "componentsData")
    vertical_children[:] = [
        entry("bar", "Slider",
              obj(("version", 2), ("sliderType", "ColourBar"),
                  ("direction", "Vertical"), ("backgroundColour", "ff263443"),
                  ("outlineColour", "ff718096"), ("valueColour", "ffffb84d"),
                  ("isBipolar", False), ("handleName", "Data")),
              "28 36 38 266", "No", "Show", "Always"),
        entry("name", "Label", label_data("Name", "ffffb84d", 20.0, "horizontallyCentred verticallyCentred"),
              "0 0 95 30", "No", "Show", "Always"),
        entry("value", "Label", label_data("Value", "fff2f5fa", 20.0, "horizontallyCentred verticallyCentred"),
              "0 310 95 30", "No", "Show", "Always")
    ]
    defs.append(("vockitVerticalBand", vertical))
    for tab in tabs:
        name = kv(tab, "tabName")
        if name.startswith("BAND") and name[-1] in "123":
            n = int(name[-1])
            start = 19 + (n - 1) * 8
            children = []
            children.append(entry("heading", "Label",
                label_data("Text", "ffffb84d", 32.0, "left", "VOCKIT / 24-BAND SPECTRAL MIXER"),
                "45 12 1160 55", "No", "Show", "Always"))
            for i in range(8):
                children.append(entry("Band %02d" % ((n - 1) * 8 + i + 1),
                    "vockitVerticalBand", obj(("version", 1), ("handleName", "Data")),
                    "%d 116 95 340" % (35 + i * 150),
                    "Yes", "Hide", "Always",
                    remap=[("Data", "Parameter %d" % (start + i))]))
            put(kv(tab, "componentDefinition"), "componentsData", children)
            put(tab, "qlinkBoundsData", ["0 0 0 0"] * 4)
        elif name == "HELP":
            lines = [
                "VOCKIT FX  /  SETUP AND TIPS",
                "INTERNAL: Insert VocKit FX on the vocal audio track",
                "Select Internal carrier mode and set Mix to 100%",
                "JURA: Send synth and voice tracks to Submix 1",
                "Pan JURA hard LEFT and voice hard RIGHT",
                "Insert VocKit only on Submix 1",
                "Select L Carrier / R Voice in Input Mode",
                "MIC: Enable input monitoring; use headphones",
                "Bright saw carrier improves word clarity",
                "Short release works well for fast rap",
                "Avoid reverb before the vocoder",
                "Save and load presets from the MPC preset menu",
                "GUIDES: https://mpctoolkit.com"
            ]
            children = [
                entry("help %d" % i, "Label",
                      label_data("Text", "ffffb84d" if i in (0, 12) else "fff2f5fa",
                                 29.0 if i == 0 else 23.0, "left", line),
                      "44 %d 1160 39" % (15 + i * 44), "No", "Show", "Always")
                for i, line in enumerate(lines)
            ]
            put(kv(tab, "componentDefinition"), "componentsData", children)
'''
s=s.replace(needle,injection+"\n"+needle)
p.write_text(s)
print("Patched official generator with vertical VocKit band controls and one English HELP page")
