#include <libfrozen.h>
#include <mustache.h>

#include <errors_list.c>
/**
 * @ingroup machine
 * @addtogroup mod_machine_mustache module/mustache
 */
/**
 * @ingroup mod_machine_mustache
 * @page page_mustache_info Description
 *
 */
/**
 * @ingroup mod_machine_mustache
 * @page page_mustache_config Configuration
 *  
 * Accepted configuration:
 * @code
 * {
 *              class                   = "modules/mustache",
 *              template                =                                      # template data
 *                                        (file_t){ ... }                      #  - from file
 *                                        (machine_t){ ... }                   #  - from machine
 *                                        ...
 *              },
 *              output                  = (hashkey_t)"buffer"                  # output result to, default "output"
 * }
 * @endcode
 */

#define HK_VALUE_template 21633 // TODO replace this with HDK
#define MUSTACHE_ERR 0
#define MUSTACHE_OK  1
#define ITER_OK 0

typedef struct mustache_userdata {
	mustache_template_t   *template;
	data_t                *tpl_data;
	hashkey_t              output;
} mustache_userdata;

typedef struct mustache_ctx {
	mustache_api_t        *api;
	request_t             *request;
	data_t                *output;
	
	mustache_template_t   *section;
	
	uintmax_t              error_lineno;
	char                  *error_msg;
} mustache_ctx;

typedef struct token_userdata {
	hashkey_t              key;
	format_t               format;
	machine_t             *enum_shop;
} token_userdata;

static machine_t c_mustache_sect_proto;

static ssize_t mustache_frozen_sect_handler(machine_t *machine, request_t *request){ // {{{
	mustache_ctx          *ctx               = (mustache_ctx *)machine->userdata;
	
	if(hash_data_find(request, HK(data)) == NULL) // EOF
		return 0;

	request_t r_next[] = {
		hash_inline(request),
		hash_inline(ctx->request),
		hash_end
	};
	mustache_ctx new_ctx = { ctx->api, r_next, ctx->output };
	
	if(mustache_render(ctx->api, &new_ctx, ctx->section) == 0)
		return error("mustache section render failed");
	
	return 0;
} // }}}

static uintmax_t mustache_frozen_read   (mustache_api_t *api, mustache_userdata *userdata, char *buffer, uintmax_t buffer_size){ // {{{
	fastcall_read r_read = { { 5, ACTION_READ }, 0, buffer, buffer_size };
	if( data_query(userdata->tpl_data, &r_read) < 0 )
		return MUSTACHE_ERR;
	
	return r_read.buffer_size;
} // }}}
static uintmax_t mustache_frozen_write  (mustache_api_t *api, mustache_ctx *ctx, char *buffer, uintmax_t buffer_size){ // {{{
	fastcall_write r_write = { { 5, ACTION_WRITE }, 0, buffer, buffer_size };
	if( data_query(ctx->output, &r_write) < 0 )
		return MUSTACHE_ERR;
	
	return r_write.buffer_size;
} // }}}
static uintmax_t mustache_frozen_varget (mustache_api_t *api, mustache_ctx *ctx, mustache_token_variable_t *token){ // {{{
	data_t                *data;
	token_userdata        *userdata          = (token_userdata *)token->userdata;
	
	if( (data = hash_data_find(ctx->request, userdata->key)) == NULL)
		return MUSTACHE_ERR;
	
	fastcall_convert_to r_convert = { { 5, ACTION_CONVERT_TO }, ctx->output, userdata->format };
	if(data_query(data, &r_convert) < 0)
		return MUSTACHE_ERR;
	
	return r_convert.transfered;
} // }}}
static uintmax_t mustache_frozen_sectget(mustache_api_t *api, mustache_ctx *ctx, mustache_token_section_t  *token){ // {{{
	data_t                *data;
	token_userdata        *token_data    = (token_userdata *)token->userdata;
	
	if( (data = hash_data_find(ctx->request, token_data->key)) == NULL){
		if(token->inverted == 1){
			return mustache_render(api, ctx, token->section);
		}
		return MUSTACHE_ERR;
	}
	
	ctx->section                    = token->section;
	token_data->enum_shop->userdata = ctx;
	
	data_t        d_shop = DATA_MACHINET(token_data->enum_shop);
	fastcall_enum r_enum = { { 4, ACTION_ENUM }, &d_shop };
	if(data_query(data, &r_enum) < 0)
		return MUSTACHE_ERR;
	
	return MUSTACHE_OK;
} // }}}
static void      mustache_frozen_error  (mustache_api_t *api, mustache_ctx *ctx, uintmax_t lineno, char *error){ // {{{
	ctx->error_lineno = lineno;
	ctx->error_msg    = strdup(error);
} // }}}
static void      mustache_frozen_freedata (mustache_api_t *api, token_userdata *userdata){ // {{{
	if(userdata->enum_shop)
		free(userdata->enum_shop);	
	free(userdata);
} // }}}

