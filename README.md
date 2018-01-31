# Nginx Accept Language module

This module parses the Accept-Language header and gives the most suitable locale for the user from a list of supported locales from your website.

THIS MODULE IS <b>FOR DYNIMIC MODULE</b> NOT STATIC MODULE ! 

IF YOU WANT A STATIC MODULE, USE https://github.com/giom/nginx_accept_language_module

### Install

extract module's source.
```sh
cd /tmp
wget https://github.com/hgati/nginx_accept_language_module/archive/master.zip
unzip master.zip
rm -f master.zip
```

nginx compile with module's source
```sh
NGINX_VERSION=1.13.6
cd /tmp
wget http://nginx.org/download/nginx-${NGINX_VERSION}.tar.gz
tar xvzf /tmp/nginx-${NGINX_VERSION}.tar.gz
cd nginx-${NGINX_VERSION}
./configure \
  --prefix=/etc/nginx \
  --sbin-path=/usr/sbin/nginx \
  --modules-path=/usr/lib/nginx/modules \
  --conf-path=/etc/nginx/nginx.conf \
  --pid-path=/var/run/nginx.pid \
  --lock-path=/var/run/nginx.lock \
  --http-log-path=/var/log/nginx/access.log \
  --error-log-path=/var/log/nginx/error.log \
  --with-pcre-jit \
  --with-http_ssl_module \
  --with-http_realip_module \
  --with-stream_ssl_module \
  --with-stream_realip_module \
  --with-http_stub_status_module \
  --with-http_gzip_static_module \
  --with-http_v2_module \
  --with-http_auth_request_module \
  --with-http_geoip_module \
  # for dynimic module
  --with-compat \
  --add-dynamic-module=/tmp/nginx_accept_language_module-master
make -j$(getconf _NPROCESSORS_ONLN)
make install
ln -s /usr/lib/nginx/modules /etc/nginx/modules
```

nginx.conf
```sh
load_module "modules/ngx_http_accept_language_module.so";

http {
  
  set_from_accept_language $lang en ko ru ar bg ca cs da de el es fi fy fr gl hu id it ja nb nl pl pt-br pt-pt ro sv tr uk zh-hans zh-hant zh-cn;
  
}
```

virtualhost.conf
```sh
server {
  
  listen   80;
  server_name www.example.com;
  
  root /etc/nginx/html;
  
  if ($lang ~ (?i)ja) { 
    return 302 http://www.lelong.com.my;
  }
  
  if ($lang ~ ^$) {
    return 302 http://www.lelong.com.my;
  }
  
}
```

### Usage

https://github.com/giom/nginx_accept_language_module
