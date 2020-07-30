AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)

AM_CFLAGS = \
	$(GLOBAL_CFLAGS) \
	$(libplist_CFLAGS) \
	$(sqlite3_CFLAGS) \
	$(LFS_CFLAGS)

AM_LDFLAGS = \
	$(sqlite3_LIBS) \
	$(libplist_LIBS)

bin_PROGRAMS = \
	ibackup_list

ibackup_list_SOURCES = ibackup_list.c
ibackup_list_CFLAGS = $(AM_CFLAGS)
ibackup_list_LDFLAGS = $(AM_LDFLAGS)
ibackup_list_LDADD = $(top_builddir)/src/libibackup-1.0.la