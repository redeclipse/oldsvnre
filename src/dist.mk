appversion=$(shell sed -n 's,.*RE_VER_STR.*"\(.*\)",\1,p' engine/engine.h)
dirname=$(APPNAME)-$(appversion)
tarname=$(APPNAME)_$(appversion)_linux_bsd.tar

../$(dirname):
	rm -rf ../$(dirname)
	tar \
		--exclude-vcs --exclude-backups \
		--exclude='../$(dirname)' --exclude='../$(tarname)*' \
		--exclude='*.o' --exclude='*.lo' --exclude='*.gch' \
		--exclude='*src/reclient' --exclude='*src/reserver' \
		--exclude='*.exe' --exclude='*.dll' \
		--exclude='*redeclipse.app*' --exclude='*.bat' \
		--exclude='*src/lib*' --exclude='*src/include*' \
		--exclude='*src/xcode*' --exclude='*src/site*' \
		-cf - ../ | (mkdir ../$(dirname)/; cd ../$(dirname)/ ; tar -xpf -)
	$(MAKE) -C ../$(dirname)/src clean
	-$(MAKE) -C ../$(dirname)/src/enet distclean
	rm -rf ../$(dirname)/src/enet/autom4te.cache/

distdir: ../$(dirname)

../$(tarname): ../$(dirname)
	tar -cf ../$(tarname) ../$(dirname)

dist-tar: ../$(tarname)

dist-gz: ../$(tarname)
	gzip -c < ../$(tarname) > ../$(tarname).gz

dist-bz2: ../$(tarname)
	bzip2 -c < ../$(tarname) > ../$(tarname).bz2

dist-xz: ../$(tarname)
	xz -c < ../$(tarname) > ../$(tarname).xz

dist: dist-bz2

dist-all: dist-gz dist-bz2 dist-xz

dist-clean:
	rm -rf ../$(dirname)
	rm -f ../$(tarname)
	rm -f ../$(tarname).gz
	rm -f ../$(tarname).bz2
	rm -f ../$(tarname).xz

../doc/cube2font.txt: ../doc/man/cube2font.1
	MANWIDTH=80 man --no-justification --no-hyphenation \
		../doc/man/cube2font.1 \
		| col -b > ../doc/cube2font.txt ;\
	echo "\nThis text file was automatically generated from cube2font.1\n\
	Please do not edit it manually." \
		>> ../doc/cube2font.txt

cube2font-txt: ../doc/cube2font.txt

