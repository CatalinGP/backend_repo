#!/bin/bash

echo "🚀 Starting NGINX Deployment..."

# Define certificate paths
SSL_DIR="/etc/nginx/certs"
CERT_FILE="$SSL_DIR/selfsigned.crt"
KEY_FILE="$SSL_DIR/selfsigned.key"

# Ensure the SSL directory exists
if [ ! -d "$SSL_DIR" ]; then
    echo "📁 Creating SSL directory..."
    sudo mkdir -p "$SSL_DIR"
fi

# Generate a self-signed certificate if it doesn't exist
if [ ! -f "$CERT_FILE" ] || [ ! -f "$KEY_FILE" ]; then
    echo "🔐 Generating a self-signed SSL certificate..."
    sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
        -keyout "$KEY_FILE" -out "$CERT_FILE" \
        -subj "/C=US/ST=Local/L=Local/O=Dev/CN=localhost"
else
    echo "✅ SSL certificate already exists. Skipping generation."
fi

# Define NGINX config paths
NGINX_CONF_SOURCE="/home/nicu/PoC/PoC/src/backend/rest_api/nginx.conf"
NGINX_CONF_DEST="/etc/nginx/sites-available/flask-app"
NGINX_ENABLED="/etc/nginx/sites-enabled/flask-app"

# Copy or link the NGINX configuration
echo "📝 Deploying NGINX configuration..."
sudo ln -sf "$NGINX_CONF_SOURCE" "$NGINX_CONF_DEST"

# Enable the site in NGINX
sudo ln -sf "$NGINX_CONF_DEST" "$NGINX_ENABLED"

# Test NGINX configuration
echo "🔍 Testing NGINX configuration..."
sudo nginx -t

# Restart NGINX if the test passes
if [ $? -eq 0 ]; then
    echo "♻️ Restarting NGINX..."
    sudo systemctl restart nginx
    echo "✅ NGINX successfully restarted!"
else
    echo "❌ NGINX configuration test failed. Check for errors!"
    exit 1
fi
