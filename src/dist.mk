appname=$(APPNAME)
appnamefull=$(shell sed -n 's/versionname *"\([^"]*\)"/\1/p' ../game/$(APPSHORTNAME)/version.cfg)
appversion=$(shell sed -n 's/versionstring *"\([^"]*\)"/\1/p' ../game/$(APPSHORTNAME)/version.cfg)

dirname=$(appname)-$(appversion)
dirname-osx=$(appname).app
dirname-win=$(dirname)-win

exename=$(appname)_$(appversion)_win.exe

tarname=$(appname)_$(appversion)_nix.tar
tarname-all=$(appname)_$(appversion)_all.tar
tarname-osx=$(appname)_$(appversion)_osx.tar

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
	src/include \
	src/lib \
	src/scripts \
	src/shared

SRC_FILES= \
	$(shell cd ../ && find $(SRC_DIRS) -not -iname *.lo -not -iname *.gch -not -iname *.o || echo "") \
	src/core.mk \
	src/dist.mk \
	src/dpiaware.manifest \
	src/system-install.mk \
	src/$(APPNAME).* \
	src/install/nix/$(APPNAME)* \
	src/install/win/*

SRC_XCODE= \
	src/xcode/*.h \
	src/xcode/*.m \
	src/xcode/*.mm \
	src/xcode/*.lproj \
	src/xcode/$(APPNAME)* \

OSX_APP=
ifeq ($(APPNAME),redeclipse)
OSX_APP=bin/$(APPNAME).app
endif

BIN_FILES= \
	bin/amd64/*.txt \
	bin/amd64/*.dll \
	bin/amd64/$(APPCLIENT)* \
	bin/amd64/$(APPSERVER)* \
	bin/x86/*.txt \
	bin/x86/*.dll \
	bin/x86/$(APPCLIENT)* \
	bin/x86/$(APPSERVER)* \
	$(OSX_APP)

DOC_FILES= \
	doc/all-licenses.txt \
	doc/cc-by-sa.txt \
	doc/changelog.txt \
	doc/cube2font.txt \
	doc/examples \
	doc/guidelines.txt \
	doc/irc.txt \
	doc/license.txt \
	doc/man/cube2font* \
	doc/man/$(APPNAME)* \
	doc/trademark.txt

DISTFILES= \
	$(FILES) \
	$(BIN_FILES) \
	data \
	game/$(APPSHORTNAME) \
	$(DOC_FILES) \
	$(SRC_FILES) \
	$(SRC_XCODE)

weapon-names=$(shell sed -n '/WPSVAR(0, name,/,/);/s/ *"\([^"]*\)",*/\1 /g;s/ $$//p' game/weapons.h)
weapon-wiki-pages=$(shell for w in $(weapon-names); do echo "../doc/wiki-weapon-$${w}.txt"; done)

../$(dirname):
	rm -rf $@
	# exclude VCS and transform relative to src/ dir
	tar --exclude='.svn' --exclude='*.git' --exclude='*.hg' \
		--exclude='*.bzr' \
		-cf - $(DISTFILES:%=../%) | (mkdir $@/; cd $@/ ; tar -xpf -)
	# create dedicated Makefile
	echo "APPNAME=$(APPNAME)" >$@/src/Makefile
ifneq ($(APPNAME),$(appname))
	echo "appname=$(appname)" >>$@/src/Makefile
endif
	echo >>$@/src/Makefile
	echo "all:" >>$@/src/Makefile
	echo >>$@/src/Makefile
	echo "include $(APPNAME).mk" >>$@/src/Makefile
	echo >>$@/src/Makefile
	echo "include core.mk" >>$@/src/Makefile
	$(MAKE) -C $@/src clean
	-$(MAKE) -C $@/src/enet distclean
	rm -rf $@/src/enet/autom4te.cache/
	rm -f $@/data/misc/largeandincharge.png

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
	mkdir tmpdir-osx
	mkdir tmpdir-osx/$(dirname-osx)
	mkdir tmpdir-osx/$(dirname-osx)/Contents
	mkdir tmpdir-osx/$(dirname-osx)/Contents/Resources
	# Use links with tar dereference to change directory paths
	ln -s ../$</data/ tmpdir-osx/$(dirname-osx)/data
	ln -s ../$</doc/ tmpdir-osx/$(dirname-osx)/doc
	ln -s ../$</game/ tmpdir-osx/$(dirname-osx)/game
	ln -s ../$</src/ tmpdir-osx/$(dirname-osx)/src
