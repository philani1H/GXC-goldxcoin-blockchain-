# Development vs Production Server Comparison

## Quick Reference

| Aspect | Development Server | Production Server |
|--------|-------------------|-------------------|
| **Command** | `python admin_api.py` | `python wsgi_server.py` |
| **Server** | Flask built-in | Waitress WSGI |
| **Use Case** | Testing, debugging | Live deployment |
| **Debug Mode** | Enabled | Disabled |
| **Auto-reload** | Yes (on code changes) | No |
| **Performance** | Single-threaded | Multi-threaded |
| **Security** | Basic | Production-grade |
| **Concurrency** | Poor (1 request at a time) | Excellent (multiple threads) |
| **Stability** | Not recommended for production | Production-ready |

## When to Use Each

### Development Server (`python admin_api.py`)

✅ **Use for:**
- Local testing and debugging
- Development and code changes
- Quick prototyping
- Learning the API

❌ **Don't use for:**
- Production deployment
- Public-facing servers
- High-traffic scenarios
- Critical applications

**Warning shown:**
```
WARNING: This is a development server. Do not use it in a production deployment.
Use a production WSGI server instead.
```

### Production Server (`python wsgi_server.py`)

✅ **Use for:**
- Production deployment
- Public-facing APIs
- Multi-user environments
- High-traffic scenarios
- Critical business applications
- 24/7 uptime requirements

❌ **Don't use for:**
- Quick debugging (no auto-reload)
- Rapid development iteration

**Features:**
- No debug information leaked to clients
- Proper connection management
- Thread pooling for concurrency
- Production-grade error handling
- Secure header handling

## Performance Comparison

### Development Server
```
Concurrent Requests: 1
Threads: 1 (single-threaded)
Requests/sec: ~10-50 (limited)
Connection Handling: Basic
Error Recovery: Auto-restart only
```

### Production Server (Waitress)
```
Concurrent Requests: 1000 (configurable)
Threads: 4 (configurable, up to 8-16+)
Requests/sec: ~500-2000+ (depends on hardware)
Connection Handling: Advanced pooling
Error Recovery: Graceful degradation
```

## Starting the Servers

### Development Mode

**Direct:**
```bash
python admin_api.py
```

**Output:**
```
================================================================================
GXC Admin & Market Maker API - DEVELOPMENT MODE
================================================================================
Database: gxc_admin.db
WARNING: This is a development server!
For production, use: python wsgi_server.py
================================================================================
```

### Production Mode

**Windows:**
```bash
start_production.bat
```

**Linux/macOS:**
```bash
./start_production.sh
```

**Direct:**
```bash
python wsgi_server.py
```

**Output:**
```
================================================================================
GXC Admin & Market Maker API - Production Server
================================================================================
Database: gxc_admin.db
Server: Waitress WSGI Server
Host: 0.0.0.0
Port: 5002
Threads: 4
================================================================================
Server is ready and listening...
```

## Configuration Options

### Development Server

Limited configuration:
- Port: Hardcoded to 5002 (edit code to change)
- Host: 0.0.0.0
- Debug: Always enabled

### Production Server

Flexible environment variables:
- `SERVER_HOST` - Bind address (default: 0.0.0.0)
- `SERVER_PORT` - Port number (default: 5002)
- `SERVER_THREADS` - Worker threads (default: 4)
- `ADMIN_DB_PATH` - Database path

**Example:**
```bash
export SERVER_PORT=8080
export SERVER_THREADS=8
python wsgi_server.py
```

## Security Differences

### Development Server
- ⚠️ Exposes debug information
- ⚠️ Detailed error messages to clients
- ⚠️ Not hardened against attacks
- ⚠️ Single point of failure

### Production Server
- ✅ No debug information exposed
- ✅ Generic error messages to clients
- ✅ Secure header handling
- ✅ Connection limits and timeouts
- ✅ Better DoS protection
- ✅ Untrusted proxy header filtering

## Migration Checklist

Moving from development to production:

- [ ] Install Waitress: `pip install waitress`
- [ ] Test production server locally first
- [ ] Set environment variables for configuration
- [ ] Update deployment scripts to use `wsgi_server.py`
- [ ] Configure reverse proxy (Nginx/Apache) if needed
- [ ] Set up SSL/TLS certificates
- [ ] Configure firewall rules
- [ ] Set up monitoring and logging
- [ ] Change default admin password
- [ ] Test all API endpoints
- [ ] Load test with expected traffic
- [ ] Document deployment process

## Common Issues

### "Address already in use"
**Problem:** Port 5002 is already taken

**Solution:**
```bash
# Find and kill the process
netstat -ano | findstr :5002  # Windows
lsof -i :5002                 # Linux/macOS

# Or change the port
export SERVER_PORT=8080
python wsgi_server.py
```

### "Module not found: waitress"
**Problem:** Waitress not installed

**Solution:**
```bash
pip install waitress
# or
pip install -r requirements.txt
```

### "Permission denied" on Linux
**Problem:** Port < 1024 requires root

**Solution:**
```bash
# Use port >= 1024
export SERVER_PORT=5002

# Or run with sudo (not recommended)
sudo python3 wsgi_server.py

# Better: Use reverse proxy
```

## Best Practices

1. **Always use production server for deployment**
   - Never deploy with Flask's built-in server

2. **Configure proper limits**
   - Set appropriate thread count for your workload
   - Monitor resource usage

3. **Use reverse proxy**
   - Nginx or Apache in front of Waitress
   - Handle SSL termination at proxy level

4. **Monitor your application**
   - Set up logging
   - Monitor error rates
   - Track response times

5. **Security first**
   - Change default credentials
   - Use HTTPS
   - Implement rate limiting
   - Regular security updates

## Summary

**Development:** Fast iteration, easy debugging, not for production.

**Production:** Stable, secure, performant, ready for real users.

**Rule of thumb:** If anyone besides you will use it → production server.
