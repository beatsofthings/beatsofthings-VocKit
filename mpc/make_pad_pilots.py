#!/usr/bin/env python3
"""Create four mono PCM WAV pilot tones for VocKit PADS test.
Run: python mpc/make_pad_pilots.py
"""
from pathlib import Path
import math, struct, wave
rate=48000
duration=1.0
out=Path("pad-pilots")
out.mkdir(exist_ok=True)
for pad,freq in (("A02",750),("A03",1000),("A04",1250),("A05",1500)):
    path=out/f"VocKit_{pad}_{freq}Hz.wav"
    with wave.open(str(path),"wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(rate)
        samples=[]
        for i in range(int(rate*duration)):
            t=i/rate
            fade=min(1.,t/.005,(duration-t)/.005)
            value=.5*max(0.,fade)*math.sin(2*math.pi*freq*t)
            samples.append(struct.pack("<h",round(value*32767)))
        wav.writeframes(b"".join(samples))
    print(path)
