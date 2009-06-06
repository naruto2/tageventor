all:	tagEventor
	@echo DONE
	@echo Other make targets are \"clean\" and \"install\" \(as root\)

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

install: exampleScipts/generic tagEventor tagEventord
	@echo You must be \"root\" for install to work correctly
	@mkdir -p /etc/tagEventor
	@cp -f exampleScipts/generic /etc/tagEventor/
	@echo Script \"generic\" copied to /etc/tagEventor
	@cp -f tagEventor /usr/sbin
	@echo Executable \"tagEventor\" copied to /usr/sbin/
	@cp -f tagEventord /etc/init.d
	@echo Init script \"tagEventord\" copied to /etc/init.d/
	@cd /etc/init.d
	@echo Adding init script links to /etc/rc\?.d with update-rc.d
	@update-rc.d -f tagEventord start 80 2 3 4 5 .
	@cd -
