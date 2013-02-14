ifeq ($(APPNAME),mekarcade)
APPFLAGS= -DMEK=1
APPNAME=mekarcade
APPCLIENT=mekclient
APPSERVER=mekserver
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
