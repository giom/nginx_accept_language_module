#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char *ngx_http_accept_language(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_accept_language_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);

static ngx_command_t  ngx_http_accept_language_commands[] = {

    { ngx_string("set_from_accept_language"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_accept_language,
      NGX_HTTP_MAIN_CONF_OFFSET,
      0,
      NULL },
      ngx_null_command
};

typedef struct ngx_http_accept_language_s {
  ngx_hash_t hash;
  ngx_str_t default_language;
} ngx_http_accept_language_t;

// No need for any configuration callback
static ngx_http_module_t  ngx_http_accept_language_module_ctx = {
    NULL,  NULL, NULL, NULL,  NULL, NULL, NULL, NULL 
};

ngx_module_t  ngx_http_accept_language_module = {
    NGX_MODULE_V1,
    &ngx_http_accept_language_module_ctx,       /* module context */
    ngx_http_accept_language_commands,          /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static char * ngx_http_accept_language(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  ngx_uint_t          i;
  ngx_str_t           *value, *snew, name;
  ngx_http_variable_t *var;
  ngx_hash_init_t  hash;
  ngx_http_accept_language_t *al;
  ngx_hash_keys_arrays_t  hash_keys;

  value = cf->args->elts;
  name = value[1];
  
  if (name.data[0] != '$') {
      ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "\"%V\" variable name should start with '$'", &name);
  } else {
      name.len--;
      name.data++;
  }
  
  var = ngx_http_add_variable(cf, &name, NGX_HTTP_VAR_CHANGEABLE);
  if (var == NULL) {
      return NGX_CONF_ERROR;
  }
  if (var->get_handler != NULL) {
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "variable already defined: \"%V\"", &name);
    return NGX_CONF_ERROR;
  }
  
  var->get_handler = ngx_http_accept_language_variable;

  al = ngx_pcalloc(cf->pool, sizeof(ngx_http_accept_language_t));
  if (al == NULL) {
      return NGX_CONF_ERROR;
  }
  
  hash_keys.pool = cf->pool;
  hash_keys.temp_pool = cf->temp_pool;

  ngx_hash_keys_array_init(&hash_keys, NGX_HASH_SMALL);
  
  for (i = 2; i < cf->args->nelts; i++) {
    if(al->default_language.len == 0){
      al->default_language = value[i];
    }
    snew = ngx_palloc(cf->pool, sizeof(ngx_str_t));
    if (snew == NULL) {
      return NGX_CONF_ERROR;
    }
    *snew = value[i];
    ngx_hash_add_key(&hash_keys, snew, snew, 0);
  }

  hash.hash = &al->hash;
  hash.key = ngx_hash_key_lc;
  hash.max_size = 512;
  hash.bucket_size = ngx_align(64, ngx_cacheline_size);
  hash.name = "accept_key_hash";
  hash.pool = cf->pool;
  hash.temp_pool = cf->temp_pool;

  if(ngx_hash_init(&hash, hash_keys.keys.elts, hash_keys.keys.nelts) != NGX_OK) {
      return NGX_CONF_ERROR;
  }

  var->data = (uintptr_t)al;
  
  return NGX_CONF_OK;
}

static ngx_int_t ngx_http_accept_language_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data) 
{
  u_char            *start, *pos, *end;
  ngx_http_accept_language_t    *al = (ngx_http_accept_language_t *) data;
  ngx_str_t         *l;
  ngx_uint_t   key;


  if ( NULL != r->headers_in.accept_language ) {       
    start = r->headers_in.accept_language->value.data;
    end = start + r->headers_in.accept_language->value.len;

    while (start < end) {
      // eating spaces
      while (start < end && *start == ' ') {start++; }
      
      pos = start;
    
      while (pos < end && *pos != ',' && *pos != ';') { pos++; }
    
      key = ngx_hash_key(start, end - start);
      l = (ngx_str_t *)ngx_hash_find(&al->hash, key, start, end - start);
      if(l != NULL){
        v->data = l->data;
        v->len  = l->len;
        goto set;
      }
    
      // We discard the quality value
      if (*pos == ';') {
        while (pos < end && *pos != ',') {pos++; }
      }
      if (*pos == ',') {
        pos++;
      }
      
      start = pos;
    }
  }

  v->data = al->default_language.data;
  v->len  = al->default_language.len;

set:
  /* Set all required params */
  v->valid = 1;
  v->no_cacheable = 0;
  v->not_found = 0;
  return NGX_OK; 
}
