#!/usr/bin/env python3
"""
GXC Admin & Market Maker Verification API
Complete implementation of admin management and market maker verification
"""

import os
import sqlite3
import hashlib
import secrets
import time
from datetime import datetime
from flask import Flask, request, jsonify
from functools import wraps

app = Flask(__name__)

# Database path
DB_PATH = os.environ.get('ADMIN_DB_PATH', '/tmp/gxc_admin.db' if os.path.exists('/tmp') else 'gxc_admin.db')

# Initialize database
def init_database():
    """Initialize admin and market maker database"""
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    # Admin users table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS admins (
            admin_id TEXT PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL,
            permissions TEXT NOT NULL,
            is_active BOOLEAN DEFAULT TRUE,
            created_at INTEGER NOT NULL,
            last_login_at INTEGER,
            created_by TEXT
        )
    ''')
    
    # Admin sessions table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS admin_sessions (
            session_token TEXT PRIMARY KEY,
            admin_id TEXT NOT NULL,
            created_at INTEGER NOT NULL,
            expires_at INTEGER NOT NULL,
            FOREIGN KEY (admin_id) REFERENCES admins(admin_id)
        )
    ''')
    
    # Market maker applications table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS mm_applications (
            application_id TEXT PRIMARY KEY,
            applicant_address TEXT NOT NULL,
            company_name TEXT NOT NULL,
            license_number TEXT NOT NULL,
            regulatory_body TEXT NOT NULL,
            country TEXT NOT NULL,
            contact_email TEXT NOT NULL,
            contact_phone TEXT,
            website TEXT,
            license_document_hash TEXT,
            financial_statements_hash TEXT,
            technical_capabilities_hash TEXT,
            kyc_documents_hash TEXT,
            status TEXT DEFAULT 'PENDING',
            submitted_at INTEGER NOT NULL,
            last_updated_at INTEGER NOT NULL,
            reviewed_by TEXT,
            approved_at INTEGER,
            rejected_at INTEGER,
            rejection_reason TEXT
        )
    ''')
    
    # Verification steps table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS verification_steps (
            step_id TEXT PRIMARY KEY,
            application_id TEXT NOT NULL,
            step_name TEXT NOT NULL,
            status TEXT DEFAULT 'PENDING',
            passed BOOLEAN,
            verified_by TEXT,
            completed_at INTEGER,
            notes TEXT,
            FOREIGN KEY (application_id) REFERENCES mm_applications(application_id)
        )
    ''')
    
    # Audit log table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS audit_log (
            log_id TEXT PRIMARY KEY,
            admin_id TEXT,
            action TEXT NOT NULL,
            target_type TEXT,
            target_id TEXT,
            details TEXT,
            timestamp INTEGER NOT NULL
        )
    ''')
    
    conn.commit()
    
    # Create super admin if not exists
    cursor.execute('SELECT COUNT(*) FROM admins WHERE role = ?', ('super_admin',))
    if cursor.fetchone()[0] == 0:
        # Create default super admin
        admin_id = 'admin_philani_gxc_foundation'
        username = 'Philani-GXC.Foundation'
        password = 'GXC_SuperAdmin_2024!'
        password_hash = hashlib.sha256(password.encode()).hexdigest()
        
        cursor.execute('''
            INSERT INTO admins (admin_id, username, password_hash, role, permissions, created_at)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (admin_id, username, password_hash, 'super_admin', 'all', int(time.time())))
        
        conn.commit()
        print(f"[ADMIN] Created super admin: {username}")
        print(f"[ADMIN] Default password: {password}")
        print(f"[ADMIN] Please change this password immediately!")
    
    conn.close()

# Initialize on import
init_database()

# Helper functions
def hash_password(password):
    """Hash password using SHA256"""
    return hashlib.sha256(password.encode()).hexdigest()

def generate_id(prefix):
    """Generate unique ID"""
    timestamp = int(time.time())
    random = secrets.token_hex(4)
    return f"{prefix}_{timestamp}_{random}"

def generate_session_token(admin_id):
    """Generate session token"""
    timestamp = int(time.time())
    random = secrets.token_hex(8)
    return f"SESSION_{admin_id}_{timestamp}_{random}"

def log_action(admin_id, action, target_type=None, target_id=None, details=None):
    """Log admin action"""
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    log_id = generate_id('LOG')
    cursor.execute('''
        INSERT INTO audit_log (log_id, admin_id, action, target_type, target_id, details, timestamp)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    ''', (log_id, admin_id, action, target_type, target_id, details, int(time.time())))
    
    conn.commit()
    conn.close()

