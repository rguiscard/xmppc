main_source = src/main.c

core_sources = \
	src/xmppc.h src/xmppc.c \
	src/mode/omemo.h src/mode/omemo.c \
	src/mode/roster.h src/mode/roster.c \
	src/mode/muc.h src/mode/muc.c \
	src/mode/message.h src/mode/message.c \
	src/mode/pgp.h src/mode/pgp.c \
	src/mode/account.h src/mode/account.c \
	src/mode/monitor.h src/mode/monitor.c \
	src/mode/openpgp.h src/mode/openpgp.c

AM_CFLAGS = @AM_CFLAGS@ -I$(srcdir)/src

dist_doc_DATA = README.md

bin_PROGRAMS = xmppc
xmppc_SOURCES = $(core_sources) $(main_source)
