#
# Makefile.am
#
# Author: Víctor M. Jáquez L. <vjaquez@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS =

if BUILD_GRILO_NET
SUBDIRS += net
endif

if BUILD_GRILO_MEDIA_CACHE
SUBDIRS += media-cache
endif

DIST_SUBDIRS = net media-cache

MAINTAINERCLEANFILES = \
	*.in \
	*~

DISTCLEANFILES =$(MAINTAINERCLEANFILES)