uintmax_t mustache_frozen_prevarget (mustache_api_t *api, mustache_ctx *ctx, mustache_token_variable_t *token){ // {{{
	char                  *split;
	token_userdata        *userdata;
	
	if( (userdata = token->userdata = malloc(sizeof(token_userdata))) == NULL)
		return MUSTACHE_ERR;
	
	userdata->format    = FORMAT(clean);
	userdata->enum_shop = NULL;
	
	if( (split = memchr(token->text, ':', token->text_length)) == NULL){
		data_t                 hashkey_str       = DATA_RAW(token->text, token->text_length);
		data_t                 hashkey_d         = DATA_PTR_HASHKEYT(&userdata->key);
		
		fastcall_convert_from  r_convert         = { { 4, ACTION_CONVERT_FROM }, &hashkey_str, FORMAT(config) };
		if(data_query(&hashkey_d, &r_convert) < 0)
			return MUSTACHE_ERR;
	}else{
		data_t                 hashkey_str       = DATA_RAW(token->text, (split - token->text));
		data_t                 format_str        = DATA_RAW(split + 1, token->text_length - (split - token->text) - 1);
		data_t                 hashkey_d         = DATA_PTR_HASHKEYT(&userdata->key);
		data_t                 format_d          = DATA_PTR_FORMATT(&userdata->format);
		
		fastcall_convert_from  r_convert         = { { 4, ACTION_CONVERT_FROM }, &hashkey_str, FORMAT(config) };
		if(data_query(&hashkey_d, &r_convert) < 0)
			return MUSTACHE_ERR;
		
		fastcall_convert_from  r_convert2        = { { 4, ACTION_CONVERT_FROM }, &format_str,  FORMAT(config) };
		if(data_query(&format_d, &r_convert2) < 0)
			return MUSTACHE_ERR;
	}
	return MUSTACHE_OK;
} // }}}
uintmax_t mustache_frozen_presectget(mustache_api_t *api, mustache_ctx *ctx, mustache_token_section_t  *token){ // {{{
	hashkey_t              hashkey;
	machine_t             *enum_shop;
	data_t                 hashkey_str       = DATA_STRING(token->name);
	data_t                 hashkey_d         = DATA_PTR_HASHKEYT(&hashkey);
	
	fastcall_convert_from r_convert = { { 4, ACTION_CONVERT_FROM }, &hashkey_str, FORMAT(config) };
	if(data_query(&hashkey_d, &r_convert) < 0)
		return MUSTACHE_ERR;
	
	if( (token->userdata = malloc(sizeof(token_userdata))) == NULL)
		return MUSTACHE_ERR;
	
	if( (enum_shop = malloc(sizeof(machine_t))) == NULL){
		free(token->userdata);
		return MUSTACHE_ERR;
	}
	memcpy(enum_shop, &c_mustache_sect_proto, sizeof(machine_t));
	
	((token_userdata *)token->userdata)->key       = hashkey;
	((token_userdata *)token->userdata)->enum_shop = enum_shop;
	
	if( (mustache_prerender(api, ctx, token->section)) == 0)
		return MUSTACHE_ERR;
		
	return MUSTACHE_OK;
} // }}}

mustache_api_t mustache_api = {
	.read     = (mustache_api_read)&mustache_frozen_read,
	.write    = (mustache_api_write)&mustache_frozen_write,
	.varget   = (mustache_api_varget)&mustache_frozen_varget,
	.sectget  = (mustache_api_sectget)&mustache_frozen_sectget,
	.error    = (mustache_api_error)&mustache_frozen_error,
	.freedata = (mustache_api_freedata)&mustache_frozen_freedata
};

static ssize_t mustache_init(machine_t *machine){ // {{{
	mustache_userdata         *userdata;

	if((userdata = machine->userdata = calloc(1, sizeof(mustache_userdata))) == NULL)
		return error("calloc failed");
	
	userdata->output = HK(output);
	return MUSTACHE_ERR;
} // }}}
static ssize_t mustache_destroy(machine_t *machine){ // {{{
	mustache_userdata     *userdata          = (mustache_userdata *)machine->userdata;
	
	if(userdata->template)
		mustache_free(&mustache_api, userdata->template);
	
	free(userdata);
	return MUSTACHE_ERR;
} // }}}
static ssize_t mustache_configure(machine_t *machine, config_t *config){ // {{{
	ssize_t                ret;
	data_t                *tpl_data;
	mustache_userdata     *userdata          = (mustache_userdata *)machine->userdata;
	
	hash_data_get(ret, TYPE_HASHKEYT, userdata->output, config, HK(output));
	
	if( (tpl_data = hash_data_find(config, HK(template))) == NULL)
		return -EINVAL;
	
	data_t dslide = DATA_SLIDERT(tpl_data, 0);
	userdata->tpl_data = &dslide;
	
	if( (userdata->template = mustache_compile(&mustache_api, userdata)) == NULL)
		return error("bad template");
	
	mustache_api_t pre_api = {
		.varget  = (mustache_api_varget)&mustache_frozen_prevarget,
		.sectget = (mustache_api_sectget)&mustache_frozen_presectget
	};
	if( (mustache_prerender(&pre_api, userdata, userdata->template)) == 0)
		return error("bad precompile");
	
	return MUSTACHE_ERR;
} // }}}

static ssize_t mustache_handler(machine_t *machine, request_t *request){ // {{{
	data_t                *output;
	mustache_userdata     *userdata = (mustache_userdata *)machine->userdata;
	
	if( (output = hash_data_find(request, userdata->output)) == NULL)
		return error("buffer not supplied");
	
	data_t                 dslide   = DATA_SLIDERT(output, 0);
	mustache_ctx           ctx      = { &mustache_api, request, &dslide };
	
	if(mustache_render(&mustache_api, &ctx, userdata->template) == 0)
		return error("mustache_render failed");
	
	return machine_pass(machine, request);
} // }}}

static machine_t c_mustache_proto = {
	.class          = "modules/mustache",
	.supported_api  = API_HASH,
	.func_init      = &mustache_init,
	.func_configure = &mustache_configure,
	.func_destroy   = &mustache_destroy,
	.machine_type_hash = {
		.func_handler = &mustache_handler
	}
};

static machine_t c_mustache_sect_proto = {
	.supported_api  = API_HASH,
	.machine_type_hash = {
		.func_handler = &mustache_frozen_sect_handler
	}
};

int main(void){
	errors_register((err_item *)&errs_list, &emodule);
	class_register(&c_mustache_proto);
	return MUSTACHE_ERR;
}
