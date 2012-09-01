appversion=$(shell sed -n 's,.*RE_VER_STR.*"\(.*\)",\1,p' engine/engine.h)
dirname=$(APPNAME)-$(appversion)
tarname=$(APPNAME)_$(appversion)_nix_bsd.tar
tarname-all=$(APPNAME)_$(appversion)_all.tar
tarname-osx=$(APPNAME)_$(appversion)_osx.tar
exename=$(APPNAME)_$(appversion)_win.exe

../$(dirname):
	rm -rf $@
	tar \
		--exclude-vcs --exclude-backups \
		--exclude='$@*' \
		--exclude='../$(tarname)*' --exclude='../$(tarname-all)*' \
		--exclude='../$(tarname-osx)*' --exclude='../$(exename)' \
		--exclude='*.o' --exclude='*.lo' --exclude='*.gch' \
		--exclude='../src/reclient' --exclude='../src/reserver' \
		--exclude='../src/site*' \
		-cf - ../ | (mkdir $@/; cd $@/ ; tar -xpf -)
	$(MAKE) -C $@/src clean
	-$(MAKE) -C $@/src/enet distclean
	rm -rf $@/src/enet/autom4te.cache/

distdir: ../$(dirname)

../$(tarname): ../$(dirname)
	tar \
		--exclude='$</bin*/redeclipse.app*' \
		--exclude='$</bin*/*.exe' \
		--exclude='$</bin*/*.dll' \
		--exclude='$</bin*/*.txt' \
		--exclude='$</*.bat' \
		-cf $@ $<

dist-tar: ../$(tarname)

../$(tarname-all): ../$(dirname)
	tar -cf $@ $<

dist-tar-all: ../$(tarname-all)

../$(tarname-osx): ../$(dirname)
	tar \
		--exclude='$</bin*/*linux*' \
		--exclude='$</bin*/*bsd*' \
		--exclude='$</*.sh' \
		--exclude='$</bin*/*.exe' \
		--exclude='$</bin*/*.dll' \
		--exclude='$</bin*/*.txt' \
		--exclude='$</*.bat' \
		-cf $@ $<

dist-tar-osx: ../$(tarname-osx)

dist-gz: ../$(tarname)
	gzip -f $<

dist-bz2: ../$(tarname)
	bzip2 -f $<

dist-xz: ../$(tarname)
	xz -f $<

dist-gz-all: ../$(tarname-all)
	gzip -f $<

dist-bz2-all: ../$(tarname-all)
	bzip2 -f $<

dist-xz-all: ../$(tarname-all)
	xz -f $<

dist-gz-osx: ../$(tarname-osx)
	gzip -f $<

dist-bz2-osx: ../$(tarname-osx)
	bzip2 -f $<

dist-xz-osx: ../$(tarname-osx)
	xz -f $<

../$(exename): ../$(dirname)
	rm -rf $</bin*/redeclipse.app/
	rm -rf $</bin*/*linux*
	rm -rf $</bin*/*freebsd*
	makensis $</src/install/win/redeclipse.nsi
	mv $</src/install/win/$(exename) ../
	cp -r ../bin* $<

dist-win: ../$(exename)

dist: dist-bz2 dist-bz2-all dist-bz2-osx dist-win

dist-clean:
	rm -rf ../$(dirname)
	rm -f ../$(tarname)*
	rm -f ../$(tarname-all)*
	rm -f ../$(tarname-osx)*
	rm -f ../$(exename)

../doc/cube2font.txt: ../doc/man/cube2font.1
	scripts/generate-cube2font-txt $< $@

cube2font-txt: ../doc/cube2font.txt

../doc/guidelines-wiki.txt: ../doc/guidelines.txt
	scripts/generate-guidelines-wiki $< $@

guidelines-wiki: ../doc/guidelines-wiki.txt

../doc/examples/servexec.cfg: ../data/usage.cfg install-server
	scripts/update-servexec-defaults $@
	scripts/update-servexec-comments $< $@

update-servexec: ../doc/examples/servexec.cfg