ifeq ($(APPNAME),redeclipse)
	ln -s ../$</readme.txt tmpdir-osx/$(dirname-osx)/readme.txt
endif
	tar \
		-hrf $@ -C tmpdir-osx $(dirname-osx)
	rm -rf tmpdir-osx/

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
	scripts/cube2font-txt $< $@

cube2font-txt: ../doc/cube2font.txt

../doc/examples/servinit.cfg: ../data/config/usage.cfg install-server
	scripts/servinit-defaults $@
	scripts/servinit-comments $< $@

update-servinit: ../doc/examples/servinit.cfg

../doc/wiki-contributors.txt: ../readme.txt
	scripts/wiki-contributors $< $@

wiki-contributors: ../doc/wiki-contributors.txt

../doc/wiki-guidelines.txt: ../doc/guidelines.txt
	scripts/wiki-guidelines $< $@

wiki-guidelines: ../doc/wiki-guidelines.txt

../doc/varsinfo-all.txt: install-client
	RE_TEMPHOME="$$(mktemp -d)"; \
		../redeclipse.sh -h"$$RE_TEMPHOME" -df0 -w640 -dh480 -du0 -x"writevarsinfo; quit"; \
		mv "$$RE_TEMPHOME/varsinfo.txt" $@; \
		rm -r "$$RE_TEMPHOME"

../doc/varsinfo-weapon-%.txt: ../doc/varsinfo-all.txt
	# check if beginning matches weapon name
	awk 'match($$1, /^$*/) {print}' $^ > $@

../doc/varsinfo-non-weapons.txt: ../doc/varsinfo-all.txt
	# don't match weapons, do match VAR, FVAR and SVAR types
	awk '!match($$1, /^('"$$(echo $(weapon-names) | tr ' ' '|' )"')/) && \
		(match($$2, 0) || match($$2, 1) || match($$2, 2)) \
		{print}' $^ > $@

../doc/varsinfo-client-and-admin.txt: ../doc/varsinfo-all.txt
	# don't match weapons, commands
	# do match client or admin flags
	# overlaps world vars
	gawk '!match($$1, /^('"$$(echo $(weapon-names) | tr ' ' '|' )"')/) && \
		!match($$2, "3") && \
		!and($$3, lshift(1, 3)) && \
		and($$3, or(lshift(1, 6), lshift(1,9))) \
		{print}' $^ > $@

../doc/varsinfo-textures.txt: ../doc/varsinfo-all.txt
	# check if texture flag is set
	# overlaps weapon tex vars
	gawk 'and($$3, lshift(1, 5)) {print}' $^ > $@

../doc/varsinfo-world.txt: ../doc/varsinfo-all.txt
	# check if world flag is set
	# overlaps client-and-admin vars
	gawk 'and($$3, lshift(1, 3)) {print}' $^ > $@

../doc/varsinfo-commands.txt: ../doc/varsinfo-all.txt
	# check if type is == 3
	awk 'match($$2, "3") {print}' $^ > $@

../doc/varsinfo-aliases.txt: ../doc/varsinfo-all.txt
	# check if type is == 4
	awk 'match($$2, "4") {print}' $^ > $@

../doc/wiki-%.txt: ../doc/varsinfo-%.txt
	scripts/wiki-convert $^ > $@

../doc/wiki-all-vars-commands.txt: ../doc/varsinfo-all.txt
	scripts/wiki-convert $^ > $@

wiki-all-vars-commands: ../doc/wiki-all-vars-commands.txt

wiki-weapons: $(weapon-wiki-pages)

wiki-non-weapons: ../doc/wiki-non-weapons.txt

wiki-client-and-admin: ../doc/wiki-client-and-admin.txt

wiki-textures: ../doc/wiki-textures.txt

wiki-world: ../doc/wiki-world.txt

wiki-commands: ../doc/wiki-commands.txt

wiki-aliases: ../doc/wiki-aliases.txt

wiki-all: ../doc/wiki-all-vars-commands.txt $(wiki-weapon-pages) ../doc/wiki-non-weapons.txt ../doc/wiki-client-and-admin.txt ../doc/wiki-textures.txt ../doc/wiki-world.txt ../doc/wiki-commands.txt ../doc/wiki-aliases.txt

wiki-clean:
	rm -f ../doc/varsinfo-*.txt ../doc/wiki-*.txt
