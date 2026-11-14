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
    label("note","Waveform display requires host data access; pads show decorative waveforms",28,400,970,34,19,"ff9ab0be"),
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

# The 24 bands are actual editable DSP gains, grouped into two pages of 12.
def band_page(start):
    nodes=[label("title","24-BAND VOICE ENGINE  /  BANDS %02d-%02d"%(start+1,start+12),25,8,970,48,31,"ff72dfea"),
           label("explain","Each band controls its synthesis level (0-200%).",25,62,960,34,21,"ff9ab0be")]
    for j in range(12):
        x=25+(j%4)*244
        y=118+(j//4)*156
        band=start+j
        nodes.append(label("bandname"+str(band),"BAND %02d"%(band+1),x,y,215,30,23))
        nodes.append(slider("band"+str(band),13+band,x,y+35,222,89))
    return {"version":2,"actions":[],"backgroundData":background("ff15222d"),"ignoreMousePresses":False,"disableCoarseDataWheel":False,"componentsData":nodes}

vocoder={"version":2,"actions":[],"backgroundData":background("ff15222d"),"ignoreMousePresses":False,"disableCoarseDataWheel":False,"componentsData":[
    label("title","24-BAND VOICE ENGINE",30,10,940,55,36,"ff72dfea"),
    label("description","24 analysis filters  >  envelope followers  >  24 carrier filters",30,75,1000,50,23),
    label("formant_label","FORMANT SHIFT (SEMITONES)",30,160,480,45,25),
    slider("formant",12,30,210,420,110),
    label("attack_label","ENVELOPE ATTACK (MS)",510,160,460,45,25),
    slider("attack",5,510,210,420,110),
    label("release_label","ENVELOPE RELEASE (MS)",30,355,460,45,25),
    slider("release",6,30,410,420,110),
    label("gain_label","MODULATOR INPUT LEVEL",510,355,460,45,25),
    slider("input",9,510,410,420,110)]}
tui={"pageData":{"version":1,"componentDefinitions":{"version":2,"importFiles":[],"localComponentDefinitions":[]},
"replacementStrings":[{"key":"Parameter 1","value":[f"Pad {i}" for i in range(1,9)]},{"key":"Parameter 2","value":["Off","On"]}],
"info":{"version":1,"type":"CompleteDescription"},
"tabs":[{"version":1,"tabName":name,"fnKeyIndex":i,"fnKeySubIndex":0,"componentDefinition":page,"qlinkBoundsData":[]}
for i,(name,page) in enumerate([("SAMPLES",root),("CARRIER",voice),("VOCODER",vocoder),("BAND1",band_page(0)),("BAND2",band_page(12))])]}}
(dest/"TUI.json").write_text(json.dumps(tui,indent=2)+"\n")
# Map every control to its stable parameter index; unused Q-links are -1.
qlinks={"version":1,"default":{}}
# Q-Link schema is supplied by SDK's own generator, then customized separately.
print("Generated custom sample-pad TUI.json and 16 button PNGs")
