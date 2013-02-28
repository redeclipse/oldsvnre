appnamefull:=$(shell sed -n 's/versionname *"\([^"]*\)"/\1/p' ../game/$(APPSHORTNAME)/version.cfg)
appversion:=$(shell sed -n 's/versionstring *"\([^"]*\)"/\1/p' ../game/$(APPSHORTNAME)/version.cfg)

dirname=$(APPNAME)-$(appversion)
dirname-osx=$(APPNAME).app
dirname-win=$(dirname)-win

resourcespath-osx=$(APPNAME).app/Contents/Resources
tmpdir-osx:=$(shell cd ../ && DIR=$$(mktemp -d $(dirname)-osx_XXXXX); rmdir $$DIR; echo $$DIR)
exename=$(APPNAME)_$(appversion)_win.exe

tarname=$(APPNAME)_$(appversion)_nix_bsd.tar
tarname-all=$(APPNAME)_$(appversion)_all.tar
tarname-osx=$(APPNAME)_$(appversion)_osx.tar

torrent-trackers-url="udp://tracker.openbittorrent.com:80,udp://tracker.publicbt.com:80,udp://tracker.ccc.de:80,udp://tracker.istole.it:80"
torrent-webseed-baseurl="http://downloads.sourceforge.net/redeclipse"

FILES+= \
	$(APPCLIENT).bat \
	$(APPCLIENT).sh \
	$(APPSERVER).bat \
	$(APPSERVER).sh

SRC_DIRS= \
	src/enet \
	src/engine \
	src/game \
	src/install \
	src/include \
	src/lib \
	src/scripts \
	src/shared

SRC_FILES= \
	src/Makefile \
	src/core.mk \
	src/dist.mk \
	src/dpiaware.manifest \
	src/system-install.mk \
	src/$(APPNAME).*

SRC_XCODE:= \
	src/xcode/*.h \
	src/xcode/*.m \
	src/xcode/*.mm \
	src/xcode/*.lproj \
	src/xcode/$(APPNAME)* \

OSX_APP=
ifeq ($(APPNAME),redeclipse)
OSX_APP=bin/$(APPNAME).app
endif

BIN_FILES:= \
	bin/amd64/*.txt \
	bin/amd64/*.dll \
	bin/amd64/$(APPCLIENT)* \
	bin/amd64/$(APPSERVER)* \
	bin/x86/*.txt \
	bin/x86/*.dll \
	bin/x86/$(APPCLIENT)* \
	bin/x86/$(APPSERVER)* \
	$(OSX_APP)

DISTFILES:= \
	$(FILES) \
	$(BIN_FILES) \
	data \
	game/$(APPSHORTNAME) \
	doc \
	$(shell cd ../ && find $(SRC_DIRS) -not -iname *.lo -not -iname *.gch -not -iname *.o) \
	$(SRC_FILES) \
	$(SRC_XCODE)

../$(dirname):
	rm -rf $@
	# Transform relative to src/ dir
	tar -cf - $(DISTFILES:%=../%) | (mkdir $@/; cd $@/ ; tar -xpf -)
	# create dedicated Makefile
	echo "APPNAME=$(APPNAME)" > $@/src/Makefile
	echo >>$@/src/Makefile
	echo "all:" >>$@/src/Makefile
	echo >>$@/src/Makefile
	echo "include $(APPNAME).mk" >>$@/src/Makefile
	echo >>$@/src/Makefile
	echo "include core.mk" >>$@/src/Makefile
	$(MAKE) -C $@/src clean
	-$(MAKE) -C $@/src/enet distclean
	rm -rf $@/src/enet/autom4te.cache/

distdir: ../$(dirname)

../$(tarname): ../$(dirname)
	tar \
		--exclude='$</bin/*.app*' \
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
ifeq ($(APPNAME),redeclipse)
	ln -s ../../../$</readme.txt ../$(tmpdir-osx)/$(dirname-osx)/Contents/Resources/readme.txt
endif
	tar \
		-hrf $@ -C ../$(tmpdir-osx) $(dirname-osx)
	rm -rf ../$(tmpdir-osx)/

dist-tar-osx: ../$(tarname-osx)

../$(dirname-win): ../$(dirname)
	cp -r $< $@
	rm -rf $@/bin/*.app/
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
	makensis $</src/install/win/$(APPNAME).nsi
	$(MV) $</src/install/win/$(exename) ../

dist-win: ../$(exename)

ifeq ($(APPNAME),redeclipse)
dist: dist-bz2 dist-bz2-all dist-bz2-osx dist-win
else
ifeq ($(APPNAME),mekarcade)
dist: dist-bz2 dist-bz2-all
endif
endif

../$(tarname).bz2.torrent: ../$(tarname).bz2
	rm -f $@
	cd ../ &&\
		mktorrent \
		-a $(torrent-trackers-url) \
		-w $(torrent-webseed-baseurl)/$(tarname).bz2 \
		-n $(tarname).bz2 \
		-c "Red Eclipse $(appversion) for Linux and BSD" \
		$(tarname).bz2

dist-torrent: ../$(tarname).bz2.torrent

../$(tarname-all).bz2.torrent: ../$(tarname-all).bz2
	rm -f $@
	cd ../ &&\
		mktorrent \
		-a $(torrent-trackers-url) \
		-w $(torrent-webseed-baseurl)/$(tarname-all).bz2 \
		-n $(tarname-all).bz2 \
		-c "$(appnamefull) $(appversion) for All Platforms" \
		$(tarname-all).bz2

dist-torrent-all: ../$(tarname-all).bz2.torrent

../$(tarname-osx).bz2.torrent: ../$(tarname-osx).bz2
	rm -f $@
	cd ../ &&\
		mktorrent \
		-a $(torrent-trackers-url) \
		-w $(torrent-webseed-baseurl)/$(tarname-osx).bz2 \
		-n $(tarname-osx).bz2 \
		-c "$(appnamefull) $(appversion) for OSX" \
		$(tarname-osx).bz2

dist-torrent-osx: ../$(tarname-osx).bz2.torrent

../$(exename).torrent: ../$(exename)
	rm -f $@
	cd ../ &&\
		mktorrent \
		-a $(torrent-trackers-url) \
		-w $(torrent-webseed-baseurl)/$(exename) \
		-n $(exename) \
		-c "$(appnamefull) $(appversion) for Windows" \
		$(exename)

dist-torrent-win: ../$(exename).torrent

ifeq ($(APPNAME),redeclipse)
dist-torrents: dist-torrent dist-torrent-all dist-torrent-osx dist-torrent-win
else
ifeq ($(APPNAME),mekarcade)
dist-torrents: dist-torrent dist-torrent-all
endif
endif

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

wiki-check-all-result: ../doc/wiki-all-vars-commands.txt
	scripts/check-wiki-all-result ../data/usage.cfg ../doc/wiki-all-vars-commands.txt
