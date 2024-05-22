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

make_delay.exe: make_delay.obj Makefile
	cl $(CFLAGS) $(@R).c /link /incremental:no

delaydll.c delay.h: make_delay.exe Makefile
	make_delay.exe

delaydll.lib delaydll.dll: delaydll.obj delay.h
	cl /LD $(CFLAGS) $(@R).c /link /incremental:no

# /guard:cf is important
# delayimp.lib is searched
# ..\delayimp.lib has the bug
#
delayexe.exe: delayexe.obj delay.h delaydll.lib Makefile delayref.c
	cl $(CFLAGS) delayexe.obj /link /incremental:no delaydll.lib /delayload:delaydll.dll ..\delayimp.lib /guard:cf

