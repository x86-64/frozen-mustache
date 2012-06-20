modules_LTLIBRARIES = module_c_mustache.la
module_c_mustache_la_CFLAGS = -I@includedir@/frozen/ -I../ -DFROZEN_MODULE
module_c_mustache_la_LDFLAGS = -module -avoid-version -shared -lmustache_c -lfrozen
module_c_mustache_la_SOURCES = main_mustache.c
EXTRA_DIST = errors_list.c
