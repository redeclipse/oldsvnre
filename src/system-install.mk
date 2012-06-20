appname=$(APPNAME)
cappname=$(shell echo $(appname) | sed 's,\(.*\),\U\1,')# Uppercase
appclient=$(APPCLIENT)
appserver=$(APPSERVER)
prefix=/usr/local
games=
gamesbin=/bin
bindir=$(DESTDIR)$(prefix)/bin
gamesbindir=$(DESTDIR)$(prefix)$(gamesbin)
libexecdir=$(DESTDIR)$(prefix)/lib$(games)
datadir=$(DESTDIR)$(prefix)/share$(games)
docdir=$(DESTDIR)$(prefix)/share/doc
mandir=$(DESTDIR)$(prefix)/share/man
menudir=$(DESTDIR)$(prefix)/share/applications
icondir=$(DESTDIR)$(prefix)/share/icons/hicolor

GZIPPER=gzip

ICONS= \
	install/nix/$(appname)_x16.png \
	install/nix/$(appname)_x32.png \
	install/nix/$(appname)_x48.png \
	install/nix/$(appname)_x64.png \
	install/nix/$(appname)_x128.png

install/nix/$(appname)_x16.png: $(appname).ico
	convert '$(appname).ico[0]' $@

install/nix/$(appname)_x32.png: $(appname).ico
	convert '$(appname).ico[1]' $@

install/nix/$(appname)_x48.png: $(appname).ico
	convert '$(appname).ico[2]' -resize 48x48 $@

install/nix/$(appname)_x64.png: $(appname).ico
	convert '$(appname).ico[2]' $@

install/nix/$(appname)_x128.png: $(appname).ico
	convert '$(appname).ico[3]' $@

icons: $(ICONS)

system-install-client: client
	install -d $(libexecdir)/$(appname)
	install -d $(gamesbindir)
	install -m755 $(appclient) $(libexecdir)/$(appname)/$(appname)
	install -m755 install/nix/$(appname).am \
		$(gamesbindir)/$(appname)
	sed -e 's,@LIBEXECDIR@,$(patsubst $(DESTDIR)%,%,$(libexecdir)),g' \
		-e 's,@DATADIR@,$(patsubst $(DESTDIR)%,%,$(datadir)),g' \
		-e 's,@DOCDIR@,$(patsubst $(DESTDIR)%,%,$(docdir)),g' \
		-e 's,@APPNAME@,$(appname),g' \
		-i $(gamesbindir)/$(appname)
	ln -s $(patsubst $(DESTDIR)%,%,$(datadir))/$(appname)/data \
		$(libexecdir)/$(appname)/data

system-install-server: server
	install -d $(libexecdir)/$(appname)
	install -d $(gamesbindir)
	install -m755 $(appserver) \
		$(libexecdir)/$(appname)/$(appname)-server
	install -m755 install/nix/$(appname)-server.am \
		$(gamesbindir)/$(appname)-server
	sed -e 's,@LIBEXECDIR@,$(patsubst $(DESTDIR)%,%,$(libexecdir)),g' \
		-e 's,@DATADIR@,$(patsubst $(DESTDIR)%,%,$(datadir)),g' \
		-e 's,@DOCDIR@,$(patsubst $(DESTDIR)%,%,$(docdir)),g' \
		-e 's,@APPNAME@,$(appname),g' \
		-i $(gamesbindir)/$(appname)-server

system-install-data:
	install -d $(datadir)/$(appname)
	cp -r ../data $(datadir)/$(appname)/data
	@rm -rv $(datadir)/$(appname)/data/examples