# Authentication decorator
def require_auth(required_permission=None):
    """Require authentication and optional permission"""
    def decorator(f):
        @wraps(f)
        def decorated_function(*args, **kwargs):
            auth_header = request.headers.get('Authorization')
            if not auth_header or not auth_header.startswith('Bearer '):
                return jsonify({'success': False, 'error': 'Missing or invalid authorization header'}), 401
            
            session_token = auth_header.replace('Bearer ', '')
            
            conn = sqlite3.connect(DB_PATH)
            cursor = conn.cursor()
            
            # Check session
            cursor.execute('''
                SELECT s.admin_id, a.username, a.role, a.permissions, a.is_active
                FROM admin_sessions s
                JOIN admins a ON s.admin_id = a.admin_id
                WHERE s.session_token = ? AND s.expires_at > ?
            ''', (session_token, int(time.time())))
            
            result = cursor.fetchone()
            conn.close()
            
            if not result:
                return jsonify({'success': False, 'error': 'Invalid or expired session'}), 401
            
            admin_id, username, role, permissions, is_active = result
            
            if not is_active:
                return jsonify({'success': False, 'error': 'Admin account is inactive'}), 403
            
            # Check permission
            if required_permission:
                perms = permissions.split(',') if permissions != 'all' else ['all']
                if 'all' not in perms and required_permission not in perms:
                    return jsonify({'success': False, 'error': f'Missing permission: {required_permission}'}), 403
            
            # Add admin info to request
            request.admin_id = admin_id
            request.admin_username = username
            request.admin_role = role
            request.admin_permissions = permissions
            
            return f(*args, **kwargs)
        return decorated_function
    return decorator

# ========================================
# ADMIN MANAGEMENT ENDPOINTS
# ========================================

@app.route('/api/v1/admin/create', methods=['POST'])
@require_auth()
def create_admin():
    """Create new admin user (super admin only)"""
    if request.admin_role != 'super_admin':
        return jsonify({'success': False, 'error': 'Only super admin can create admins'}), 403
    
    data = request.json
    username = data.get('username')
    password = data.get('password')
    role = data.get('role', 'verifier')
    permissions = ','.join(data.get('permissions', []))
    
    if not username or not password:
        return jsonify({'success': False, 'error': 'Username and password required'}), 400
    
    admin_id = generate_id('ADMIN')
    password_hash = hash_password(password)
    
    try:
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO admins (admin_id, username, password_hash, role, permissions, created_at, created_by)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (admin_id, username, password_hash, role, permissions, int(time.time()), request.admin_id))
        
        conn.commit()
        conn.close()
        
        log_action(request.admin_id, 'CREATE_ADMIN', 'admin', admin_id, f'Created admin: {username}')
        
        return jsonify({
            'success': True,
            'adminId': admin_id,
            'username': username,
            'role': role,
            'permissions': permissions.split(','),
            'message': 'Admin created successfully'
        })
    except sqlite3.IntegrityError:
        return jsonify({'success': False, 'error': 'Username already exists'}), 400

@app.route('/api/v1/admin/login', methods=['POST'])
def admin_login():
    """Admin login"""
    data = request.json
    username = data.get('username')
    password = data.get('password')
    
    if not username or not password:
        return jsonify({'success': False, 'error': 'Username and password required'}), 400
    
    password_hash = hash_password(password)
    
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT admin_id, role, permissions, is_active
        FROM admins
        WHERE username = ? AND password_hash = ?
    ''', (username, password_hash))
    
    result = cursor.fetchone()
    
    if not result:
        conn.close()
        return jsonify({'success': False, 'error': 'Invalid credentials'}), 401
    
    admin_id, role, permissions, is_active = result
    
    if not is_active:
        conn.close()
        return jsonify({'success': False, 'error': 'Admin account is inactive'}), 403
    
    # Create session
    session_token = generate_session_token(admin_id)
    expires_at = int(time.time()) + (24 * 60 * 60)  # 24 hours
    
    cursor.execute('''
        INSERT INTO admin_sessions (session_token, admin_id, created_at, expires_at)
        VALUES (?, ?, ?, ?)
    ''', (session_token, admin_id, int(time.time()), expires_at))
    
    # Update last login
    cursor.execute('UPDATE admins SET last_login_at = ? WHERE admin_id = ?', (int(time.time()), admin_id))
    
    conn.commit()
    conn.close()
    
    log_action(admin_id, 'LOGIN', 'admin', admin_id)
    
    return jsonify({
        'success': True,
        'sessionToken': session_token,
        'adminId': admin_id,
        'role': role,
        'permissions': permissions.split(',') if permissions != 'all' else ['all'],
        'expiresAt': expires_at
    })

@app.route('/api/v1/admin/list', methods=['GET'])
@require_auth()
def list_admins():
    """List all admins (super admin only)"""
    if request.admin_role != 'super_admin':
        return jsonify({'success': False, 'error': 'Only super admin can list admins'}), 403
    
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT admin_id, username, role, permissions, is_active, created_at, last_login_at
        FROM admins
        ORDER BY created_at DESC
    ''')
    
    admins = []
    for row in cursor.fetchall():
        admins.append({
            'adminId': row[0],
            'username': row[1],
            'role': row[2],
            'permissions': row[3].split(',') if row[3] != 'all' else ['all'],
            'isActive': bool(row[4]),
            'createdAt': row[5],
            'lastLoginAt': row[6]
        })
    
    conn.close()
    
    return jsonify({
        'success': True,
        'admins': admins,
        'total': len(admins)
    })

