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

# Three separate filter/envelope modules, no misleading static response curves.
im=Image.new("RGBA",(1280,628),(0,0,0,0))
d=ImageDraw.Draw(im)
for top,bottom in ((8,193),(207,397),(411,616)):
    d.rounded_rectangle((20,top,1260,bottom),radius=13,
        fill=(21,36,53,225),outline=(64,91,113,255),width=2)
    d.line((42,top+43,1236,top+43),fill=(255,184,77,130),width=2)
im.save(out/"filter_sections.png",optimize=True)
# Three reading columns on the HELP tab.
im=Image.new("RGBA",(1280,628),(0,0,0,0))
d=ImageDraw.Draw(im)
for left,right in ((16,424),(436,844),(856,1264)):
    d.rounded_rectangle((left,12,right,612),radius=13,
        fill=(21,36,53,230),outline=(64,91,113,255),width=2)
    d.line((left+15,66,right-15,66),fill=(255,184,77,130),width=2)
im.save(out/"help_columns.png",optimize=True)

# Decode the original user-supplied VocKit artwork (stored as base64 text).
# The image is pre-sized for MPC to avoid a large resource footprint.
import base64
source=Path(__file__).resolve().parent / "assets" / "vockit_uploaded_logo.b64"
if not source.is_file():
    raise FileNotFoundError("Uploaded VocKit logo asset missing: " + str(source))
logo=base64.b64decode(source.read_text().strip(), validate=True)
from io import BytesIO
check=Image.open(BytesIO(logo))
check.verify()
# Crop the large transparent margins in the uploaded PNG. The MPC Image
# widget scales the full canvas; margins previously made the logo microscopic.
art=Image.open(BytesIO(logo)).convert("RGBA")
alpha=art.getchannel("A")
bbox=alpha.point(lambda x: 255 if x>24 else 0).getbbox()
if not bbox:
    raise ValueError("VocKit logo has no visible pixels")
art=art.crop(bbox)
# Keep aspect ratio and avoid a huge texture.
art.thumbnail((800,220),Image.Resampling.LANCZOS)
art.save(out / "vockit_logo.png",optimize=True)
print("Cropped uploaded logo to",art.size)

# Original vintage-poly-synth-inspired background: charcoal chassis, warm
# copper/cream accent stripes and subtle grain; no third-party artwork.
im=Image.new("RGBA",(1280,628),(13,20,29,255))
d=ImageDraw.Draw(im)
for y in range(628):
    shade=int(9+12*y/628)
    d.line((0,y,1279,y),fill=(shade+4,shade+11,shade+20,255))
for y in (8,12,615,620):
    d.line((0,y,1279,y),fill=(167,106,53,135),width=2)
for y in range(45,600,108):
    d.line((25,y,1254,y),fill=(55,73,88,55),width=1)
for x in range(0,1280,26):
    d.line((x,0,x,628),fill=(47,59,69,24),width=1)
# Warm left rail reminiscent of classic analog synthesizer panel graphics.
d.rounded_rectangle((8,42,15,582),radius=3,fill=(183,114,59,165))
im.save(out/"vockit_vintage_background.png",optimize=True)

# Dedicated FX modules: saturation, stereo chorus, and voice shaping.
im=Image.new("RGBA",(1280,628),(0,0,0,0))
d=ImageDraw.Draw(im)
for left,right in ((18,392),(405,860),(873,1262)):
    d.rounded_rectangle((left,22,right,608),radius=14,
        fill=(21,36,53,230),outline=(75,96,113,255),width=2)
    d.line((left+17,72,right-17,72),fill=(220,142,71,170),width=2)
im.save(out/"fx_sections.png",optimize=True)
