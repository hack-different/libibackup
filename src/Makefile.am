AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)

AM_CFLAGS = \
	$(GLOBAL_CFLAGS) \
	$(libplist_CFLAGS) \
	$(LFS_CFLAGS) \
	$(sqlite3_CFLAGS) \
	$(PTHREAD_CFLAGS)

AM_LDFLAGS = \
	$(sqlite3_LIBS) \
	$(libplist_LIBS) \
	$(PTHREAD_LIBS)

lib_LTLIBRARIES = libibackup-1.0.la
libibackup_1_0_la_LIBADD =
libibackup_1_0_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBIBACKUP_SO_VERSION) -no-undefined
libibackup_1_0_la_SOURCES = \
	libibackup.c libibackup.h



if WIN32
libibackup_1_0_la_LDFLAGS += -avoid-version -static-libgcc
libibackup_1_0_la_LIBADD += -lole32 -lws2_32 -lgdi32
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libibackup-1.0.pc
