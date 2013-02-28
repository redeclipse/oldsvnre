ifeq ($(APPNAME),mekarcade)
# core
APPSHORTNAME=mek
APPFLAGS= -DMEK=1
APPNAME=mekarcade
APPCLIENT=mekclient
APPSERVER=mekserver

# system-install and dist
ICON=../game/mek/textures/emblem.png
endif

MEK_DEFINES= \
	APPNAME=mekarcade \
	APPCLIENT=mekclient \
	APPSERVER=mekserver \
	APPSHORTNAME=mek \
	APPFLAGS=" -DMEK=1"

mekarcade:
	$(MAKE) $(MEK_DEFINES)

install-mekarcade:
	$(MAKE) $(MEK_DEFINES) install
