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
  ngx_str_t           *value, *elt, name;
  ngx_http_variable_t *var;
  ngx_array_t       *langs_array;

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
  langs_array = ngx_array_create(cf->pool, cf->args->nelts - 1, sizeof(ngx_str_t));
  if (langs_array == NULL) {
    return NGX_CONF_ERROR;
  }
  var->data = (uintptr_t) langs_array;
  
  for (i = 2; i < cf->args->nelts; i++) {
    elt = ngx_array_push(langs_array); 
    if (elt == NULL) {
      return  NGX_CONF_ERROR;
    }
    
    *elt = value[i];
  }
  
  return NGX_CONF_OK;
}

static ngx_int_t ngx_http_accept_language_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data) 
{
  ngx_uint_t         i = 0;
  ngx_uint_t         found = 0;
  u_char            *start, *pos, *end;
  ngx_array_t       *langs_array = (ngx_array_t *) data;
  ngx_str_t         *langs = (ngx_str_t *) langs_array->elts;  

  if ( NULL != r->headers_in.accept_language ) {       
    start = r->headers_in.accept_language->value.data;
    end = start + r->headers_in.accept_language->value.len;

    while (start < end) {
      // eating spaces
      while (start < end && *start == ' ') {start++; }
      
      pos = start;
    
      while (pos < end && *pos != ',' && *pos != ';') { pos++; }
    
      for (i = 0; i < langs_array->nelts; i++)
      {      
        if ((ngx_uint_t)(pos - start) >= langs[i].len && ngx_strncasecmp(start, langs[i].data, langs[i].len) == 0) {
          found = 1;
          break;
        }
      }
      if (found)
        break;
      
      i = 0; // If not found default to the first language from the list
    
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

  v->data = langs[i].data;
  v->len  = langs[i].len;

  /* Set all required params */
  v->valid = 1;
  v->no_cacheable = 0;
  v->not_found = 0;
  return NGX_OK; 
}
