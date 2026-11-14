#!/usr/bin/env python3
"""Generate an MPC scripted touch skin with eight radio sample pads.
Requires Pillow. No live waveform API is available through SDK skins.
"""
import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import sys

dest = Path(sys.argv[1])
dest.mkdir(parents=True, exist_ok=True)
fontpath = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
font = ImageFont.truetype(fontpath, 22)

def pad_image(index, active):
    img = Image.new("RGBA", (218, 104), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    bg = "#136d82" if active else "#253642"
    edge = "#49d6e8" if active else "#516575"
    d.rounded_rectangle((2,2,215,101), radius=13, fill=bg, outline=edge, width=3)
    d.text((17,13), f"VOICE {index+1:02d}", font=font, fill="#f5faff")
    # Decorative sample waveform, not an audio-derived waveform.
    import math
    for x in range(15,202,3):
        amp=6+int(11*abs(math.sin(x*.19+index)*math.sin(x*.051)))
        d.line((x,70-amp,x,70+amp), fill="#63e2f1", width=2)
    img.save(dest/f"pad{index+1}_{'on' if active else 'off'}.png")

for i in range(8):
    pad_image(i,False)
    pad_image(i,True)

def background(color="0"):
    return {"version":1,"focussed":{"version":1,"colour":color,"image":""},"unfocussed":{"version":1,"colour":color,"image":""}}
def bounds(x,y,w,h,focus="No"):
    return {"version":1,"acceptsHWFocus":focus,"showWhenDataModelInvalid":"Show","whenVisible":"Always","boundsType":"Absolute","bounds":f"{x} {y} {w} {h}"}
def entry(name,typ,data,x,y,w,h,handle=None,focus="No"):
    return {"version":2,"componentData":{"version":1,"name":name,"type":typ,"data":data},
      "handle remapping":{"version":1,"map":([{"key":"Data","value":f"Parameter {handle}"}] if handle is not None else [])},
      "bounds":bounds(x,y,w,h,focus)}
def label(name,text,x,y,w,h,size=24,color="ffe8f7fb"):
    style={"version":1,"font":{"version":1,"name":"Titillium Web","style":"Bold","height":float(size)},"colour":color,"justification":"left verticallyCentred","case":"Original"}
    return entry(name,"Label",{"version":1,"textStyle":style,"type":"Text","handleName":"Invalid","text":text},x,y,w,h)
def slider(name,param,x,y,w=235,h=85):
    return entry(name,"Slider",{"version":1,"handleName":"Data"},x,y,w,h,param,"Yes")
def knob(name,param,x,y):
    return entry(name,"Knob",{"version":5,"knobType":"GenericLegacy","polarity":"UniPolar","dragOrientation":"Vertical","handleName":"Data"},x,y,150,130,param,"Yes")

children=[label("title","VOCKIT SYNTH  /  SAMPLE BANK",30,10,800,48,34,"ff72dfea")]
for i in range(8):
    x=28+(i%4)*245
    y=80+(i//4)*127
    children.append(entry(f"pad{i+1}","Button",{"version":2,"onImage":f"pad{i+1}_on.png","offImage":f"pad{i+1}_off.png","buttonId":i,"numButtonsInGroup":8,"handleName":"Data","gestureBehaviour":"Instant"},x,y,218,104,1,"Yes"))
children += [
    label("wave","SAMPLE PREVIEW  |  WAV SOURCE",28,351,650,45,25),
    label("note","YOUR WAVS: Content/User Samples  |  8 FILES, SORTED A-Z  |  RELOAD PLUGIN",28,400,970,34,19,"ff9ab0be"),
    slider("speed",3,25,464),slider("loop",2,270,464),slider("preview",10,515,464),slider("level",0,760,464)
]
root={"version":2,"actions":[],"backgroundData":background("ff15222d"),"ignoreMousePresses":False,"disableCoarseDataWheel":False,"componentsData":children}
voice={"version":2,"actions":[],"backgroundData":background("ff15222d"),"ignoreMousePresses":False,"disableCoarseDataWheel":False,
"componentsData":[label("title","VOCKIT SYNTH  /  CARRIER",30,10,900,55,34,"ff72dfea"),
label("trans_label","TRANSPOSE (SEMITONES)",25,80,210,35,20),knob("pitch",4,25,110),
label("tone_label","SAW / PULSE BLEND",265,80,210,35,20),knob("tone",8,265,110),
label("attack_label","VOICE ATTACK (MS)",505,80,210,35,20),knob("attack",5,505,110),
label("release_label","VOICE RELEASE (MS)",745,80,210,35,20),knob("release",6,745,110),
slider("mix",7,25,340),slider("level",0,275,340),slider("input",9,525,340)]}

# All 24 editable synthesis bands in one portrait-oriented equalizer.
# The bars are real MPC ColourBar sliders, not a static illustration.
def vertical_band(index):
    x=26+index*40
    hue="ff32cce3" if index<12 else "ffffba5b"
    return [
        label("freq"+str(index),"%02d"%(index+1),x,142,38,26,18,"ffb9d3de"),
        entry("band"+str(index),"Slider",{
            "version":2,"sliderType":"ColourBar","direction":"Vertical",
            "backgroundColour":"ff263a46","outlineColour":"ff48606d",
            "valueColour":hue,"isBipolar":False,"handleName":"Data"
        },x,180,30,265,13+index,"Yes")
    ]

bands=[label("title","VOCKIT  /  24-BAND VOICE ENGINE",25,7,1000,52,34,"ff72dfea"),
       label("subtitle","SPECTRAL SHAPING  -  DRAG VERTICAL BARS",25,65,1000,38,21,"ffadc3d1"),
       label("low","LOW  140 Hz",25,105,300,30,21,"ff4dd8e9"),
       label("mid","MID",444,105,120,30,21,"ffb9d3de"),
       label("high","HIGH  6.8 kHz",780,105,230,30,21,"ffffba5b")]
for i in range(24):
    bands.extend(vertical_band(i))
bands.extend([
    label("formant_label","FORMANT SHIFT",25,474,300,34,21),
    slider("formant",12,25,511,285,86),
    label("attack_label","ATTACK",355,474,220,34,21),
    slider("attack",5,355,511,275,86),
    label("release_label","RELEASE",678,474,220,34,21),
    slider("release",6,678,511,275,86)])
vocoder={"version":2,"actions":[],"backgroundData":background("ff15222d"),
         "ignoreMousePresses":False,"disableCoarseDataWheel":False,"componentsData":bands}
filesPage={"version":2,"actions":[],"backgroundData":background("ff15222d"),
"ignoreMousePresses":False,"disableCoarseDataWheel":False,
"componentsData":[label("title","VOCKIT / DRIVE  >  FOLDER  >  WAV",25,8,980,52,34,"ff72dfea"),
label("hint","File menus show DRIVE / FOLDER / FILENAME. Reload to rescan drives.",25,60,1010,38,20,"ffadc3d1")]
+sum(([label("filelabel"+str(i),"PAD %02d / WAV FILE"%(i+1),25+(i%2)*495,115+(i//2)*125,460,30,22),
slider("file"+str(i),37+i,25+(i%2)*495,150+(i//2)*125,445,80)] for i in range(8)),[])}
tui={"pageData":{"version":1,"componentDefinitions":{"version":2,"importFiles":[],"localComponentDefinitions":[]},
"replacementStrings":[{"key":"Parameter 1","value":[f"Pad {i}" for i in range(1,9)]},{"key":"Parameter 2","value":["Off","On"]}],
"info":{"version":1,"type":"CompleteDescription"},
"tabs":[{"version":1,"tabName":name,"fnKeyIndex":i,"fnKeySubIndex":0,"componentDefinition":page,"qlinkBoundsData":[]}
for i,(name,page) in enumerate([("SAMPLES",root),("CARRIER",voice),("VOCODER",vocoder),("FILES",filesPage)])]}}
(dest/"TUI.json").write_text(json.dumps(tui,indent=2)+"\n")
# Map every control to its stable parameter index; unused Q-links are -1.
qlinks={"version":1,"default":{}}
# Q-Link schema is supplied by SDK's own generator, then customized separately.
print("Generated custom sample-pad TUI.json and 16 button PNGs")
