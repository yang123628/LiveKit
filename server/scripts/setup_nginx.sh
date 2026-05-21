#!/bin/bash

set -e

echo "=== LiveKit Nginx-RTMP Setup Script ==="

if ! command -v nginx &> /dev/null; then
    echo "Nginx is not installed. Installing..."
    sudo apt-get update
    sudo apt-get install -y nginx
fi

RTMP_MODULE=$(nginx -V 2>&1 | grep -o "with-http_rtmp_module" || true)
if [ -z "$RTMP_MODULE" ]; then
    echo "Nginx does not have RTMP module. Installing nginx-rtmp..."
    if ! command -v dpkg-buildpackage &> /dev/null; then
        sudo apt-get install -y build-essential libpcre3 libpcre3-dev libssl-dev zlib1g-dev
    fi

    if [ ! -d /tmp/nginx-rtmp ]; then
        cd /tmp
        git clone https://github.com/arut/nginx-rtmp-module.git
        wget -q http://nginx.org/download/nginx-1.24.0.tar.gz
        tar xzf nginx-1.24.0.tar.gz
        cd nginx-1.24.0
        ./configure --add-module=/tmp/nginx-rtmp-module --with-http_ssl_module
        make
        sudo make install
    fi
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONF_SRC="$SCRIPT_DIR/nginx_rtmp.conf"
CONF_DST="/etc/nginx/nginx.conf"

echo "Writing Nginx-RTMP configuration to $CONF_DST..."
sudo cp "$CONF_SRC" "$CONF_DST"

echo "Creating required directories..."
sudo mkdir -p /var/log/nginx
sudo mkdir -p /usr/share/nginx/html

echo "Testing Nginx configuration..."
sudo nginx -t

echo "Restarting Nginx..."
sudo nginx -s reload 2>/dev/null || sudo nginx

echo ""
echo "=== Nginx-RTMP Setup Complete ==="
echo "RTMP server: rtmp://localhost/live/"
echo "HTTP proxy:  http://localhost:8080/"
echo ""
echo "Test push:   ffmpeg -re -i test.mp4 -c copy -f flv rtmp://localhost/live/test"
echo "Test play:   ffplay rtmp://localhost/live/test"
