ifeq ($(APPNAME),mekarcade)
# core
APPSHORTNAME=mek
APPFLAGS= -DMEK=1

# system-install and dist
ICON=../game/mek/textures/emblem.png
FILES=
endif

MEK_DEFINES= \
	APPNAME=mekarcade \
	APPSHORTNAME=mek \
	APPFLAGS=" -DMEK=1"

install-mekarcade:
	$(MAKE) $(MEK_DEFINES) install
