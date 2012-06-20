
static err_item errs_list[] = {
 { -263, "src/main_mustache.c: mustache_render failed" },
 { -257, "src/main_mustache.c: buffer not supplied" },
 { -247, "src/main_mustache.c: bad precompile" },
 { -240, "src/main_mustache.c: bad template" },
 { -212, "src/main_mustache.c: calloc failed" },
 { -75, "src/main_mustache.c: mustache section render failed" },

	{ 0, NULL }
};
#define            errs_list_size      sizeof(errs_list[0])
#define            errs_list_nelements sizeof(errs_list) / errs_list_size
