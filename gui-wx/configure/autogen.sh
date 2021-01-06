#!/bin/bash -e

rm -f sources.am
echo nodist_libatlast_a_SOURCES = ../../Atlast/*.{h,hpp,c} >>sources.am
echo nodist_libembed_a_SOURCES = ../../embed/*.{h,hpp,c} >>sources.am
echo nodist_liblibforth_a_SOURCES = ../../libforth/*.{h,hpp,c} >>sources.am
echo nodist_libzforth_a_SOURCES = ../../zForth/src/zforth/*.{h,hpp,c} >>sources.am
echo nodist_libkforth64_a_SOURCES = ../../kForth-64/src/*.{h,cpp,s,c} >>sources.am
echo nodist_liblua_a_SOURCES = ../../lua/*.{h,hpp,c} >>sources.am
echo libgolly_a_SOURCES = ../../gollybase/*.{h,cpp} >>sources.am
echo golly_SOURCES = ../../gui-wx/*.{h,cpp} >>sources.am
echo 'gollydatadir = $(pkgdatadir)/Patterns/Life/' >>sources.am
echo nobase_dist_gollydata_DATA = `find ../../{Help,Patterns,Rules,Scripts} -type f | sort` >>sources.am
echo EXTRA_DIST = \
	../../Atlast \
	../../embed \
	../../libforth \
	../../kForth-64 \
	../../kForth-64/src \
	../../zForth \
	../../zForth/src/zforth \
	../../lua \
	../../gui-wx/{makefile-{gtk,mac,win},Info.plist.in,*.mk,*.rc,configure/autogen.sh,icons,bitmaps} \
	../../gui-common $(find ../../gui-{android,ios,web} -type f | sort) \
	../../docs >>sources.am

aclocal -I m4
automake --add-missing --copy
autoconf
