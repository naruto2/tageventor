all: Debug Release
	@echo "All Done."
	@echo ""

Debug:
	@make -s -f libtagReader.mak Debug
	@make -s -f tagEventor.mak Debug
	@make -s -f gtagEventor.mak Debug
	@echo "Debug Done."
	@echo ""

Release:
	@make -s -f libtagReader.mak Release
	@make -s -f tagEventor.mak Release
	@make -s -f gtagEventor.mak Release
	@echo "Release Done."
	@echo ""

cleanDebug:
	@make -s -f libtagReader.mak cleanDebug
	@make -s -f tagEventor.mak cleanDebug
	@make -s -f gtagEventor.mak cleanDebug
	@echo "cleanDebug Done."
	@echo ""

cleanRelease:
	@make -s -f libtagReader.mak cleanRelease
	@make -s -f tagEventor.mak cleanRelease
	@make -s -f gtagEventor.mak cleanRelease
	@echo "cleanRelease Done."
	@echo ""

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean:
	@rm -f *~ scripts/*~
	@make -s -f libtagReader.mak clean
	@make -s -f tagEventor.mak clean
	@make -s -f gtagEventor.mak clean
	@echo "Clean Done."
