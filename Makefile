all: debug

debug: lib/Debug/libtagReader.a bin/Debug/tagEventor bin/Debug/cpanel

#TODO define list of objects some time
bin/Debug/tagEventor: obj/Debug/tagEventor.o libtagReader.a
	gcc obj/Debug/tagEventor.o -Wall -l pcsclite  -L. -ltagReader -o $@
	@echo tageventor BUILT

# TODO do a generic .c to .o rule sometime
obj/Debug/tagEventor.o: tagEventor.c tagReader.h
	gcc -c tagEventor.c -Wall -I . -o $@

bin/Debug/cpanel: obj/Debug/cpanel.o
	gcc obj/Debug/cpanel.o `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -l pcsclite  -L. -ltagReader -o $@
	@echo cpanel BUILT

obj/Debug/cpanel.o: cpanel.c tagReader.h
	gcc -c cpanel.c `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/icons" -Wall -I . -o $@

lib/Debug/libtagReader.a: obj/Debug/tagReader.o
	ar rcs $@ $<
	@echo libtagReader BUILT

obj/Debug/tagReader.o: tagReader.c  tagReader.h
	gcc -c tagReader.c -Wall -I . -o $@

clean:
	rm -f obj/Debug/*.o obj/Debug/*.so.* lib/Debug/*.a *~ bin/Debug/*


