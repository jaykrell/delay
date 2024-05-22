# Microsoft nmake on Windows, Visual C++.
CFLAGS=-MD -Gy -Z7
LFLAGS=-incremental:no

all: delaydll.dll delayexe.exe

clean:
	del /f *.exe *.obj *.pdb *.ilk delay.h delaydll.* delayref.* *.lib *.exp

OBJS=   delaydll.$O \
        delayexe.$O \

.SUFFIXES: .c .obj

.c.obj:
	cl /c $(CFLAGS) $<

make_delay.exe: make_delay.obj
	cl $(CFLAGS) $(@R).c /link /incremental:no

delaydll.c delay.h: make_delay.exe
	make_delay.exe

delaydll.lib delaydll.dll: delaydll.obj delay.h
	cl /LD $(CFLAGS) $(@R).c /link /incremental:no

delayexe.exe: delayexe.obj delay.h delaydll.lib
	cl $(CFLAGS) delayexe.obj /link /incremental:no delaydll.lib /delayload:delaydll.dll delayimp.lib

