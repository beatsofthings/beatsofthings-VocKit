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
        # Underlay: independent original vintage-synth inspired background.
        # Insert first, so native controls and their focus overlays stay visible.
        tab_children = kv(kv(tab, "componentDefinition"), "componentsData")
        tab_children.insert(0, entry("VocKit vintage background", "Image",
            obj(("version", 2), ("imageType", "Regular"),
                ("colour", "0"), ("image", "vockit_vintage_background.png")),
            "0 0 1280 628", "No", "Show", "Always"))
        if name == "VOICE":
            children = kv(kv(tab, "componentDefinition"), "componentsData")
            children.append(entry("VocKit BeatsOfThings logo", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", "vockit_logo.png")),
                "805 492 410 110", "No", "Show", "Always"))
        if name == "OSC":
            children = kv(kv(tab, "componentDefinition"), "componentsData")
            children.insert(0, entry("osc background", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", "osc_panels.png")),
                "0 0 1280 628", "No", "Show", "Always"))
            for title, y in (("OSC 1 / MAIN CARRIER + LFO", 11),
                             ("OSC 2 / SECOND CARRIER", 471)):
                children.append(entry("osc heading " + title, "Label",
                    label_data("Text", "ffffb84d", 22.0, "left", title),
                    "42 %d 850 28" % y, "No", "Show", "Always",
                    remap=[("Data", "Parameter 0")]))
        if name.startswith("BAND") and name[-1] in "123":
            n = int(name[-1])
            start = 19 + (n - 1) * 12
            children = [entry("vintage background", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", "vockit_vintage_background.png")),
                "0 0 1280 628", "No", "Show", "Always")]
            children.append(entry("heading", "Label",
                label_data("Text", "ffffb84d", 32.0, "left", "VOCKIT / 24-BAND SPECTRAL MIXER"),
                "45 12 1160 55", "No", "Show", "Always"))
            for i in range(12):
                children.append(entry("Band %02d" % ((n - 1) * 12 + i + 1),
                    "vockitVerticalBand", obj(("version", 1), ("handleName", "Data")),
                    "%d 60 64 520" % (28 + i * 102),
                    "Yes", "Hide", "Always",
                    remap=[("Data", "Parameter %d" % (start + i))]))
            children.append(entry("VocKit band page logo", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", "vockit_logo.png")),
                "1040 574 196 45", "No", "Show", "Always"))
            put(kv(tab, "componentDefinition"), "componentsData", children)
            put(tab, "qlinkBoundsData", ["0 0 0 0"] * 4)
        elif name == "FX":
            originals = kv(kv(tab, "componentDefinition"), "componentsData")
            children = [entry("FX vintage background", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", "vockit_vintage_background.png")),
                "0 0 1280 628", "No", "Show", "Always"),
                entry("FX module background", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", "fx_sections.png")),
                "0 0 1280 628", "No", "Show", "Always")]
            def fx_heading(title, x):
                children.append(entry("fx heading " + title, "Label",
                    label_data("Text", "ffffb84d", 24.0, "left", title),
                    "%d 30 390 40" % x, "No", "Show", "Always",
                    remap=[("Data", "Parameter 0")]))
            def fx_control(label, x, y, width=300):
                original = next((item for item in originals
                    if kv(kv(item, "componentData"), "name") == label), None)
                if original is None:
                    raise ValueError("Missing SDK-generated FX control: " + label)
                put(kv(original, "bounds"), "bounds",
                    "%d %d %d 126" % (x, y, width))
                children.append(original)
            fx_heading("DRIVE / RING MOD", 42)
            fx_control("Saturation", 48, 105, 305)
            fx_control("Ring Mix", 48, 275, 305)
            fx_control("Ring Frequency", 48, 445, 305)
            fx_heading("STEREO CHORUS", 426)
            fx_control("Chorus Mix", 432, 108, 380)
            fx_control("Chorus Rate", 432, 271, 380)
            fx_control("Chorus Depth", 432, 434, 380)
            fx_heading("VOICE SHAPING", 893)
            fx_control("Band Profile", 892, 110, 340)
            fx_control("Profile Amount", 892, 266, 340)
            fx_control("Voice Character", 892, 424, 340)
            put(kv(tab, "componentDefinition"), "componentsData", children)
        elif name == "FILTER":
            # The generator's 4x4 layout mixes filter and envelope controls.
            # Rebuild only this tab using the SDK's proven control components,
            # with explicit parameter binding and grouped rows.
            originals = kv(kv(tab, "componentDefinition"), "componentsData")
            children = [entry("vintage background", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", "vockit_vintage_background.png")),
                "0 0 1280 628", "No", "Show", "Always"),
                entry("filter panel background", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", "filter_sections.png")),
                "0 0 1280 628", "No", "Show", "Always")]
            def section(title, y):
                children.append(entry("section " + title, "Label",
                    label_data("Text", "ffffb84d", 27.0, "left", title),
                    "45 %d 1140 40" % y, "No", "Show", "Always",
                    remap=[("Data", "Parameter 0")]))
            def control(label, x, y):
                original = next((item for item in originals
                    if kv(kv(item, "componentData"), "name") == label), None)
                if original is None:
                    raise ValueError("Missing SDK-generated FILTER control: " + label)
                put(kv(original, "bounds"), "bounds", "%d %d 278 107" % (x, y))
                children.append(original)
            section("VOCODER ENVELOPE  /  SPEECH TRACKING", 110)
            control("Attack", 110, 230)
            control("Release", 670, 230)
            put(kv(tab, "componentDefinition"), "componentsData", children)
        elif name == "OSC":
            children = kv(kv(tab, "componentDefinition"), "componentsData")
            children.append(entry("lfo caption", "Label",
                label_data("Text", "ffffb84d", 23.0, "left",
                           "LFO: SINE / TRIANGLE / SQUARE - RATE + PITCH DEPTH"),
                "45 545 1160 35", "No", "Show", "Always"))
        elif name == "HELP":
            # Three independent reading columns; all labels bind to an existing
            # valid handle because unbound SDK text labels can render blank.
            columns = [
                ("SETUP / ROUTING", [
                    "INTERNAL CARRIER:",
                    "Insert VocKit on the vocal track",
                    "Input Mode: Internal",
                    "Mix 100% - adjust Carrier level",
                    "EXTERNAL JURA:",
                    "Route voice + JURA to Submix 1",
                    "JURA LEFT, voice RIGHT:",
                    "Mode: L Carrier / R Voice",
                    "Voice LEFT, JURA RIGHT:",
                    "Mode: R Carrier / L Voice",
                    "DIAG: Audio Monitor L / R",
                    "Return Monitor to Normal",
                    "Use headphones for microphone"]),
                ("24-BAND SPECTRAL MIXER", [
                    "Each slider shapes one frequency",
                    "band of the vocoded output",
                    "01-08: bass / warmth / body",
                    "09-16: vowels / intelligibility",
                    "17-24: brightness / presence",
                    "100 = neutral band level",
                    "0 = remove that frequency band",
                    "200 = boost that band",
                    "Raise mids for clearer vowels",
                    "Lower lows to reduce muddiness",
                    "Use high bands for brightness",
                    "Bands are NOT a dry vocal EQ",
                    "BAND1: 01-12; BAND2: 13-24"]),
                ("SYNTH / SOUND RECIPES", [
                    "CLASSIC ROBOT:",
                    "Saw/Pulse; Single; short Attack",
                    "Release 70-120; bright carrier",
                    "ELECTRO FUNK:",
                    "Saw + OSC2 Square at 20-40%",
                    "Detune OSC2 5-12 cents",
                    "SOFT CHOIR:",
                    "Triangle; Major chord",
                    "Slow Synth Attack + Release",
                    "DEEP MACHINE:",
                    "Formant Shift -4 to -8",
                    "Sub Osc 15-30; lower Cutoff",
                    "Guides: mpctoolkit.com"])
            ]
            children = [entry("vintage background", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", "vockit_vintage_background.png")),
                "0 0 1280 628", "No", "Show", "Always"),
                entry("help panel background", "Image",
                obj(("version", 2), ("imageType", "Regular"),
                    ("colour", "0"), ("image", "help_columns.png")),
                "0 0 1280 628", "No", "Show", "Always")]
            for col, (heading, lines) in enumerate(columns):
                x = 34 + col * 420
                children.append(entry("help heading %d" % col, "Label",
                    label_data("Text", "ffffb84d", 22.0, "left", heading),
                    "%d 27 375 34" % x, "No", "Show", "Always",
                    remap=[("Data", "Parameter 0")]))
                for i, line in enumerate(lines):
                    children.append(entry("help %d %d" % (col, i), "Label",
                        label_data("Text", "fff2f5fa", 18.0, "left", line),
                        "%d %d 375 31" % (x, 83 + i * 38),
                        "No", "Show", "Always",
                        remap=[("Data", "Parameter 0")]))
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
