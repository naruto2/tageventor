all: debug

debug: bin/Debug/tagEventor

#TODO define list of objects some time
bin/Debug/tagEventor: lib/Debug/libtagReader.a obj/Debug/tagEventor.o obj/Debug/aboutDialog.o obj/Debug/controlPanel.o obj/Debug/systemTray.o
	gcc obj/Debug/tagEventor.o obj/Debug/controlPanel.o `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -l pcsclite  -Llib/Debug -l tagReader -o $@
	@echo tagEventor BUILT \(./bin/Debug/tagEventor\)

obj/Debug/tagEventor.o: tagEventor.c tagReader.h
	gcc -c tagEventor.c -Wall -I . -o $@

obj/Debug/aboutDialog.o: aboutDialog.c aboutDialog.h
	gcc -c aboutDialog.c `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

obj/Debug/controlPanel.o: controlPanel.c tagReader.h
	gcc -c controlPanel.c `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

obj/Debug/systemTray.o: systemTray.c
	gcc -c systemTray.c `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

lib/Debug/libtagReader.a: obj/Debug/tagReader.o
	ar rcs $@ $<
	@echo libtagReader BUILT

obj/Debug/tagReader.o: tagReader.c  tagReader.h
	gcc -c tagReader.c -Wall -I . -o $@

clean:
	rm -f *.o *.a obj/Debug/*.o obj/Debug/*.so.* lib/Debug/*.a *~ bin/Debug/*


