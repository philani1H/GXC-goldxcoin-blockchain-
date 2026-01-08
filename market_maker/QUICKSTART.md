# GXC Admin API - Quick Start Guide

## 🚀 Getting Started in 2 Minutes

### 1. Install Dependencies

```bash
pip install -r requirements.txt
```

This installs:
- Flask (web framework)
- Waitress (production WSGI server)
- requests (HTTP library)

### 2. Choose Your Mode

#### Development Mode (for testing)
```bash
python admin_api.py
```

#### Production Mode (recommended)
**Windows:**
```bash
start_production.bat
```

**Linux/macOS:**
```bash
chmod +x start_production.sh
./start_production.sh
```

**Manual:**
```bash
python wsgi_server.py
```

### 3. Access the API

Server runs on: `http://localhost:5002`

## 📋 Default Credentials

**Super Admin:**
- Username: `Philani-GXC.Foundation`
- Password: `GXC_SuperAdmin_2024!`

⚠️ **IMPORTANT:** Change this password immediately after first login!

## 🧪 Test the API

### 1. Login as Admin

```bash
curl -X POST http://localhost:5002/api/v1/admin/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "Philani-GXC.Foundation",
    "password": "GXC_SuperAdmin_2024!"
  }'
```

**Response:**
```json
{
  "success": true,
  "sessionToken": "SESSION_admin_philani_gxc_foundation_1234567890_abc123",
  "adminId": "admin_philani_gxc_foundation",
  "role": "super_admin",
  "permissions": ["all"],
  "expiresAt": 1234567890
}
```

**Save the `sessionToken` for subsequent requests!**

### 2. Submit Market Maker Application

```bash
curl -X POST http://localhost:5002/api/v1/marketmaker/apply \
  -H "Content-Type: application/json" \
  -d '{
    "applicantAddress": "GXC1234567890ABCDEF",
    "companyName": "Example Trading LLC",
    "licenseNumber": "FCA-12345",
    "regulatoryBody": "FCA",
    "country": "United Kingdom",
    "contactEmail": "compliance@example.com",
    "contactPhone": "+44 20 1234 5678",
    "website": "https://example-trading.com"
  }'
```

**Response:**
```json
{
  "success": true,
  "applicationId": "APP_1234567890_abc123",
  "status": "PENDING",
  "message": "Application submitted successfully. You will be notified of the review status.",
  "estimatedReviewTime": "5-7 business days"
}
```

**Save the `applicationId` to check status later!**

### 3. Check Application Status

```bash
curl "http://localhost:5002/api/v1/marketmaker/status?applicationId=APP_1234567890_abc123"
```

### 4. Admin: View Pending Applications

```bash
curl -X GET http://localhost:5002/api/v1/admin/applications/pending \
  -H "Authorization: Bearer SESSION_admin_philani_gxc_foundation_1234567890_abc123"
```

### 5. Admin: Verify a Step

```bash
curl -X POST http://localhost:5002/api/v1/admin/verify/license \
  -H "Authorization: Bearer SESSION_admin_philani_gxc_foundation_1234567890_abc123" \
  -H "Content-Type: application/json" \
  -d '{
    "applicationId": "APP_1234567890_abc123",
    "passed": true,
    "notes": "License verified with FCA database"
  }'
```

### 6. Admin: Approve Application

```bash
curl -X POST http://localhost:5002/api/v1/admin/approve \
  -H "Authorization: Bearer SESSION_admin_philani_gxc_foundation_1234567890_abc123" \
  -H "Content-Type: application/json" \
  -d '{
    "applicationId": "APP_1234567890_abc123"
  }'
```

## 📚 All API Endpoints

