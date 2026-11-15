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
              "26 30 38 145", "No", "Show", "Always"),
        entry("name", "Label", label_data("Name", "ffffb84d", 20.0, "horizontallyCentred verticallyCentred"),
              "0 0 64 28", "No", "Show", "Always"),
        entry("value", "Label", label_data("Value", "fff2f5fa", 20.0, "horizontallyCentred verticallyCentred"),
              "0 466 64 30", "No", "Show", "Always")
    ]
    defs.append(("vockitVerticalBand", vertical))
    for tab in tabs:
        name = kv(tab, "tabName")
        if name.startswith("BAND") and name[-1] in "123":
            n = int(name[-1])
            start = 19 + (n - 1) * 12
            children = []
            children.append(entry("heading", "Label",
                label_data("Text", "ffffb84d", 32.0, "left", "VOCKIT / 24-BAND SPECTRAL MIXER"),
                "45 12 1160 55", "No", "Show", "Always"))
            for i in range(12):
                children.append(entry("Band %02d" % ((n - 1) * 12 + i + 1),
                    "vockitVerticalBand", obj(("version", 1), ("handleName", "Data")),
                    "%d 60 64 520" % (28 + i * 102),
                    "Yes", "Hide", "Always",
                    remap=[("Data", "Parameter %d" % (start + i))]))
            put(kv(tab, "componentDefinition"), "componentsData", children)
            put(tab, "qlinkBoundsData", ["0 0 0 0"] * 4)
        elif name in ("FILTER", "OSC"):
            children = kv(kv(tab, "componentDefinition"), "componentsData")
            # Static schematic guides, not live modulation readouts.
            def line(x, y, w, h, vertical=False):
                return entry("graphic %d" % len(children), "Separator",
                    obj(("version", 1), ("colour", "ffffb84d"),
                        ("separatorType", 0 if vertical else 1)),
                    "%d %d %d %d" % (x, y, w, h), "No", "Show", "Always")
            if name == "FILTER":
                children.append(entry("filter curve title", "Label",
                    label_data("Text", "ffffb84d", 22.0, "left", "FILTER RESPONSE"),
                    "45 465 490 32", "No", "Show", "Always"))
                children.append(entry("envelope title", "Label",
                    label_data("Text", "ffffb84d", 22.0, "left", "ENVELOPE SHAPE"),
                    "660 465 500 32", "No", "Show", "Always"))
                # Low-pass slope, with resonance peak.
                points = [(60,525),(160,525),(240,520),(300,495),(320,510),(360,535),(430,558),(530,570)]
                # Attack, sustain and release envelope silhouette.
                env_points = [(670,580),(710,505),(790,505),(840,530),(970,530),(1080,580),(1170,580)]
                for pts in (points, env_points):
                    for (x1,y1),(x2,y2) in zip(pts,pts[1:]):
                        children.append(line(x1,y1,max(2,x2-x1),3))
                        children.append(line(x2,min(y1,y2),3,max(2,abs(y2-y1)),True))
            else:
                children.append(entry("lfo title", "Label",
                    label_data("Text", "ffffb84d", 22.0, "left", "LFO WAVEFORM / PITCH MODULATION"),
                    "55 468 1120 30", "No", "Show", "Always"))
                # Simple stepped triangular LFO graphic, static.
                points=[(80,565),(160,510),(240,565),(320,510),(400,565),(480,510)]
                for (x1,y1),(x2,y2) in zip(points,points[1:]):
                    children.append(line(x1,y1,max(2,x2-x1),3))
                    children.append(line(x2,min(y1,y2),3,max(2,abs(y2-y1)),True))
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
                      "44 %d 1160 39" % (15 + i * 44), "No", "Show", "Always",
                      remap=[("Data", "Parameter 0")])
                for i, line in enumerate(lines)
            ]
            put(kv(tab, "componentDefinition"), "componentsData", children)
'''
s=s.replace(needle,injection+"\n"+needle)
p.write_text(s)
print("Patched official generator with vertical VocKit band controls and one English HELP page")
