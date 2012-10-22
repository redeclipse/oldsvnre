appversion:=$(shell \
	VER="$$(sed -n '/else/,/Red Eclipse/ s,.*RE_VER_STR.*"\(.*\)",\1,p' engine/engine.h | tr '\n' ' ')"; \
	if [ -z $$VER ]; then VER=0; fi; \
	echo $$VER)
dirname=$(APPNAME)-$(appversion)
dirname-osx=$(APPNAME).app
resourcespath-osx=$(APPNAME).app/Contents/Resources
tmpdir-osx:=$(shell cd ../ && DIR=$$(mktemp -d $(dirname)-osx_XXXXX); rmdir $$DIR; echo $$DIR)
dirname-win=$(dirname)-win
tarname=$(APPNAME)_$(appversion)_nix_bsd.tar
tarname-all=$(APPNAME)_$(appversion)_all.tar
tarname-osx=$(APPNAME)_$(appversion)_osx.tar
exename=$(APPNAME)_$(appversion)_win.exe
torrent-trackers-url="udp://tracker.openbittorrent.com:80,udp://tracker.publicbt.com:80,udp://tracker.ccc.de:80,udp://tracker.istole.it:80"
torrent-webseed-baseurl="http://downloads.sourceforge.net/redeclipse"

SRC_DIRS=src/enet src/engine src/game src/include src/install src/lib src/scripts src/shared src/xcode

# Relative to root dir
DISTFILES:= \
	bin/amd64 \
	bin/redeclipse.app \
	bin/x86 \
	data \
	doc \
	readme.txt \
	redeclipse.sh \
	server.sh \
	redeclipse.bat \
	server.bat \
	$(shell cd ../ && find $(SRC_DIRS) -not -iname *.lo -not -iname *.gch -not -iname *.o) \
	src/Makefile \
	src/dist.mk \
	src/dpiaware.manifest \
	src/redeclipse.cbp \
	src/redeclipse.ico \
	src/redeclipse.rc \
	src/system-install.mk

../$(dirname):
	rm -rf $@
	# Transform relative to src/ dir
	tar -cf - $(DISTFILES:%=../%) | (mkdir $@/; cd $@/ ; tar -xpf -)
	$(MAKE) -C $@/src clean
	-$(MAKE) -C $@/src/enet distclean
	rm -rf $@/src/enet/autom4te.cache/

distdir: ../$(dirname)

../$(tarname): ../$(dirname)
	tar \
		--exclude='$</bin/redeclipse.app*' \
		--exclude='$</bin/*/*.exe' \
		--exclude='$</bin/*/*.dll' \
		--exclude='$</bin/*/*.txt' \
		--exclude='$</*.bat' \
		-cf $@ $<

dist-tar: ../$(tarname)

../$(tarname-all): ../$(dirname)
	tar -cf $@ $<

dist-tar-all: ../$(tarname-all)

../$(tarname-osx): ../$(dirname)
	tar -cf $@ -C $</bin $(dirname-osx)
	if [ -z $(tmpdir-osx) ]; then exit 1; fi
	rm -rf ../$(tmpdir-osx)/
	mkdir ../$(tmpdir-osx)
	mkdir ../$(tmpdir-osx)/$(dirname-osx)
	mkdir ../$(tmpdir-osx)/$(dirname-osx)/Contents
	mkdir ../$(tmpdir-osx)/$(dirname-osx)/Contents/Resources
	# Use links with tar dereference to change directory paths
	ln -s ../../../$</data/ ../$(tmpdir-osx)/$(dirname-osx)/Contents/Resources/data
	ln -s ../../../$</doc/ ../$(tmpdir-osx)/$(dirname-osx)/Contents/Resources/doc
	ln -s ../../../$</src/ ../$(tmpdir-osx)/$(dirname-osx)/Contents/Resources/src
	ln -s ../../../$</readme.txt ../$(tmpdir-osx)/$(dirname-osx)/Contents/Resources/readme.txt
	tar \
		-hrf $@ -C ../$(tmpdir-osx) $(dirname-osx)
	rm -rf ../$(tmpdir-osx)/

dist-tar-osx: ../$(tarname-osx)

