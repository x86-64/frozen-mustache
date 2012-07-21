
static err_item errs_list[] = {
 { -463, "src/main_mustache.c: mustache_render failed" },
 { -457, "src/main_mustache.c: buffer not supplied" },
 { -447, "src/main_mustache.c: bad precompile" },
 { -440, "src/main_mustache.c: bad template" },
 { -412, "src/main_mustache.c: calloc failed" },
 { -275, "src/main_mustache.c: mustache section render failed" },

};
#define            errs_list_size      sizeof(errs_list[0])
#define            errs_list_nelements sizeof(errs_list) / errs_list_size
