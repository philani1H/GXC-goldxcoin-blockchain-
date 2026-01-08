@echo off
REM GXC Admin API Production Server - Windows Startup Script

echo ========================================
echo GXC Admin API - Production Server
echo ========================================

REM Set environment variables (customize as needed)
set SERVER_HOST=0.0.0.0
set SERVER_PORT=5002
set SERVER_THREADS=4
set ADMIN_DB_PATH=gxc_admin.db

echo Starting production server...
echo Host: %SERVER_HOST%
echo Port: %SERVER_PORT%
echo Threads: %SERVER_THREADS%
echo Database: %ADMIN_DB_PATH%
echo.

python wsgi_server.py

pause
