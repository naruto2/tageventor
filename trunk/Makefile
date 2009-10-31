all: Debug Release
	@echo "All Done."
	@echo ""

binary: Release
	tar -cpz --exclude .svn scripts/generic bin/Release \
                 tagEventord gtagEventor.desktop icons \
                 install-sh > tagEventor.tar.gz 
Debug:
	@make -s -C lib/source/ -f Makefile Debug
	@make -s -f tagEventor.mak Debug
	@make -s -f gtagEventor.mak Debug
	@echo "Debug Done."
	@echo ""

Release:
	@make -s -C lib/source/ -f Makefile Release
	@make -s -f tagEventor.mak Release
	@make -s -f gtagEventor.mak Release
	@echo "Release Done."
	@echo ""

cleanDebug:
	@make -s -f tagEventor.mak cleanDebug
	@make -s -f gtagEventor.mak cleanDebug
	@make -s -C lib/source/ -f Makefile cleanDebug
	@echo "cleanDebug Done."
	@echo ""

cleanRelease:
	@make -s -f tagEventor.mak cleanRelease
	@make -s -f gtagEventor.mak cleanRelease
	@make -s -C lib/source/ -f Makefile cleanRelease
	@echo "cleanRelease Done."
	@echo ""

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean:
	@rm -f *~ scripts/*~
	@make -s -f tagEventor.mak clean
	@make -s -f gtagEventor.mak clean
	@make -s -C lib/source/ -f Makefile clean
	@echo "Clean Done."
