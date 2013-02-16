ifeq ($(APPNAME),mekarcade)
# core
APPFLAGS= -DMEK=1
APPNAME=mekarcade
APPCLIENT=mekclient
APPSERVER=mekserver

# system-install and dist
ICON=../game/mek/textures/emblem.png
EXTRADATA=../game/mek
appnamefull=MekArcade
endif

MEK_DEFINES= \
	APPNAME=mekarcade \
	APPCLIENT=mekclient \
	APPSERVER=mekserver \
	APPFLAGS=" -DMEK=1"

mekarcade:
	$(MAKE) $(MEK_DEFINES)

install-mekarcade:
	$(MAKE) $(MEK_DEFINES) install
