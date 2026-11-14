#!/usr/bin/env python3
"""Create eight short, royalty-free synthetic voice-like test WAVs.
Replace these in Content/Samples with your own recordings after installation.
"""
from pathlib import Path
import math, wave, struct, sys
dest=Path(sys.argv[1]);dest.mkdir(parents=True,exist_ok=True)
sr=22050
for k in range(8):
    duration=.7+k*.09
    frames=int(sr*duration)
    out=bytearray()
    for n in range(frames):
        t=n/sr
        fundamental=95+17*k+8*math.sin(2*math.pi*4*t)
        # Harmonic glottal source with simple vowel-like resonances.
        v=0.
        f1=430+45*k;f2=1100+95*k
        for h in range(1,43):
            hz=h*fundamental
            if hz>=sr*.48:break
            resonance=math.exp(-((hz-f1)/220)**2)+.65*math.exp(-((hz-f2)/360)**2)
            v+=math.sin(2*math.pi*hz*t)/h*(.12+resonance)
        env=min(1.,t/.04)*min(1.,(duration-t)/.15)
        v=max(-1.,min(1.,v*.28*env))
        out.extend(struct.pack("<h",int(v*32767)))
    with wave.open(str(dest/f"sample{k+1}.wav"),"wb") as w:
        w.setnchannels(1);w.setsampwidth(2);w.setframerate(sr);w.writeframes(out)
print("Created 8 synthetic vowel test WAVs")
