#!/usr/bin/env python3
"""Generate parameter-indexed MPC filmstrips for the FILTER page.
Each graphic represents its own parameter, not a combined DSP response.
"""
import sys, math
from pathlib import Path
from PIL import Image, ImageDraw
out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
W,H,N=440,105,48
def render(name,fn):
    strip=Image.new("RGBA",(W,H*N),(0,0,0,0))
    for frame in range(N):
        v=frame/(N-1)
        im=Image.new("RGBA",(W,H),(0,0,0,0));d=ImageDraw.Draw(im)
        d.rounded_rectangle((1,1,W-2,H-2),radius=9,fill=(17,30,44,255),outline=(55,75,90,255),width=2)
        for y in (27,52,77):d.line((16,y,W-16,y),fill=(42,57,71,255),width=1)
        for x in range(16,W-15,52):d.line((x,14,x,H-14),fill=(42,57,71,255),width=1)
        pts=[(x,fn(x/W,v)) for x in range(17,W-16,3)]
        d.line(pts,fill=(255,184,77,255),width=4,joint="curve")
        strip.paste(im,(0,frame*H))
    strip.save(out/name,optimize=True)
def cutoff(x,v):
    knee=.10+.78*v
    t=max(0,(x-knee)*5)
    return 25+min(61,t*61)
def resonance(x,v):
    knee=.58
    peak=40*v*math.exp(-((x-knee)/.075)**2)
    return 37-peak+min(47,max(0,(x-knee)*5)*47)
def attack(x,v):
    attack_fraction=.06+.72*v
    return 84-65*min(1,x/attack_fraction)
def release(x,v):
    start=.25
    if x<start:return 20
    return 20+65*min(1,(x-start)/(.08+.65*v))
for name,fn in [("filter_cutoff.png",cutoff),("filter_resonance.png",resonance),("env_attack.png",attack),("env_release.png",release)]:
    render(name,fn)
print("Created four parameter-driven 48-frame filter/envelope displays in",out)