../$(dirname-win): ../$(dirname)
	cp -r $< $@
	rm -rf $@/bin/redeclipse.app/
	rm -rf $@/bin/*/*linux*
	rm -rf $@/bin/*/*freebsd*
	rm -f $@/*.sh

distdir-win: ../$(dirname-win)

../$(tarname).gz: ../$(tarname)
	gzip -c < $< > $@

dist-gz: ../$(tarname).gz

../$(tarname).bz2: ../$(tarname)
	bzip2 -c < $< > $@

dist-bz2: ../$(tarname).bz2

dist-nix: ../$(tarname).bz2

../$(tarname).xz: ../$(tarname)
	xz -c < $< > $@

dist-xz: ../$(tarname).xz

../$(tarname-all).gz: ../$(tarname-all)
	gzip -c < $< > $@

dist-gz-all: ../$(tarname-all).gz

../$(tarname-all).bz2: ../$(tarname-all)
	bzip2 -c < $< > $@

dist-bz2-all: ../$(tarname-all).bz2

dist-all: ../$(tarname-all).bz2

../$(tarname-all).xz: ../$(tarname-all)
	xz -c < $< > $@

dist-xz-all: ../$(tarname-all).xz

../$(tarname-osx).gz: ../$(tarname-osx)
	gzip -c < $< > $@

dist-gz-osx: ../$(tarname-osx).gz

../$(tarname-osx).bz2: ../$(tarname-osx)
	bzip2 -c < $< > $@

dist-bz2-osx: ../$(tarname-osx).bz2

dist-osx: ../$(tarname-osx).bz2

../$(tarname-osx).xz: ../$(tarname-osx)
	xz -c < $< > $@

dist-xz-osx: ../$(tarname-osx).xz

../$(exename): ../$(dirname-win)
	makensis $</src/install/win/redeclipse.nsi
	mv $</src/install/win/$(exename) ../

dist-win: ../$(exename)

dist: dist-bz2 dist-bz2-all dist-bz2-osx dist-win

../$(tarname).bz2.torrent: ../$(tarname)
	rm -f $@
	cd ../ &&\
		mktorrent \
		-a $(torrent-trackers-url) \
		-w $(torrent-webseed-baseurl)/$(tarname).bz2 \
		-n $(tarname).bz2 \
		-c "Red Eclipse $(appversion) for Linux and BSD" \
		$(tarname).bz2

dist-torrent: ../$(tarname).bz2.torrent

../$(tarname-all).bz2.torrent: ../$(tarname-all)
	rm -f $@
	cd ../ &&\
		mktorrent \
		-a $(torrent-trackers-url) \
		-w $(torrent-webseed-baseurl)/$(tarname-all).bz2 \
		-n $(tarname-all).bz2 \
		-c "Red Eclipse $(appversion) for All Platforms" \
		$(tarname-all).bz2

dist-torrent-all: ../$(tarname-all).bz2.torrent

../$(tarname-osx).bz2.torrent: ../$(tarname-osx)
	rm -f $@
	cd ../ &&\
		mktorrent \
		-a $(torrent-trackers-url) \
		-w $(torrent-webseed-baseurl)/$(tarname-osx).bz2 \
		-n $(tarname-osx).bz2 \
		-c "Red Eclipse $(appversion) for OSX" \
		$(tarname-osx).bz2

dist-torrent-osx: ../$(tarname-osx).bz2.torrent

../$(exename).torrent: ../$(exename)
	rm -f $@
	cd ../ &&\
		mktorrent \
		-a $(torrent-trackers-url) \
		-w $(torrent-webseed-baseurl)/$(exename) \
		-n $(exename) \
		-c "Red Eclipse $(appversion) for Windows" \
		$(exename)

dist-torrent-win: ../$(exename).torrent

dist-torrents: dist-torrent dist-torrent-all dist-torrent-osx dist-torrent-win

dist-mostlyclean:
	rm -rf ../$(dirname)
	rm -rf ../$(dirname-win)
	rm -f ../$(tarname)
	rm -f ../$(tarname-all)
	rm -f ../$(tarname-osx)

dist-clean: dist-mostlyclean
	rm -f ../$(tarname)*
	rm -f ../$(tarname-all)*
	rm -f ../$(tarname-osx)*
	rm -f ../$(exename)*

../doc/cube2font.txt: ../doc/man/cube2font.1
	scripts/generate-cube2font-txt $< $@

cube2font-txt: ../doc/cube2font.txt

../doc/examples/servexec.cfg: ../data/usage.cfg install-server
	scripts/update-servexec-defaults $@
	scripts/update-servexec-comments $< $@

update-servexec: ../doc/examples/servexec.cfg

../doc/wiki-contributors.txt: ../readme.txt
	scripts/generate-wiki-contributors $< $@

wiki-contributors: ../doc/wiki-contributors.txt

../doc/wiki-guidelines.txt: ../doc/guidelines.txt
	scripts/generate-wiki-guidelines $< $@

wiki-guidelines: ../doc/wiki-guidelines.txt

../doc/wiki-%.txt: ../data/usage.cfg scripts/wiki-common
	scripts/generate-wiki-$* $< $@

../doc/wiki-all-vars-commands.txt: ../doc/wiki-game-vars.txt ../doc/wiki-engine-vars.txt ../doc/wiki-weapon-vars.txt ../doc/wiki-commands.txt ../doc/wiki-aliases.txt
	scripts/generate-wiki-all-vars-commands $^ $@

wiki-game-vars: ../doc/wiki-game-vars.txt

wiki-engine-vars: ../doc/wiki-engine-vars.txt

wiki-weapon-vars: ../doc/wiki-weapon-vars.txt

wiki-commands: ../doc/wiki-commands.txt

wiki-aliases: ../doc/wiki-aliases.txt

wiki-all-vars-commands: ../doc/wiki-all-vars-commands.txt