# ========================================
# MARKET MAKER APPLICATION ENDPOINTS
# ========================================

@app.route('/api/v1/marketmaker/apply', methods=['POST'])
def submit_application():
    """Submit market maker application"""
    data = request.json
    
    required_fields = ['applicantAddress', 'companyName', 'licenseNumber', 'regulatoryBody', 'country', 'contactEmail']
    for field in required_fields:
        if not data.get(field):
            return jsonify({'success': False, 'error': f'Missing required field: {field}'}), 400
    
    application_id = generate_id('APP')
    timestamp = int(time.time())
    
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    cursor.execute('''
        INSERT INTO mm_applications (
            application_id, applicant_address, company_name, license_number,
            regulatory_body, country, contact_email, contact_phone, website,
            license_document_hash, financial_statements_hash, technical_capabilities_hash,
            kyc_documents_hash, submitted_at, last_updated_at
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    ''', (
        application_id, data['applicantAddress'], data['companyName'], data['licenseNumber'],
        data['regulatoryBody'], data['country'], data['contactEmail'], data.get('contactPhone'),
        data.get('website'), data.get('licenseDocumentHash'), data.get('financialStatementsHash'),
        data.get('technicalCapabilitiesHash'), data.get('kycDocumentsHash'), timestamp, timestamp
    ))
    
    # Create verification steps
    steps = [
        'License Verification',
        'Company Reputation Check',
        'Financial Standing Review',
        'Technical Capabilities Verification',
        'KYC/AML Compliance'
    ]
    
    for step in steps:
        step_id = generate_id('STEP')
        cursor.execute('''
            INSERT INTO verification_steps (step_id, application_id, step_name)
            VALUES (?, ?, ?)
        ''', (step_id, application_id, step))
    
    conn.commit()
    conn.close()
    
    return jsonify({
        'success': True,
        'applicationId': application_id,
        'status': 'PENDING',
        'message': 'Application submitted successfully. You will be notified of the review status.',
        'estimatedReviewTime': '5-7 business days'
    })

@app.route('/api/v1/marketmaker/status', methods=['GET'])
def check_application_status():
    """Check application status"""
    application_id = request.args.get('applicationId')
    
    if not application_id:
        return jsonify({'success': False, 'error': 'Application ID required'}), 400
    
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT company_name, status, submitted_at, last_updated_at
        FROM mm_applications
        WHERE application_id = ?
    ''', (application_id,))
    
    app_data = cursor.fetchone()
    
    if not app_data:
        conn.close()
        return jsonify({'success': False, 'error': 'Application not found'}), 404
    
    # Get verification steps
    cursor.execute('''
        SELECT step_name, status, passed, completed_at
        FROM verification_steps
        WHERE application_id = ?
        ORDER BY step_id
    ''', (application_id,))
    
    steps = []
    for row in cursor.fetchall():
        steps.append({
            'stepName': row[0],
            'status': row[1],
            'passed': row[2],
            'completedAt': row[3]
        })
    
    conn.close()
    
    return jsonify({
        'success': True,
        'applicationId': application_id,
        'companyName': app_data[0],
        'status': app_data[1],
        'submittedAt': app_data[2],
        'lastUpdatedAt': app_data[3],
        'verificationSteps': steps
    })

@app.route('/api/v1/admin/applications/pending', methods=['GET'])
@require_auth('view_applications')
def get_pending_applications():
    """Get pending applications"""
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT application_id, company_name, license_number, regulatory_body,
               country, contact_email, submitted_at, status
        FROM mm_applications
        WHERE status IN ('PENDING', 'UNDER_REVIEW')
        ORDER BY submitted_at DESC
    ''')
    
    applications = []
    for row in cursor.fetchall():
        applications.append({
            'applicationId': row[0],
            'companyName': row[1],
            'licenseNumber': row[2],
            'regulatoryBody': row[3],
            'country': row[4],
            'contactEmail': row[5],
            'submittedAt': row[6],
            'status': row[7]
        })
    
    conn.close()
    
    return jsonify({
        'success': True,
        'applications': applications,
        'total': len(applications)
    })

