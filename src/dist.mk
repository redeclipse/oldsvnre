appversion=$(shell sed -n 's,.*RE_VER_STR.*"\(.*\)",\1,p' engine/engine.h)
dirname=$(APPNAME)-$(appversion)
tarname=$(APPNAME)_$(appversion)_linux_bsd.tar

../$(dirname):
	rm -rf ../$(dirname)
	tar \
		--exclude-vcs --exclude-backups \
		--exclude='$@' --exclude='../$(tarname)*' \
		--exclude='*.o' --exclude='*.lo' --exclude='*.gch' \
		--exclude='*src/reclient' --exclude='*src/reserver' \
		--exclude='*.exe' --exclude='*.dll' \
		--exclude='*redeclipse.app*' --exclude='*.bat' \
		--exclude='*src/lib*' --exclude='*src/include*' \
		--exclude='*src/xcode*' --exclude='*src/site*' \
		-cf - ../ | (mkdir $@/; cd $@/ ; tar -xpf -)
	$(MAKE) -C $@/src clean
	-$(MAKE) -C $@/src/enet distclean
	rm -rf $@/src/enet/autom4te.cache/

distdir: ../$(dirname)

../$(tarname): ../$(dirname)
	tar -cf $@ $<

dist-tar: ../$(tarname)

dist-gz: ../$(tarname)
	gzip -c < $< > $<.gz

dist-bz2: ../$(tarname)
	bzip2 -c < $< > $<.bz2

dist-xz: ../$(tarname)
	xz -c < $< > $<.xz

dist: dist-bz2

dist-all: dist-gz dist-bz2 dist-xz

dist-clean:
	rm -rf ../$(dirname)
	rm -f ../$(tarname)
	rm -f ../$(tarname).gz
	rm -f ../$(tarname).bz2
	rm -f ../$(tarname).xz

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

