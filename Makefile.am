ACLOCAL_AMFLAGS = -I m4
CPPFLAGS+= -I@abs_top_srcdir@

lib_LTLIBRARIES=    libsick.la
libsick_la_SOURCES= Timestamp.cpp sick.c sick_poster.c util.c
libsick_la_LDFLAGS= -version-info 0:0:0
libsick_la_LIBADD=  

include_HEADERS=    Timestamp.hpp sick.h sick_poster.h util.h

bin_PROGRAMS=       csick
csick_SOURCES=      csick.cpp
csick_LDADD=        libsick.la

