#!/usr/bin/env python3
from pathlib import Path
from PIL import Image, ImageDraw
R=Path(__file__).resolve().parents[1]
T=(128,208,224)
CR=[T,(24,24,32),(232,88,24),(248,144,40),(248,232,184),(216,184,144),(248,224,64),(248,96,24),(40,64,104),(64,104,152),(0,0,0),(248,184,32),(248,248,248),(104,64,40),(112,104,104),(160,56,24)]
CS=[T,(24,24,32),(72,120,184),(96,168,216),(232,240,248),(168,192,208),(184,248,240),(40,208,208),(64,48,112),(104,80,168),(0,0,0),(152,248,232),(248,248,248),(72,64,104),(120,120,144),(48,88,152)]
TX=[T,(24,24,32),(116,84,132),(160,116,176),(92,72,56),(132,104,80),(184,216,48),(224,240,72),(56,48,48),(216,184,144),(0,0,0),(244,236,212),(176,104,64),(120,152,48),(72,56,88),(192,152,112)]
TS=[T,(24,24,32),(56,124,120),(88,176,160),(80,72,64),(128,112,96),(248,152,40),(248,200,64),(48,48,48),(216,192,160),(0,0,0),(244,236,212),(176,88,56),(176,104,40),(48,88,88),(184,152,120)]

def img(p,s=(64,64)):
    a=Image.new("P",s,0);q=sum((list(x) for x in p),[])+[0]*(768-48);a.putpalette(q);return a
def flame(d,x,y,k=1):
    d.polygon([(x,y),(x-int(5*k),y+int(8*k)),(x-int(3*k),y+int(14*k)),(x,y+int(10*k)),(x+int(3*k),y+int(15*k)),(x+int(6*k),y+int(8*k)),(x+int(2*k),y+int(4*k))],fill=7,outline=1)
    d.polygon([(x,y+int(4*k)),(x-int(2*k),y+int(9*k)),(x,y+int(12*k)),(x+int(2*k),y+int(9*k))],fill=6)
def crab(n,back=False):
    a=img(CR);d=ImageDraw.Draw(a)
    if n==0:
        for x in (12,19,45,52):d.line([(x,43),(x+(-7 if x<32 else 7),54)],fill=1,width=4);d.line([(x,43),(x+(-7 if x<32 else 7),54)],fill=2,width=2)
        d.ellipse((6,34,23,52),fill=2,outline=1);d.ellipse((41,34,58,52),fill=2,outline=1);d.ellipse((15,25,49,49),fill=2,outline=1);d.ellipse((22,38,42,49),fill=4,outline=1);d.ellipse((20,21,44,33),fill=4,outline=1);d.line([(32,24),(32,18)],fill=13,width=2);flame(d,32,7,.85)
    elif n==1:
        for x1,y1,x2,y2 in ((14,42,5,55),(20,45,13,58),(44,45,51,58),(50,42,59,55)):d.line([(x1,y1),(x2,y2)],fill=1,width=6);d.line([(x1,y1),(x2,y2)],fill=8,width=3)
        d.ellipse((3,26,23,52),fill=2,outline=1);d.ellipse((41,26,61,52),fill=2,outline=1);d.ellipse((15,19,49,47),fill=2,outline=1);d.ellipse((18,16,46,30),fill=4,outline=1)
        for x in (21,43):d.polygon([(x-3,20),(x-2,11),(x+2,11),(x+3,20)],fill=13,outline=1)
        d.line([(32,19),(32,12)],fill=13,width=2);flame(d,32,1,1)
    else:
        for x1,y1,x2,y2 in ((13,42,2,56),(19,45,12,61),(45,45,52,61),(51,42,62,56)):d.line([(x1,y1),(x2,y2)],fill=1,width=7);d.line([(x1,y1),(x2,y2)],fill=8,width=4)
        d.ellipse((0,21,24,54),fill=2,outline=1);d.ellipse((40,21,64,54),fill=2,outline=1);d.ellipse((12,16,52,49),fill=15,outline=1);d.ellipse((15,18,49,45),fill=2,outline=1);d.polygon([(24,26),(25,7),(39,7),(40,26)],fill=13,outline=1);d.ellipse((21,20,43,31),fill=4,outline=1)
        for x in (18,46):d.polygon([(x-4,23),(x-3,12),(x+3,12),(x+4,23)],fill=13,outline=1);flame(d,x,5,.55)
        flame(d,32,-8,1.15)
    if not back:
        for x in (25,39):d.ellipse((x-5,29,x+5,40),fill=10,outline=1);d.ellipse((x-3,30,x+3,36),fill=11);d.ellipse((x-2,30,x,32),fill=12)
    return a
