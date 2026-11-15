#!/usr/bin/env python3
"""Generate parameter-indexed MPC filmstrips for the FILTER page.
Each graphic represents its own parameter, not a combined DSP response.
"""
import sys, math
from pathlib import Path
from PIL import Image, ImageDraw
out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
W,H,N=440,105,48
def render(name,fn,title):
    strip=Image.new("RGBA",(W,H*N),(0,0,0,0))
    for frame in range(N):
        v=frame/(N-1)
        im=Image.new("RGBA",(W,H),(0,0,0,0));d=ImageDraw.Draw(im)
        d.rounded_rectangle((1,1,W-2,H-2),radius=9,fill=(17,30,44,255),outline=(55,75,90,255),width=2)
        d.rectangle((12,4,W-12,25),fill=(17,30,44,255))
        d.text((16,7),title,fill=(255,184,77,255))
        for y in (34,56,79):d.line((16,y,W-16,y),fill=(42,57,71,255),width=1)
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
    render(name,fn,{"filter_cutoff.png":"FILTER CUTOFF","filter_resonance.png":"FILTER RESONANCE","env_attack.png":"ENVELOPE ATTACK","env_release.png":"ENVELOPE RELEASE"}[name])
print("Created four parameter-driven 48-frame filter/envelope displays in",out)

# Combined, explicitly schematic diagrams. These are not falsely presented
# as a complete response computed from two independent controls.
def schematic(filename,title,points,labels):
    im=Image.new("RGBA",(580,160),(0,0,0,0));d=ImageDraw.Draw(im)
    d.rounded_rectangle((1,1,578,158),radius=12,fill=(17,30,44,255),outline=(65,85,102,255),width=2)
    d.text((16,10),title,fill=(255,184,77,255))
    for x in range(20,565,54):d.line((x,35,x,139),fill=(42,57,71,255))
    for y in (44,75,106,137):d.line((20,y,560,y),fill=(42,57,71,255))
    d.line(points,fill=(255,184,77,255),width=4,joint="curve")
    for x,y,label in labels:d.text((x,y),label,fill=(220,232,240,255))
    im.save(out/filename,optimize=True)
schematic("filter_response.png","FILTER RESPONSE  /  SCHEMATIC",
    [(24,54),(190,54),(260,48),(300,42),(330,67),(380,112),(445,134),(555,134)],
    [(245,15,"RESONANCE"),(345,113,"CUTOFF")])
schematic("synth_envelope.png","SYNTH ENVELOPE  /  SCHEMATIC",
    [(24,137),(135,47),(370,47),(555,137)],
    [(70,112,"ATTACK"),(445,112,"RELEASE")])

# Subtle transparent panel graphics placed BEHIND the SDK-generated controls.
def panel_background(filename,sections):
    im=Image.new("RGBA",(1280,628),(0,0,0,0))
    d=ImageDraw.Draw(im)
    for x0,y0,x1,y1 in sections:
        d.rounded_rectangle((x0,y0,x1,y1),radius=14,fill=(20,35,52,115),outline=(55,82,103,200),width=2)
        d.line((x0+16,y0+34,x1-16,y0+34),fill=(255,184,77,95),width=2)
    im.save(out/filename,optimize=True)
panel_background("osc_panels.png",[(18,8,1260,462),(18,469,1260,618)])
panel_background("filter_panels.png",[(18,8,625,438),(645,8,1260,438)])