@app.route('/api/v1/admin/verify/<step_type>', methods=['POST'])
@require_auth()
def verify_step(step_type):
    """Verify a specific step"""
    data = request.json
    application_id = data.get('applicationId')
    passed = data.get('passed')
    notes = data.get('notes', '')
    
    if not application_id or passed is None:
        return jsonify({'success': False, 'error': 'Application ID and passed status required'}), 400
    
    step_names = {
        'license': 'License Verification',
        'reputation': 'Company Reputation Check',
        'financial': 'Financial Standing Review',
        'technical': 'Technical Capabilities Verification',
        'kyc': 'KYC/AML Compliance'
    }
    
    step_name = step_names.get(step_type)
    if not step_name:
        return jsonify({'success': False, 'error': 'Invalid step type'}), 400
    
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    cursor.execute('''
        UPDATE verification_steps
        SET status = 'COMPLETED', passed = ?, verified_by = ?, completed_at = ?, notes = ?
        WHERE application_id = ? AND step_name = ?
    ''', (passed, request.admin_id, int(time.time()), notes, application_id, step_name))
    
    # Update application status
    cursor.execute('UPDATE mm_applications SET status = ?, last_updated_at = ? WHERE application_id = ?',
                  ('UNDER_REVIEW', int(time.time()), application_id))
    
    conn.commit()
    conn.close()
    
    log_action(request.admin_id, f'VERIFY_{step_type.upper()}', 'application', application_id,
              f'Passed: {passed}, Notes: {notes}')
    
    return jsonify({
        'success': True,
        'message': f'{step_name} completed',
        'passed': passed
    })

@app.route('/api/v1/admin/approve', methods=['POST'])
@require_auth('approve_applications')
def approve_application():
    """Approve market maker application"""
    data = request.json
    application_id = data.get('applicationId')
    
    if not application_id:
        return jsonify({'success': False, 'error': 'Application ID required'}), 400
    
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    # Check all steps passed
    cursor.execute('''
        SELECT COUNT(*) FROM verification_steps
        WHERE application_id = ? AND (passed = 0 OR passed IS NULL)
    ''', (application_id,))
    
    if cursor.fetchone()[0] > 0:
        conn.close()
        return jsonify({'success': False, 'error': 'All verification steps must pass before approval'}), 400
    
    cursor.execute('''
        UPDATE mm_applications
        SET status = 'APPROVED', approved_at = ?, reviewed_by = ?, last_updated_at = ?
        WHERE application_id = ?
    ''', (int(time.time()), request.admin_id, int(time.time()), application_id))
    
    conn.commit()
    conn.close()
    
    log_action(request.admin_id, 'APPROVE_APPLICATION', 'application', application_id)
    
    return jsonify({
        'success': True,
        'applicationId': application_id,
        'status': 'APPROVED',
        'message': 'Application approved successfully'
    })

@app.route('/api/v1/admin/reject', methods=['POST'])
@require_auth('reject_applications')
def reject_application():
    """Reject market maker application"""
    data = request.json
    application_id = data.get('applicationId')
    reason = data.get('reason', 'Application did not meet requirements')
    
    if not application_id:
        return jsonify({'success': False, 'error': 'Application ID required'}), 400
    
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    cursor.execute('''
        UPDATE mm_applications
        SET status = 'REJECTED', rejected_at = ?, reviewed_by = ?, rejection_reason = ?, last_updated_at = ?
        WHERE application_id = ?
    ''', (int(time.time()), request.admin_id, reason, int(time.time()), application_id))
    
    conn.commit()
    conn.close()
    
    log_action(request.admin_id, 'REJECT_APPLICATION', 'application', application_id, f'Reason: {reason}')
    
    return jsonify({
        'success': True,
        'applicationId': application_id,
        'status': 'REJECTED',
        'reason': reason,
        'message': 'Application rejected'
    })

if __name__ == '__main__':
    print("=" * 80)
    print("GXC Admin & Market Maker API")
    print("=" * 80)
    print(f"Database: {DB_PATH}")
    print("Endpoints:")
    print("  POST /api/v1/admin/create - Create admin")
    print("  POST /api/v1/admin/login - Admin login")
    print("  GET  /api/v1/admin/list - List admins")
    print("  POST /api/v1/marketmaker/apply - Submit application")
    print("  GET  /api/v1/marketmaker/status - Check status")
    print("  GET  /api/v1/admin/applications/pending - Get pending")
    print("  POST /api/v1/admin/verify/<type> - Verify step")
    print("  POST /api/v1/admin/approve - Approve application")
    print("  POST /api/v1/admin/reject - Reject application")
    print("=" * 80)
    app.run(host='0.0.0.0', port=5002, debug=True)