def toxic(n,back=False):
    a=img(TX);d=ImageDraw.Draw(a)
    if n==0:
        d.polygon([(45,37),(58,28),(54,43),(61,47),(46,50)],fill=4,outline=1);d.ellipse((12,43,25,54),fill=4,outline=1);d.ellipse((39,43,52,54),fill=4,outline=1);d.ellipse((14,25,50,49),fill=2,outline=1);d.ellipse((17,22,47,44),fill=3,outline=1);d.polygon([(24,26),(27,11),(33,18),(38,9),(40,27)],fill=4,outline=1)
    elif n==1:
        d.polygon([(43,37),(60,24),(55,40),(62,48),(46,51)],fill=4,outline=1);d.ellipse((8,42,27,59),fill=4,outline=1);d.ellipse((37,42,56,59),fill=4,outline=1);d.ellipse((18,18,46,49),fill=2,outline=1);d.ellipse((20,23,44,46),fill=3,outline=1);d.polygon([(25,24),(28,6),(34,16),(39,5),(40,25)],fill=4,outline=1)
    else:
        for b in ((3,39,24,61),(40,39,61,61),(12,34,31,55),(33,34,52,55)):d.ellipse(b,fill=4,outline=1)
        d.ellipse((9,16,55,50),fill=2,outline=1);d.ellipse((14,21,50,48),fill=3,outline=1);d.polygon([(10,29),(17,11),(24,20),(30,7),(35,19),(45,8),(52,30)],fill=4,outline=1)
        for p in (((21,18),(22,3),(30,15)),((32,16),(37,0),(43,18)),((43,20),(53,6),(52,27))):d.polygon(p,fill=4,outline=1)
    for side in (-1,1):
        x=16 if side<0 else 48;d.polygon([(x,28),(x+side*-11,24),(x+side*-6,31),(x+side*-12,35),(x+side,36)],fill=6,outline=1)
    if not back:
        for x in (25,39):d.ellipse((x-4,28,x+4,37),fill=10,outline=1);d.ellipse((x-2,29,x+2,34),fill=7)
    return a
def pal(path,p):path.write_text("JASC-PAL\n0100\n16\n"+"\n".join("%d %d %d"%x for x in p)+"\n")
def make(folder,n,p,sh,fn,female=False):
    o=R/"graphics/pokemon"/folder;o.mkdir(parents=True,exist_ok=True);f=fn(n);b=fn(n,True);s=img(p);s.paste(f.crop((0,1,64,64)),(0,0))
    an=img(p,(64,128));an.paste(f,(0,0));an.paste(s,(0,64));ic=img(p,(32,64));ic.paste(f.resize((32,32),Image.Resampling.NEAREST),(0,0));ic.paste(s.resize((32,32),Image.Resampling.NEAREST),(0,32));ow=img(p,(192,32));z=f.resize((30,30),Image.Resampling.NEAREST)
    for i in range(6):ow.paste(z,(i*32+1,i%2))
    for name,x in (("anim_front.png",an),("anim_front_gba.png",an),("back.png",b),("back_gba.png",b),("icon.png",ic),("icon_gba.png",ic),("overworld.png",ow)):x.save(o/name,optimize=True)
    if female:an.save(o/"anim_frontf.png");b.save(o/"backf.png");ow.save(o/"overworldf.png")
    for name,q in (("normal.pal",p),("normal_gba.pal",p),("shiny.pal",sh),("shiny_gba.pal",sh),("overworld_normal.pal",p),("overworld_shiny.pal",sh)):pal(o/name,q)
for f,n in (("torchic",0),("combusken",1),("blaziken",2)):make(f,n,CR,CS,crab,True)
for f,n in (("mudkip",0),("marshtomp",1),("swampert",2)):make(f,n,TX,TS,toxic)
print("Generated Test01 starter sprites.")
