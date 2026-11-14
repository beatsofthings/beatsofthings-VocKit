#!/usr/bin/env python3
"""VocKit companion-browser backend (prototype, no MPC launcher integration).
Read-only filesystem navigation; writes only a selected-path manifest.
"""
import argparse
import json
import os
from pathlib import Path
from urllib.parse import unquote

AUDIO_EXT={".wav"}
ALLOWED_FS={"vfat","exfat","ext4","ext3","ext2","ntfs","ntfs3","fuseblk","f2fs","btrfs","xfs"}
def drives():
    found={}
    with open("/proc/mounts",encoding="utf-8") as f:
        for line in f:
            p=line.split()
            if len(p)<3:continue
            dev,mount,fs=p[:3]
            mount=unquote(mount.replace("\\040","%20"))
            if fs not in ALLOWED_FS or not mount.startswith("/"):continue
            if mount in ("/","/boot","/boot/efi"):continue
            if not os.path.isdir(mount):continue
            found[mount]={"label":Path(mount).name or mount,"path":mount,"device":dev,"filesystem":fs}
    return sorted(found.values(),key=lambda x:x["path"])
def resolve(path):
    p=Path(path).resolve(strict=True)
    roots=[Path(d["path"]).resolve() for d in drives()]
    if not any(p==r or r in p.parents for r in roots):
        raise ValueError("Path outside available audio drives")
    return p
def list_dir(path):
    p=resolve(path)
    if not p.is_dir():raise ValueError("Not a directory")
    items=[]
    for child in p.iterdir():
        try:
            real=resolve(str(child))
            if real.is_dir():kind="folder"
            elif real.is_file() and real.suffix.lower() in AUDIO_EXT:kind="wav"
            else:continue
            items.append({"name":child.name,"path":str(real),"type":kind})
        except (OSError,ValueError):continue
    return sorted(items,key=lambda x:(x["type"]!="folder",x["name"].lower()))
def assign(pad,path,manifest):
    if not 1<=pad<=8:raise ValueError("Pad must be 1-8")
    p=resolve(path)
    if not p.is_file() or p.suffix.lower() not in AUDIO_EXT:raise ValueError("Select a WAV file")
    manifest=Path(manifest)
    state={"version":1,"pads":{}}
    if manifest.exists():
        state=json.loads(manifest.read_text(encoding="utf-8"))
    state.setdefault("pads",{})[str(pad)]=str(p)
    manifest.parent.mkdir(parents=True,exist_ok=True)
    tmp=manifest.with_suffix(".tmp")
    tmp.write_text(json.dumps(state,indent=2),encoding="utf-8")
    os.replace(tmp,manifest)
    return state
def main():
    parser=argparse.ArgumentParser()
    sub=parser.add_subparsers(dest="command",required=True)
    sub.add_parser("drives")
    sub.add_parser("list").add_argument("path")
    p=sub.add_parser("assign")
    p.add_argument("pad",type=int);p.add_argument("path")
    p.add_argument("--manifest",required=True)
    a=parser.parse_args()
    try:
        result=drives() if a.command=="drives" else list_dir(a.path) if a.command=="list" else assign(a.pad,a.path,a.manifest)
        print(json.dumps({"ok":True,"result":result},ensure_ascii=False))
    except (OSError,ValueError,PermissionError) as e:
        print(json.dumps({"ok":False,"error":str(e)},ensure_ascii=False))
        raise SystemExit(1)
if __name__=="__main__":main()
