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
              "19 34 34 420", "No", "Show", "Always"),
        entry("name", "Label", label_data("Name", "ffffb84d", 20.0, "horizontallyCentred verticallyCentred"),
              "0 0 64 28", "No", "Show", "Always"),
        entry("value", "Label", label_data("Value", "fff2f5fa", 20.0, "horizontallyCentred verticallyCentred"),
              "0 466 64 30", "No", "Show", "Always")
    ]
    defs.append(("vockitVerticalBand", vertical))
    for tab in tabs:
        name = kv(tab, "tabName")
        if name in ("OSC", "FILTER"):
            children = kv(kv(tab, "componentDefinition"), "componentsData")
            bg = "osc_panels.png" if name == "OSC" else "filter_panels.png"
            children.insert(0, entry("module background", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", bg)),
                "0 0 1280 628", "No", "Show", "Always"))
            if name == "OSC":
                for title, y in (("OSC 1 / MAIN CARRIER + LFO", 11),
                                 ("OSC 2 / SECOND CARRIER", 471)):
                    children.append(entry("module heading " + title, "Label",
                        label_data("Text", "ffffb84d", 22.0, "left", title),
                        "42 %d 850 28" % y, "No", "Show", "Always",
                        remap=[("Data", "Parameter 0")]))
            else:
                for title, x in (("FILTER / CUTOFF + RESONANCE", 42),
                                 ("ENVELOPES / SYNTH + VOCODER", 664)):
                    children.append(entry("module heading " + title, "Label",
                        label_data("Text", "ffffb84d", 21.0, "left", title),
                        "%d 11 550 28" % x, "No", "Show", "Always",
                        remap=[("Data", "Parameter 0")]))
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
        elif name == "FILTER":
            children = kv(kv(tab, "componentDefinition"), "componentsData")
            # Two combined schematic diagrams. A single filmstrip cannot show
            # the combined effect of two independent parameters correctly.
            for title, filename, x in (
                ("FILTER RESPONSE  /  CUTOFF + RESONANCE", "filter_response.png", 25),
                ("SYNTH ENVELOPE  /  ATTACK + RELEASE", "synth_envelope.png", 650)):
                children.append(entry("diagram title " + title, "Label",
                    label_data("Text", "ffffb84d", 20.0, "left", title),
                    "%d 443 595 28" % x, "No", "Show", "Always"))
                children.append(entry("diagram " + title, "Image",
                    obj(("version", 2), ("imageType", "Regular"),
                        ("colour", "0"), ("image", filename)),
                    "%d 477 585 140" % x, "No", "Show", "Always"))
        elif name == "OSC":
            children = kv(kv(tab, "componentDefinition"), "componentsData")
            children.append(entry("lfo caption", "Label",
                label_data("Text", "ffffb84d", 23.0, "left",
                           "LFO: SINE / TRIANGLE / SQUARE - RATE + PITCH DEPTH"),
                "45 545 1160 35", "No", "Show", "Always"))
        elif name == "HELP":
            lines = [
                "VOCKIT FX  /  SETUP AND TIPS",
                "DIAG: Audio Monitor -> Listen Left / Listen Right",
                "Normal mode restores VocKit processing",
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
# The SDK generator writes skins before build.py validates them. Generate PNGs
# at that exact point, not in a later GitHub Actions step.
marker='    tui_path = os.path.join(out_dir, "TUI.json")'
assert s.count(marker)==1, "SDK generator output point changed"
s=s.replace(marker,
    '    import subprocess\n    from pathlib import Path\n'
    '    subprocess.run([sys.executable, str(Path(__file__).resolve().parents[2] / "mpc" / "generate_graphics.py"), out_dir], check=True)\n'
    +marker)

# Override the SDK generator's column-major default for the two custom
# 12-fader band pages. Physical Q-Link numbering is sequential here.
qlink_marker='    by_index = {p.index: p for p in params}'
assert s.count(qlink_marker)==1, "SDK Q-Link generator changed"
s=s.replace(qlink_marker, '''    for ti, page in enumerate(pages):
        if page.name in ("BAND1", "BAND2"):
            first = 19 if page.name == "BAND1" else 31
            qmap = next(value for key, value in entries[ti] if key == "Q-Links")
            qmap[:] = [("Q-Link %d" % q, first + q - 1 if q <= 12 else -1)
                       for q in range(1, 17)]
    ''' + qlink_marker)

p.write_text(s)
print("Patched official generator with vertical VocKit band controls and one English HELP page")
