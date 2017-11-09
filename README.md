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

nginx compile with module's source the above.
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
  
  map $http_accept_language $lang {
    default en;
    ~ko ko; ~ru ru; ~ar ar; ~bg bg; ~ca ca; ~cs cs; ~da da; ~de de;
    ~el el; ~es es; ~fi fi; ~fy fy; ~fr fr; ~gl gl; ~hu hu; ~id id; ~it it; ~ja ja;
    ~nb nb; ~nl nl; ~pl pl; ~pt-br pt-br; ~pt-pt pt-pt; ~ro ro; ~sv sv; ~tr tr; ~uk uk;
    ~zh-hans zh-hans; ~zh-hant zh-hant; ~zh-cn zh-cn;
  }
  
}
```

virtualhost.conf
```sh
server {
  
  listen   80;
  server_name www.example.com;
  
  root /etc/nginx/html;
  
  if ($lang ~ (?i)ja) { 
    return 404;
  }
  
  if ($lang ~ ^$) {
    return 404;
  }
  
}
```
