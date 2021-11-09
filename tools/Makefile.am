AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)

AM_CFLAGS = \
	$(GLOBAL_CFLAGS) \
	$(libplist_CFLAGS) \
	$(sqlite3_CFLAGS) \
	$(limd_glue_CFLAGS) \
	$(LFS_CFLAGS)

AM_LDFLAGS = \
	$(sqlite3_LIBS) \
	$(libplist_LIBS) \
	$(limd_glue_LIBS)

bin_PROGRAMS = \
	ibackup_list ibackup_extract

ibackup_list_SOURCES = ibackup_list.c
ibackup_list_CFLAGS = $(AM_CFLAGS)
ibackup_list_LDFLAGS = $(AM_LDFLAGS)
ibackup_list_LDADD = $(top_builddir)/src/libibackup-1.0.la

ibackup_extract_SOURCES = ibackup_extract.c
ibackup_extract_CFLAGS = $(AM_CFLAGS)
ibackup_extract_LDFLAGS = $(AM_LDFLAGS)
ibackup_extract_LDADD = $(top_builddir)/src/libibackup-1.0.la