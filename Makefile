# Microsoft nmake on Windows, Visual C++.
CFLAGS=-MD -Gy -Z7
LFLAGS=-incremental:no

all: delaydll.dll delayexe.exe

clean:
	del /f *.exe *.obj *.pdb *.ilk delay.h delaydll.*

OBJS=   delaydll.$O \
        delayexe.$O \

make_delay.exe: make_delay.c
	cl $(CFLAGS) $(@R).c /link /incremental:no

delaydll.c delay.h: make_delay.exe
	make_delay.exe

delaydll.lib delaydll.dll: delaydll.c delay.h
	cl /LD $(CFLAGS) $(@R).c /link /incremental:no

delayexe.exe: delayexe.c delay.h delaydll.lib
	cl $(CFLAGS) $(@R).c /link /incremental:no

