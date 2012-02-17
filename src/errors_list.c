
static err_item errs_list[] = {
 { -264, "src/main_mustache.c: mustache_render failed" },
 { -258, "src/main_mustache.c: buffer not supplied" },
 { -248, "src/main_mustache.c: bad precompile" },
 { -241, "src/main_mustache.c: bad template" },
 { -213, "src/main_mustache.c: calloc failed" },
 { -76, "src/main_mustache.c: mustache section render failed" },

	{ 0, NULL }
};
#define            errs_list_size      sizeof(errs_list[0])
#define            errs_list_nelements sizeof(errs_list) / errs_list_size
