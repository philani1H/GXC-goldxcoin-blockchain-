# GXC Admin API - Production Deployment Guide

## Overview

The GXC Admin & Market Maker API uses **Waitress** as the production WSGI server, replacing Flask's built-in development server for better performance, stability, and security.

## Production Server Features

- **Multi-threaded**: Handles concurrent requests efficiently
- **Production-grade**: Designed for real-world deployment
- **Cross-platform**: Works on Windows, Linux, and macOS
- **Secure**: Proper header handling and connection limits
- **Stable**: No debug mode, optimized for uptime

## Quick Start

### Windows

Simply double-click or run:
```bash
start_production.bat
```

### Linux/macOS

```bash
chmod +x start_production.sh
./start_production.sh
```

### Manual Start

```bash
python wsgi_server.py
```

## Configuration

### Environment Variables

You can customize the server by setting these environment variables:

| Variable | Default | Description |
|----------|---------|-------------|
| `SERVER_HOST` | `0.0.0.0` | Server bind address |
| `SERVER_PORT` | `5002` | Server port |
| `SERVER_THREADS` | `4` | Number of worker threads |
| `ADMIN_DB_PATH` | `gxc_admin.db` | Database file path |

### Example: Custom Configuration

**Windows:**
```batch
set SERVER_PORT=8080
set SERVER_THREADS=8
python wsgi_server.py
```

**Linux/macOS:**
```bash
export SERVER_PORT=8080
export SERVER_THREADS=8
python3 wsgi_server.py
```

## Server Specifications

### Waitress Configuration

The production server is configured with:
- **Connection limit**: 1000 concurrent connections
- **Channel timeout**: 120 seconds
- **Cleanup interval**: 30 seconds
- **Clear untrusted proxy headers**: Enabled (security)

### Performance Tuning

For high-traffic scenarios, adjust these settings:

1. **Increase threads** (CPU-bound tasks):
   ```bash
   export SERVER_THREADS=8
   ```

2. **Adjust connection limit** (edit `wsgi_server.py`):
   ```python
   connection_limit=2000  # Increase from 1000
   ```

3. **Database optimization**:
   - Use persistent storage path (not `/tmp`)
   - Enable WAL mode for SQLite
   - Consider PostgreSQL for high concurrency

## Deployment Scenarios

### Local Network Deployment

Bind to all interfaces (default):
```bash
SERVER_HOST=0.0.0.0 SERVER_PORT=5002 python wsgi_server.py
```

Access from:
- Local: `http://127.0.0.1:5002`
- Network: `http://<your-ip>:5002`

### Behind Reverse Proxy (Nginx/Apache)

Bind to localhost only:
```bash
SERVER_HOST=127.0.0.1 SERVER_PORT=5002 python wsgi_server.py
```

**Nginx example:**
```nginx
server {
    listen 80;
    server_name api.goldxcoin.com;

    location / {
        proxy_pass http://127.0.0.1:5002;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

### Docker Deployment

**Dockerfile:**
```dockerfile
FROM python:3.11-slim

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

ENV SERVER_HOST=0.0.0.0
ENV SERVER_PORT=5002
ENV SERVER_THREADS=4

EXPOSE 5002

CMD ["python", "wsgi_server.py"]
```

**docker-compose.yml:**
```yaml
version: '3.8'
services:
  gxc-admin-api:
    build: .
    ports:
      - "5002:5002"
    environment:
      - SERVER_THREADS=8
      - ADMIN_DB_PATH=/data/gxc_admin.db
    volumes:
      - ./data:/data
    restart: unless-stopped
```

### Systemd Service (Linux)

Create `/etc/systemd/system/gxc-admin-api.service`:

```ini
[Unit]
Description=GXC Admin & Market Maker API
After=network.target

[Service]
Type=simple
User=gxc
WorkingDirectory=/opt/gxc-admin-api
Environment="SERVER_HOST=127.0.0.1"
Environment="SERVER_PORT=5002"
Environment="SERVER_THREADS=4"
Environment="ADMIN_DB_PATH=/var/lib/gxc/admin.db"
ExecStart=/usr/bin/python3 /opt/gxc-admin-api/wsgi_server.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl enable gxc-admin-api
sudo systemctl start gxc-admin-api
sudo systemctl status gxc-admin-api
```

## Security Considerations

### Production Checklist

- [ ] Change default admin password immediately
- [ ] Use HTTPS (via reverse proxy or load balancer)
- [ ] Restrict `SERVER_HOST` to specific interfaces if needed
- [ ] Set up firewall rules
- [ ] Enable database encryption if handling sensitive data
- [ ] Implement rate limiting (via reverse proxy)
- [ ] Regular security updates
- [ ] Monitor logs for suspicious activity
- [ ] Use environment variables for secrets (never hardcode)

### HTTPS Setup

**Option 1: Reverse Proxy (Recommended)**
Use Nginx/Apache with Let's Encrypt SSL certificates.

**Option 2: Application Level**
Waitress supports SSL, but reverse proxy is preferred.

## Monitoring & Logging

### Access Logs

Waitress logs to stdout/stderr by default. Redirect to files:

**Windows:**
```batch
python wsgi_server.py > access.log 2>&1
```

**Linux:**
```bash
python3 wsgi_server.py >> /var/log/gxc-api/access.log 2>&1
```

### Process Monitoring

Use tools like:
- **Windows**: Task Manager, NSSM (service wrapper)
- **Linux**: systemd, supervisor, PM2

## Troubleshooting

### Server won't start

1. Check if port is already in use:
   ```bash
   netstat -ano | findstr :5002  # Windows
   lsof -i :5002                 # Linux/macOS
   ```

2. Verify Waitress is installed:
   ```bash
   pip list | grep waitress
   ```

3. Check database permissions:
   ```bash
   ls -la gxc_admin.db  # Linux/macOS
   dir gxc_admin.db     # Windows
   ```

### Performance Issues

1. Increase thread count
2. Check database location (use SSD, not network drive)
3. Monitor CPU/memory usage
4. Consider connection pooling for database

### Connection Errors

1. Check firewall settings
2. Verify `SERVER_HOST` binding
3. Test with `curl` or `telnet`:
   ```bash
   curl http://localhost:5002/api/v1/marketmaker/status?applicationId=test
   ```

## Migration from Development Server

The Flask development server (`python admin_api.py`) will still work for testing, but displays a warning:

```
WARNING: This is a development server!
For production, use: python wsgi_server.py
```

**To migrate:**
1. Install Waitress: `pip install waitress`
2. Use production scripts: `start_production.bat` or `start_production.sh`
3. Update deployment scripts/services to use `wsgi_server.py`

## API Endpoints (Unchanged)

All API endpoints remain the same:

- `POST /api/v1/admin/create` - Create admin
- `POST /api/v1/admin/login` - Admin login
- `GET /api/v1/admin/list` - List admins
- `POST /api/v1/marketmaker/apply` - Submit application
- `GET /api/v1/marketmaker/status` - Check status
- `GET /api/v1/admin/applications/pending` - Get pending
- `POST /api/v1/admin/verify/<type>` - Verify step
- `POST /api/v1/admin/approve` - Approve application
- `POST /api/v1/admin/reject` - Reject application

## Support

For issues or questions:
- Check logs for error messages
- Review this deployment guide
- Test with development server first
- Verify network/firewall configuration
