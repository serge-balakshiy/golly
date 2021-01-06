ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(top_srcdir)/../../gollybase/
AM_CXXFLAGS = -DGOLLYDIR="$(GOLLYDIR)" -Wall -fno-strict-aliasing
AM_LDFLAGS = -Wl,--as-needed -rdynamic
AM_CCASFLAGS = -I ../../kForth-64/src

if MAC
liblua_a_CPPFLAGS = -DLUA_USE_MACOSX
endif

AUTOMAKE_OPTIONS = subdir-objects
DEF_DIR_VAR=KFORTH_DIR
if LINUX
liblua_a_CPPFLAGS = -DLUA_USE_LINUX
libatlast_a_CPPFLAGS = -DMEMSTAT -DEXPORT -DREADONLYSTRINGS
liblibforth_a_CPPFLAGS = 
libembed_a_CPPFLAGS = -DNDEBUG -std=c99
libkforth64_a_CPPFLAGS = -DDIR_ENV_VAR=\"$(DEF_DIR_VAR)\"
#libkforth64_a_CPPFLAGS = -DDIR_ENV_VAR=\"$(GOLLYDIR)\"
endif

bin_PROGRAMS = golly bgolly
noinst_PROGRAMS = RuleTableToTree
noinst_LIBRARIES = libgolly.a liblua.a libatlast.a liblibforth.a libembed.a libkforth64.a 
doc_DATA = ../../docs/License.html ../../docs/ReadMe.html

include sources.am

LDLIBS = -lreadline -lgforth -lm

golly_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) $(PYTHON_INCLUDE) \
	$(PERL_CPPFLAGS) $(PERL_INCLUDE) \
	$(liblua_a_CPPFLAGS) \
	$(libatlast_a_CPPFLAGS) \
	$(liblibforth_a_CPPFLAGS) \
	$(libembed_a_CPPFLAGS) \
	$(libkforth64_a_CPPFLAGS) \
	-I$(top_srcdir)/../../lua \
	-I$(top_srcdir)/../../Atlast \
	-I$(top_srcdir)/../../embed \
	-I$(top_srcdir)/../../libforth \
	-I$(top_srcdir)/../../kForth-64 \
	-I$(top_srcdir)/../../kForth-64/src \
	-I$(top_srcdir)/../../zForth \
	-I$(top_srcdir)/../../zForth/src/zforth \
	-I$(top_srcdir)/../../zForth/src/linux


golly_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS_ONLY)
golly_LDADD = $(WX_LIBS) libgolly.a liblua.a libatlast.a libembed.a liblibforth.a libkforth64.a $(LDLIBS)

if WINDOWS
golly_LDADD += gollyres.o
gollyres.o: ../golly.rc; $(WX_RESCOMP) $< $@
endif

bgolly_SOURCES = ../../cmdline/bgolly.cpp
bgolly_LDADD = libgolly.a

RuleTableToTree_SOURCES = ../../cmdline/RuleTableToTree.cpp
RuleTableToTree_LDADD = libgolly.a

srcdistdir=$(PACKAGE)-$(VERSION)$(SRCDISTSUFFIX)
bindistdir=$(PACKAGE)-$(VERSION)$(BINDISTSUFFIX)

srcdist:
	mkdir -p $(srcdistdir)/gui-wx/configure
	make distdir=$(srcdistdir)/gui-wx/configure distdir
	tardir=$(srcdistdir) && $(am__tar) \
		| GZIP=$(GZIP_ENV) gzip -c >$(srcdistdir).tar.gz
	rm -r $(srcdistdir)

bindist:
	make bindir=/ pkgdatadir=/ docdir=/ GOLLYDIR= \
		DESTDIR=$(builddir)/$(bindistdir) clean all install
	tardir=$(bindistdir) && $(am__tar) \
		| GZIP=$(GZIP_ENV) gzip -c >$(bindistdir).tar.gz
	rm -r $(bindistdir)
