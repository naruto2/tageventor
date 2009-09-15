all:	tagEventor cpanel
	@echo DONE

cpanel: cpanel.c cpanel.glade libtagReader.a
	gcc `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/icons"  -I . -L. -ltagReader -o $@ $<

#TODO define list of objects some time
tagEventor: tagEventor.o libtagReader.a
	gcc tagEventor.o -Wall -l pcsclite  -L. -ltagReader -o $@

# TODO do a generic .c to .o rule sometime
tagEventor.o: tagEventor.c tagReader.h
	gcc -c tagEventor.c -Wall -I .

libtagReader.a: tagReader.o
	ar rcs libtagReader.a tagReader.o

tagReader.o: tagReader.c  tagReader.h
	gcc -c tagReader.c -Wall -I .

clean:
	rm -f *.o *.so.* *~


