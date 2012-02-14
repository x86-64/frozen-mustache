
static err_item errs_list[] = {
 { -260, "src/main_mustache.c: mustache_render failed" },
 { -254, "src/main_mustache.c: buffer not supplied" },
 { -244, "src/main_mustache.c: bad precompile" },
 { -237, "src/main_mustache.c: bad template" },
 { -209, "src/main_mustache.c: calloc failed" },
 { -73, "src/main_mustache.c: mustache section render failed" },

	{ 0, NULL }
};
#define            errs_list_size      sizeof(errs_list[0])
#define            errs_list_nelements sizeof(errs_list) / errs_list_size