system-install-docs: $(MANPAGES)
	install	-d $(mandir)/man6
	install -d $(docdir)/$(appname)
	sed -e 's,@LIBEXECDIR@,$(patsubst $(DESTDIR)%,%,$(libexecdir)),g' \
		-e 's,@DATADIR@,$(patsubst $(DESTDIR)%,%,$(datadir)),g' \
		-e 's,@DOCDIR@,$(patsubst $(DESTDIR)%,%,$(docdir)),g' \
		-e 's,@APPNAME@,$(appname),g' \
		-e 's,@CAPPNAME@,$(cappname),g' \
		install/nix/$(appname).6.am | \
		$(GZIPPER) -9 -n -c > $(mandir)/man6/$(appname).6.gz
	sed -e 's,@LIBEXECDIR@,$(patsubst $(DESTDIR)%,%,$(libexecdir)),g' \
		-e 's,@DATADIR@,$(patsubst $(DESTDIR)%,%,$(datadir)),g' \
		-e 's,@DOCDIR@,$(patsubst $(DESTDIR)%,%,$(docdir)),g' \
		-e 's,@APPNAME@,$(appname),g' \
		-e 's,@CAPPNAME@,$(cappname),g' \
		install/nix/$(appname)-server.6.am | \
		$(GZIPPER) -9 -n -c > $(mandir)/man6/$(appname)-server.6.gz
	cp -r ../data/examples $(docdir)/$(appname)/examples

system-install-menus: icons
	install -d $(menudir)
	install -d $(icondir)/16x16/apps
	install -d $(icondir)/32x32/apps
	install -d $(icondir)/48x48/apps
	install -d $(icondir)/64x64/apps
	install -d $(icondir)/128x128/apps
	sed -e 's,@LIBEXECDIR@,$(patsubst $(DESTDIR)%,%,$(libexecdir)),g' \
		-e 's,@DATADIR@,$(patsubst $(DESTDIR)%,%,$(datadir)),g' \
		-e 's,@DOCDIR@,$(patsubst $(DESTDIR)%,%,$(docdir)),g' \
		-e 's,@APPNAME@,$(appname),g' \
		install/nix/$(appname).desktop.am > \
		$(menudir)/$(appname).desktop
	install -m644 install/nix/$(appname)_x16.png \
		$(icondir)/16x16/apps/$(appname).png
	install -m644 install/nix/$(appname)_x32.png \
		$(icondir)/32x32/apps/$(appname).png
	install -m644 install/nix/$(appname)_x48.png \
		$(icondir)/48x48/apps/$(appname).png
	install -m644 install/nix/$(appname)_x64.png \
		$(icondir)/64x64/apps/$(appname).png
	install -m644 install/nix/$(appname)_x128.png \
		$(icondir)/128x128/apps/$(appname).png

system-install-cube2font: system-install-cube2font-docs
	install -d $(bindir)
	install -m755 cube2font $(bindir)/cube2font

system-install-cube2font-docs: install/nix/cube2font.1
	install -d $(mandir)/man1
	$(GZIPPER) -9 -n -c < install/nix/cube2font.1 \
		> $(mandir)/man1/cube2font.1.gz

system-install: system-install-client system-install-server system-install-data system-install-docs system-install-menus

system-uninstall-client:
	@rm -fv $(libexecdir)/$(appname)/$(appname)
	@rm -fv $(libexecdir)/$(appname)/data
	@rm -fv $(gamesbindir)/$(appname)

system-uninstall-server:
	@rm -fv $(libexecdir)/$(appname)/$(appname)-server
	@rm -fv $(gamesbindir)/$(appname)-server

system-uninstall-data:
	rm -rf $(datadir)/$(appname)/data

system-uninstall-docs:
	@rm -rfv $(docdir)/$(appname)/examples
	@rm -fv $(mandir)/man6/$(appname).6.gz
	@rm -fv $(mandir)/man6/$(appname)-server.6.gz

system-uninstall-menus:
	@rm -fv $(menudir)/$(appname).desktop
	@rm -fv $(icondir)/16x16/apps/$(appname).png
	@rm -fv $(icondir)/32x32/apps/$(appname).png
	@rm -fv $(icondir)/48x48/apps/$(appname).png
	@rm -fv $(icondir)/64x64/apps/$(appname).png
	@rm -fv $(icondir)/128x128/apps/$(appname).png

system-uninstall: system-uninstall-client system-uninstall-server system-uninstall-data system-uninstall-docs system-uninstall-menus
	-@rmdir -v $(libexecdir)/$(appname)
	-@rmdir -v $(datadir)/$(appname)
	-@rmdir -v $(docdir)/$(appname)

system-uninstall-cube2font-docs:
	@rm -fv $(mandir)/man1/cube2font.1.gz

system-uninstall-cube2font: system-uninstall-cube2font-docs
	@rm -fv $(bindir)/bin/cube2font
