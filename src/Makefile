ifeq ($(APPNAME),mekarcade)
APPCLIENT=mekclient
APPSERVER=mekserver
endif

# defaults to redeclipse
all:

mekarcade:
	$(MAKE) -f mekarcade.mk

install-mekarcade:
	$(MAKE) -f mekarcade.mk install

include core.mk
