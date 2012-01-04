GZIP=gzip

ICONS= \
	install/nix/redeclipse_x32.png \
	install/nix/redeclipse_x48.png \
	install/nix/redeclipse_x64.png \
	install/nix/redeclipse_x128.png

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

install/nix/redeclipse_x32.png: redeclipse.ico
	convert 'redeclipse.ico[0]' $@

install/nix/redeclipse_x48.png: redeclipse.ico
	convert 'redeclipse.ico[1]' $@

install/nix/redeclipse_x64.png: redeclipse.ico
	convert 'redeclipse.ico[2]' $@

install/nix/redeclipse_x128.png: redeclipse.ico
	convert 'redeclipse.ico[3]' $@

icons: $(ICONS)

system-install-client: client
	install -d $(libexecdir)/redeclipse
	install -d $(gamesbindir)
	install -m755 reclient $(libexecdir)/redeclipse/redeclipse
	install -m755 install/nix/redeclipse.am \
		$(gamesbindir)/redeclipse
	sed -e 's,@LIBEXECDIR@,$(patsubst $(DESTDIR)%,%,$(libexecdir)),' \
		-e 's,@DATADIR@,$(patsubst $(DESTDIR)%,%,$(datadir)),' \
		-e 's,@DOCDIR@,$(patsubst $(DESTDIR)%,%,$(docdir)),' \
		-i $(gamesbindir)/redeclipse

system-install-server: server
	install -d $(libexecdir)/redeclipse
	install -d $(gamesbindir)
	install -m755 reserver $(libexecdir)/redeclipse/redeclipse-server
	install -m755 install/nix/redeclipse-server.am \
		$(gamesbindir)/redeclipse-server
	sed -e 's,@LIBEXECDIR@,$(patsubst $(DESTDIR)%,%,$(libexecdir)),' \
		-e 's,@DATADIR@,$(patsubst $(DESTDIR)%,%,$(datadir)),' \
		-e 's,@DOCDIR@,$(patsubst $(DESTDIR)%,%,$(docdir)),' \
		-i $(gamesbindir)/redeclipse-server

system-install-data:
	install -d $(datadir)/redeclipse
	install -d $(libexecdir)/redeclipse
	cp -r ../data $(datadir)/redeclipse/data
	@rm -rv $(datadir)/redeclipse/data/examples
	ln -s $$(readlink -f $(datadir))/redeclipse/data \
		$$(readlink -f $(libexecdir))/redeclipse/data

system-install-docs: $(MANPAGES)
	install	-d $(mandir)/man6
	install -d $(docdir)/redeclipse
	sed -e 's,@LIBEXECDIR@,$(patsubst $(DESTDIR)%,%,$(libexecdir)),' \
		-e 's,@DATADIR@,$(patsubst $(DESTDIR)%,%,$(datadir)),' \
		-e 's,@DOCDIR@,$(patsubst $(DESTDIR)%,%,$(docdir)),' \
		install/nix/redeclipse.6.am | \
		$(GZIP) -9 -c > $(mandir)/man6/redeclipse.6.gz
	sed -e 's,@LIBEXECDIR@,$(patsubst $(DESTDIR)%,%,$(libexecdir)),' \
		-e 's,@DATADIR@,$(patsubst $(DESTDIR)%,%,$(datadir)),' \
		-e 's,@DOCDIR@,$(patsubst $(DESTDIR)%,%,$(docdir)),' \
		install/nix/redeclipse-server.6.am | \
		$(GZIP) -9 -c > $(mandir)/man6/redeclipse-server.6.gz
	cp -r ../data/examples $(docdir)/redeclipse/examples

system-install-menus: icons
	install -d $(menudir)
	install -d $(icondir)/32x32/apps
	install -d $(icondir)/48x48/apps
	install -d $(icondir)/64x64/apps
	install -d $(icondir)/128x128/apps
	install install/nix/redeclipse.desktop $(menudir)/redeclipse.desktop
	install install/nix/redeclipse_x32.png \
		$(icondir)/32x32/apps/redeclipse.png
	install install/nix/redeclipse_x48.png \
		$(icondir)/48x48/apps/redeclipse.png
	install install/nix/redeclipse_x64.png \
		$(icondir)/64x64/apps/redeclipse.png
	install install/nix/redeclipse_x128.png \
		$(icondir)/128x128/apps/redeclipse.png

system-install-cube2font: system-install-cube2font-docs
	install -d $(bindir)
	install -m755 cube2font $(bindir)/cube2font

system-install-cube2font-docs: install/nix/cube2font.1
	install -d $(mandir)/man1
	$(GZIP) -9 -c < install/nix/cube2font.1 \
		> $(mandir)/man1/cube2font.1.gz

system-install: system-install-client system-install-server system-install-data system-install-docs system-install-menus

system-uninstall-client:
	@rm -fv $(libexecdir)/redeclipse/redeclipse
	@rm -fv $(gamesbindir)/redeclipse

system-uninstall-server:
	@rm -fv $(libexecdir)/redeclipse/redeclipse-server
	@rm -fv $(gamesbindir)/redeclipse-server

system-uninstall-data:
	rm -rf $(datadir)/redeclipse/data
	@rm -fv $(libexecdir)/redeclipse/data

system-uninstall-docs:
	@rm -rfv $(docdir)/redeclipse/examples
	@rm -fv $(mandir)/man6/redeclipse.6.gz
	@rm -fv $(mandir)/man6/redeclipse-server.6.gz

system-uninstall-menus:
	@rm -fv $(menudir)/redeclipse.desktop
	@rm -fv $(icondir)/32x32/apps/redeclipse.png
	@rm -fv $(icondir)/48x48/apps/redeclipse.png
	@rm -fv $(icondir)/64x64/apps/redeclipse.png
	@rm -fv $(icondir)/128x128/apps/redeclipse.png

system-uninstall: system-uninstall-client system-uninstall-server system-uninstall-data system-uninstall-docs system-uninstall-menus
	-@rmdir -v $(libexecdir)/redeclipse
	-@rmdir -v $(datadir)/redeclipse
	-@rmdir -v $(docdir)/redeclipse

system-uninstall-cube2font-docs:
	@rm -fv $(mandir)/man1/cube2font.1.gz

system-uninstall-cube2font: system-uninstall-cube2font-docs
	@rm -fv $(bindir)/bin/cube2font
