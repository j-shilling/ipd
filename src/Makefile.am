bin_PROGRAMS=ipd

dist_sysconf_DATA = ipd.conf

ipd_SOURCES = main.c \
              mail.c \
	      settings.h \
	      settings.c \
	      asprintf.h

ipd_CFLAGS = -DSYSCONFDIR='"$(sysconfdir)"' $(CFLAGS) $(CURL_CFLAGS)
ipd_LDADD = $(CURL_LIBS)
