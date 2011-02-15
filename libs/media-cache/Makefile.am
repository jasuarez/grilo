#
# Makefile.am
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2011 Igalia S.L. All rights reserved.

lib_LTLIBRARIES = libgrlmediacache-@GRL_MAJORMINOR@.la

libgrlmediacache_@GRL_MAJORMINOR@_la_DEPENDENCIES =	\
	$(top_builddir)/src/lib@GRL_NAME@.la

libgrlmediacache_@GRL_MAJORMINOR@_la_SOURCES = \
	grl-media-cache.c

libgrlmediacache_@GRL_MAJORMINOR@_la_CFLAGS =	\
	-I $(top_srcdir)/src		\
	-I $(top_srcdir)/src/data	\
	$(DEPS_CFLAGS)			\
	$(MEDIA_CACHE_CFLAGS)

libgrlmediacache_@GRL_MAJORMINOR@_la_LIBADD =	\
	$(top_builddir)/src/lib@GRL_NAME@.la	\
	$(DEPS_LIBS)				\
	$(MEDIA_CACHE_LIBS)

libgrlmediacache_@GRL_MAJORMINOR@includedir =	\
	$(includedir)/@GRL_NAME@/media-cache

libgrlmediacache_@GRL_MAJORMINOR@include_HEADERS =	\
	grl-media-cache.h

MAINTAINERCLEANFILES =	\
	*.in		\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
CLEANFILES =

# introspection support
if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
gir_headers = $(patsubst %,$(srcdir)/%, $(libgrlmediacache_@GRL_MAJORMINOR@include_HEADERS))
gir_sources = $(patsubst %,$(srcdir)/%, $(libgrlmediacache_@GRL_MAJORMINOR@_la_SOURCES))

INTROSPECTION_GIRS =

introspection_sources =	\
	$(gir_headers)	\
	$(gir_sources)

GrlMediaCache-@GRL_MAJORMINOR@.gir: libgrlmediacache-@GRL_MAJORMINOR@.la
GrlMediaCache_@GRL_MAJORMINOR_NORM@_gir_INCLUDES = Grl-0.1
GrlMediaCache_@GRL_MAJORMINOR_NORM@_gir_CFLAGS =	\
	--add-include-path=$(top_srcdir)/src		\
	-I$(top_srcdir)/libs $(libgrlmediacache_@GRL_MAJORMINOR@_la_CFLAGS)
GrlMediaCache_@GRL_MAJORMINOR_NORM@_gir_LIBS =	\
	libgrlmediacache-@GRL_MAJORMINOR@.la
GrlMediaCache_@GRL_MAJORMINOR_NORM@_gir_FILES = $(introspection_sources)
GrlMediaCache_@GRL_MAJORMINOR_NORM@_gir_SCANNERFLAGS = --identifier-prefix=Grl
INTROSPECTION_GIRS += GrlMediaCache-@GRL_MAJORMINOR@.gir
INTROSPECTION_COMPILER_ARGS = --includedir=. --includedir=$(top_srcdir)/src

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelib_DATA)
endif