### Public Endpoints (No Auth Required)

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/api/v1/admin/login` | Admin login |
| POST | `/api/v1/marketmaker/apply` | Submit MM application |
| GET | `/api/v1/marketmaker/status` | Check application status |

### Admin Endpoints (Require Auth Header)

| Method | Endpoint | Description | Permission |
|--------|----------|-------------|------------|
| POST | `/api/v1/admin/create` | Create new admin | super_admin only |
| GET | `/api/v1/admin/list` | List all admins | super_admin only |
| GET | `/api/v1/admin/applications/pending` | Get pending apps | view_applications |
| POST | `/api/v1/admin/verify/<type>` | Verify step | Any admin |
| POST | `/api/v1/admin/approve` | Approve application | approve_applications |
| POST | `/api/v1/admin/reject` | Reject application | reject_applications |

**Verification types:** `license`, `reputation`, `financial`, `technical`, `kyc`

## 🔐 Authentication

All admin endpoints require an `Authorization` header:

```
Authorization: Bearer <sessionToken>
```

Example:
```bash
curl -H "Authorization: Bearer SESSION_admin_philani_gxc_foundation_1234567890_abc123" \
  http://localhost:5002/api/v1/admin/list
```

Session tokens expire after 24 hours.

## ⚙️ Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `SERVER_HOST` | `0.0.0.0` | Server bind address |
| `SERVER_PORT` | `5002` | Server port |
| `SERVER_THREADS` | `4` | Worker threads |
| `ADMIN_DB_PATH` | `gxc_admin.db` | Database file path |

### Custom Configuration Example

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

## 🗂️ Project Files

```
market_maker/
├── admin_api.py                    # Main API application
├── wsgi_server.py                  # Production WSGI server
├── start_production.bat            # Windows startup script
├── start_production.sh             # Unix/Linux startup script
├── requirements.txt                # Python dependencies
├── QUICKSTART.md                   # This file
├── PRODUCTION_DEPLOYMENT.md        # Full deployment guide
├── DEVELOPMENT_VS_PRODUCTION.md    # Server comparison
└── gxc_admin.db                    # SQLite database (auto-created)
```

## 🐛 Troubleshooting

### Server won't start

**Check if port is in use:**
```bash
# Windows
netstat -ano | findstr :5002

# Linux/macOS
lsof -i :5002
```

**Kill existing process:**
```bash
# Windows
taskkill /PID <PID> /F

# Linux/macOS
kill -9 <PID>
```

### Dependencies missing

```bash
pip install -r requirements.txt
```

### Database errors

Delete and recreate:
```bash
# Remove old database
rm gxc_admin.db  # Linux/macOS
del gxc_admin.db  # Windows

# Restart server (auto-creates new DB)
python wsgi_server.py
```

### Can't access from network

Make sure `SERVER_HOST` is set to `0.0.0.0`:
```bash
export SERVER_HOST=0.0.0.0
python wsgi_server.py
```

Check your firewall allows port 5002.

## 📖 Next Steps

1. **Read full documentation:**
   - [PRODUCTION_DEPLOYMENT.md](PRODUCTION_DEPLOYMENT.md) - Complete deployment guide
   - [DEVELOPMENT_VS_PRODUCTION.md](DEVELOPMENT_VS_PRODUCTION.md) - Server comparison

2. **Security setup:**
   - Change default admin password
   - Set up HTTPS (via reverse proxy)
   - Configure firewall rules

3. **Production deployment:**
   - Set up reverse proxy (Nginx/Apache)
   - Configure systemd service (Linux)
   - Implement monitoring and logging

4. **Integration:**
   - Connect to your blockchain node
   - Integrate with frontend application
   - Set up automated testing

## 💡 Tips

- **Development:** Use `python admin_api.py` for quick testing
- **Production:** Always use `python wsgi_server.py` for deployment
- **Testing:** Use tools like Postman or curl for API testing
- **Monitoring:** Check server logs regularly
- **Security:** Never expose development server to the internet

## 🆘 Support

Having issues? Check:
1. Error messages in terminal
2. Database file permissions
3. Port availability
4. Python version (3.7+ required)
5. All dependencies installed

For detailed help, see [PRODUCTION_DEPLOYMENT.md](PRODUCTION_DEPLOYMENT.md).

---

**Ready to deploy? Use the production server!**

```bash
python wsgi_server.py
```
