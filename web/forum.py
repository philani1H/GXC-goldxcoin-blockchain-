#!/usr/bin/env python3
"""
GXC Blockchain Forum
Full-featured discussion forum with real-time chat
"""

import os
import json
import sqlite3
import hashlib
import secrets
from datetime import datetime, timedelta
from flask import Flask, render_template, request, jsonify, redirect, url_for, session, flash
from flask_socketio import SocketIO, emit, join_room, leave_room
from werkzeug.security import generate_password_hash, check_password_hash
from werkzeug.utils import secure_filename
from functools import wraps
import threading
import time
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

app = Flask(__name__, template_folder='templates', static_folder='static')
app.config['SECRET_KEY'] = secrets.token_hex(32)
# Use /tmp for Vercel (writable), otherwise use local path
app.config['DATABASE'] = os.environ.get('DATABASE_PATH', '/tmp/gxc_forum.db' if os.path.exists('/tmp') else 'gxc_forum.db')
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024  # 16MB max file size
app.config['UPLOAD_FOLDER'] = 'static/forum/uploads'

# Detect environment (Vercel or local)
IS_VERCEL = os.environ.get('VERCEL', '0') == '1' or 'vercel' in os.environ.get('VERCEL_URL', '').lower()
EXPLORER_URL = os.environ.get('EXPLORER_URL', 'https://gxc-blockchain.vercel.app' if IS_VERCEL else 'http://localhost:3000')
MARKET_MAKER_URL = os.environ.get('MARKET_MAKER_URL', 'https://gxc-blockchain-market-maker.vercel.app' if IS_VERCEL else 'http://localhost:4000')
FORUM_URL = os.environ.get('FORUM_URL', 'https://gxc-blockchain-forum.vercel.app' if IS_VERCEL else 'http://localhost:3001')
FORUM_REALTIME_URL = os.environ.get('FORUM_REALTIME_URL', 'http://localhost:3002')

# SocketIO for backward compatibility (will connect to external real-time server)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='threading')

# Import database adapter
try:
    from forum_db import get_db
    db_adapter = get_db()
    USE_DB_ADAPTER = True
    print("✅ Using database adapter (supports PostgreSQL)")
except ImportError:
    USE_DB_ADAPTER = False
    print("ℹ️  Using legacy SQLite mode")

# Database initialization
def init_database():
    """Initialize forum database"""
    if USE_DB_ADAPTER:
        # Use new database adapter (supports PostgreSQL)
        try:
            db_adapter.init_schema()
            print("✅ Database schema initialized")
            return
        except Exception as e:
            print(f"Warning: Database adapter initialization failed: {e}")
            print("Falling back to SQLite...")
    
    # Fallback to legacy SQLite
    conn = sqlite3.connect(app.config['DATABASE'])
    cursor = conn.cursor()
    
    # Users table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS forum_users (
            user_id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            avatar_url TEXT,
            signature TEXT,
            reputation INTEGER DEFAULT 0,
            posts_count INTEGER DEFAULT 0,
            is_moderator BOOLEAN DEFAULT 0,
            is_admin BOOLEAN DEFAULT 0,
            is_banned BOOLEAN DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            gxc_address TEXT
        )
    ''')
    
    # Categories table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS forum_categories (
            category_id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT,
            slug TEXT UNIQUE NOT NULL,
            icon TEXT,
            color TEXT DEFAULT '#667eea',
            post_count INTEGER DEFAULT 0,
            topic_count INTEGER DEFAULT 0,
            last_post_id INTEGER,
            last_post_time TIMESTAMP,
            display_order INTEGER DEFAULT 0,
            is_active BOOLEAN DEFAULT 1
        )
    ''')
    
    # Topics/Threads table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS forum_topics (
            topic_id INTEGER PRIMARY KEY AUTOINCREMENT,
            category_id INTEGER NOT NULL,
            author_id INTEGER NOT NULL,
            title TEXT NOT NULL,
            slug TEXT UNIQUE NOT NULL,
            content TEXT NOT NULL,
            is_pinned BOOLEAN DEFAULT 0,
            is_locked BOOLEAN DEFAULT 0,
            is_announcement BOOLEAN DEFAULT 0,
            view_count INTEGER DEFAULT 0,
            reply_count INTEGER DEFAULT 0,
            last_reply_id INTEGER,
            last_reply_time TIMESTAMP,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (category_id) REFERENCES forum_categories (category_id),
            FOREIGN KEY (author_id) REFERENCES forum_users (user_id)
        )
    ''')
    
    # Posts/Replies table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS forum_posts (
            post_id INTEGER PRIMARY KEY AUTOINCREMENT,
            topic_id INTEGER NOT NULL,
            author_id INTEGER NOT NULL,
            content TEXT NOT NULL,
            is_edited BOOLEAN DEFAULT 0,
            edited_at TIMESTAMP,
            upvotes INTEGER DEFAULT 0,
            downvotes INTEGER DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (topic_id) REFERENCES forum_topics (topic_id),
            FOREIGN KEY (author_id) REFERENCES forum_users (user_id)
        )
    ''')
    
    # Reactions table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS forum_reactions (
            reaction_id INTEGER PRIMARY KEY AUTOINCREMENT,
            post_id INTEGER NOT NULL,
            user_id INTEGER NOT NULL,
            reaction_type TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (post_id) REFERENCES forum_posts (post_id),
            FOREIGN KEY (user_id) REFERENCES forum_users (user_id),
            UNIQUE(post_id, user_id, reaction_type)
        )
    ''')
    
    # Chat messages table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS forum_chat (
            message_id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            username TEXT NOT NULL,
            message TEXT NOT NULL,
            room TEXT DEFAULT 'general',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES forum_users (user_id)
        )
    ''')
    
    # Live support chat table (for AI and admin responses)
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS forum_support_chat (
            chat_id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            username TEXT NOT NULL,
            message TEXT NOT NULL,
            response TEXT,
            response_type TEXT DEFAULT 'ai',  -- 'ai' or 'admin'
            responder_id INTEGER,  -- admin user_id if admin responded
            responder_username TEXT,
            is_resolved BOOLEAN DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            responded_at TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES forum_users (user_id),
            FOREIGN KEY (responder_id) REFERENCES forum_users (user_id)
        )
    ''')
    
    # Notifications table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS forum_notifications (
            notification_id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            type TEXT NOT NULL,
            title TEXT NOT NULL,
            message TEXT,
            link TEXT,
            is_read BOOLEAN DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES forum_users (user_id)
        )
    ''')
    
    # Insert default categories
    default_categories = [
        ('General Discussion', 'General discussions about GXC blockchain', 'general', 'fas fa-comments', '#667eea', 0),
        ('Mining', 'Mining discussions, guides, and support', 'mining', 'fas fa-hammer', '#10b981', 1),
        ('Trading & Markets', 'Price discussions and trading strategies', 'trading', 'fas fa-chart-line', '#f59e0b', 2),
        ('Development', 'Technical discussions and development updates', 'development', 'fas fa-code', '#3b82f6', 3),
        ('Stock Contracts', 'Discussions about tokenized stocks', 'stocks', 'fas fa-building', '#8b5cf6', 4),
        ('Governance', 'Governance proposals and voting discussions', 'governance', 'fas fa-vote-yea', '#ef4444', 5),
        ('Support', 'Help and support for users', 'support', 'fas fa-life-ring', '#06b6d4', 6),
        ('Announcements', 'Official announcements and news', 'announcements', 'fas fa-bullhorn', '#f97316', 7)
    ]
    
    for name, desc, slug, icon, color, order in default_categories:
        cursor.execute('''
            INSERT OR IGNORE INTO forum_categories (name, description, slug, icon, color, display_order)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (name, desc, slug, icon, color, order))
    
    # Create admin user if not exists, or update existing admin to CHade
    admin_password = generate_password_hash('admin123')  # Change in production!
    cursor.execute('''
        INSERT OR IGNORE INTO forum_users (username, email, password_hash, is_admin, is_moderator)
        VALUES (?, ?, ?, ?, ?)
    ''', ('CHade', 'admin@gxc.network', admin_password, 1, 1))
    
    # Update existing admin user to CHade if username is 'admin'
    cursor.execute('''
        UPDATE forum_users 
        SET username = 'CHade' 
        WHERE username = 'admin' AND is_admin = 1
    ''')
    
    conn.commit()
    conn.close()

# Initialize database on startup
init_database()

# Authentication decorator
def login_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if 'user_id' not in session:
            return redirect(url_for('forum_login'))
        return f(*args, **kwargs)
    return decorated_function

# Database helper
def get_db():
    conn = sqlite3.connect(app.config['DATABASE'])
    conn.row_factory = sqlite3.Row
    return conn

def allowed_file(filename):
    """Check if file extension is allowed"""
    ALLOWED_EXTENSIONS = {'png', 'jpg', 'jpeg', 'gif', 'webp', 'svg', 'pdf', 'txt', 'md'}
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

# AI Chat Helper (using Puter.js on client side)
def get_ai_context():
    """Get context for AI responses"""
    return """You are a helpful AI assistant for the GXC (GoldXCoin) blockchain forum. 
    GXC is a blockchain platform with features like:
    - Gold-backed tokens (GXC-G)
    - Stock contract tokenization
    - Mining support (SHA256, Ethash, GXHash)
    - Governance and staking
    - Cross-chain bridge
    
    Help users with questions about:
    - Blockchain technology
    - Mining setup and configuration
    - Trading and markets
    - Development and technical issues
    - General blockchain concepts
    
    Be friendly, professional, and provide accurate information. If you don't know something, 
    suggest they check the documentation or ask in the forum."""

# Routes
@app.route('/forum')
def forum_index():
    """Forum homepage"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Get categories with stats
    cursor.execute('''
        SELECT c.*, 
               COUNT(DISTINCT t.topic_id) as topic_count,
               COUNT(DISTINCT p.post_id) as post_count,
               MAX(t.created_at) as last_activity
        FROM forum_categories c
        LEFT JOIN forum_topics t ON c.category_id = t.category_id
        LEFT JOIN forum_posts p ON t.topic_id = p.topic_id
        WHERE c.is_active = 1
        GROUP BY c.category_id
        ORDER BY c.display_order ASC
    ''')
    
    categories = [dict(row) for row in cursor.fetchall()]
    
    # Get recent topics
    cursor.execute('''
        SELECT t.*, t.author_id, u.username, u.avatar_url, c.name as category_name, c.slug as category_slug
        FROM forum_topics t
        JOIN forum_users u ON t.author_id = u.user_id
        JOIN forum_categories c ON t.category_id = c.category_id
        ORDER BY t.created_at DESC
        LIMIT 10
    ''')
    
    recent_topics = [dict(row) for row in cursor.fetchall()]
    
    # Get active users (last 24 hours)
    cursor.execute('''
        SELECT COUNT(DISTINCT user_id) as active_users
        FROM forum_users
        WHERE last_seen >= datetime('now', '-24 hours')
    ''')
    
    active_users = cursor.fetchone()['active_users']
    
    conn.close()
    
    return render_template('forum/index.html', 
                         categories=categories,
                         recent_topics=recent_topics,
                         active_users=active_users,
                         session=session)

@app.route('/forum/category/<slug>')
def forum_category(slug):
    """Category page"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Get category
    cursor.execute('SELECT * FROM forum_categories WHERE slug = ?', (slug,))
    category = dict(cursor.fetchone() or {})
    
    if not category:
        flash('Category not found', 'error')
        return redirect(url_for('forum_index'))
    
    # Get topics in category
    page = request.args.get('page', 1, type=int)
    per_page = 20
    offset = (page - 1) * per_page
    
    cursor.execute('''
        SELECT t.*, u.username, u.avatar_url,
               (SELECT COUNT(*) FROM forum_posts WHERE topic_id = t.topic_id) as reply_count,
               (SELECT username FROM forum_users WHERE user_id = 
                   (SELECT author_id FROM forum_posts WHERE topic_id = t.topic_id 
                    ORDER BY created_at DESC LIMIT 1)
               ) as last_reply_author
        FROM forum_topics t
        JOIN forum_users u ON t.author_id = u.user_id
        WHERE t.category_id = ?
        ORDER BY t.is_pinned DESC, t.last_reply_time DESC, t.created_at DESC
        LIMIT ? OFFSET ?
    ''', (category['category_id'], per_page, offset))
    
    topics = [dict(row) for row in cursor.fetchall()]
    
    # Get total count
    cursor.execute('SELECT COUNT(*) as total FROM forum_topics WHERE category_id = ?', 
                   (category['category_id'],))
    total = cursor.fetchone()['total']
    
    conn.close()
    
    return render_template('forum/category.html',
                         category=category,
                         topics=topics,
                         page=page,
                         total_pages=(total + per_page - 1) // per_page,
                         session=session)

@app.route('/forum/topic/<int:topic_id>')
@app.route('/forum/topic/<int:topic_id>/<slug>')
def forum_topic(topic_id, slug=None):
    """Topic/Thread page"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Get topic
    cursor.execute('''
        SELECT t.*, u.username, u.avatar_url, u.reputation, c.name as category_name, c.slug as category_slug
        FROM forum_topics t
        JOIN forum_users u ON t.author_id = u.user_id
        JOIN forum_categories c ON t.category_id = c.category_id
        WHERE t.topic_id = ?
    ''', (topic_id,))
    
    topic = dict(cursor.fetchone() or {})
    
    if not topic:
        flash('Topic not found', 'error')
        return redirect(url_for('forum_index'))
    
    # Increment view count
    cursor.execute('UPDATE forum_topics SET view_count = view_count + 1 WHERE topic_id = ?', 
                   (topic_id,))
    
    # Get posts
    page = request.args.get('page', 1, type=int)
    per_page = 20
    offset = (page - 1) * per_page
    
    cursor.execute('''
        SELECT p.*, p.author_id, u.username, u.avatar_url, u.reputation, u.signature,
               (SELECT COUNT(*) FROM forum_reactions WHERE post_id = p.post_id AND reaction_type = 'upvote') as upvotes,
               (SELECT COUNT(*) FROM forum_reactions WHERE post_id = p.post_id AND reaction_type = 'downvote') as downvotes
        FROM forum_posts p
        JOIN forum_users u ON p.author_id = u.user_id
        WHERE p.topic_id = ?
        ORDER BY p.created_at ASC
        LIMIT ? OFFSET ?
    ''', (topic_id, per_page, offset))
    
    posts = [dict(row) for row in cursor.fetchall()]
    
    # Get total count
    cursor.execute('SELECT COUNT(*) as total FROM forum_posts WHERE topic_id = ?', (topic_id,))
    total = cursor.fetchone()['total']
    
    # Get topic author_id for permission checks
    cursor.execute('SELECT author_id FROM forum_topics WHERE topic_id = ?', (topic_id,))
    topic_author = cursor.fetchone()
    topic['author_id'] = topic_author['author_id'] if topic_author else None
    
    conn.commit()
    conn.close()
    
    return render_template('forum/topic.html',
                         topic=topic,
                         posts=posts,
                         page=page,
                         total_pages=(total + per_page - 1) // per_page,
                         session=session)

@app.route('/forum/new-topic', methods=['GET', 'POST'])
@login_required
def forum_new_topic():
    """Create new topic"""
    if request.method == 'POST':
        category_id = request.form.get('category_id')
        title = request.form.get('title')
        content = request.form.get('content')
        
        if not all([category_id, title, content]):
            flash('All fields are required', 'error')
            return redirect(url_for('forum_new_topic'))
        
        conn = get_db()
        cursor = conn.cursor()
        
        # Generate slug
        slug = title.lower().replace(' ', '-')[:100]
        slug = ''.join(c for c in slug if c.isalnum() or c == '-')
        
        # Ensure unique slug
        base_slug = slug
        counter = 1
        while cursor.execute('SELECT topic_id FROM forum_topics WHERE slug = ?', (slug,)).fetchone():
            slug = f"{base_slug}-{counter}"
            counter += 1
        
        # Create topic
        cursor.execute('''
            INSERT INTO forum_topics (category_id, author_id, title, slug, content)
            VALUES (?, ?, ?, ?, ?)
        ''', (category_id, session['user_id'], title, slug, content))
        
        topic_id = cursor.lastrowid
        
        # Create first post
        cursor.execute('''
            INSERT INTO forum_posts (topic_id, author_id, content)
            VALUES (?, ?, ?)
        ''', (topic_id, session['user_id'], content))
        
        # Update category stats
        cursor.execute('''
            UPDATE forum_categories 
            SET topic_count = topic_count + 1,
                last_post_time = CURRENT_TIMESTAMP
            WHERE category_id = ?
        ''', (category_id,))
        
        # Update user post count
        cursor.execute('''
            UPDATE forum_users 
            SET posts_count = posts_count + 1
            WHERE user_id = ?
        ''', (session['user_id'],))
        
        conn.commit()
        conn.close()
        
        flash('Topic created successfully!', 'success')
        return redirect(url_for('forum_topic', topic_id=topic_id))
    
    # GET request - show form
    conn = get_db()
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM forum_categories WHERE is_active = 1 ORDER BY display_order')
    categories = [dict(row) for row in cursor.fetchall()]
    conn.close()
    
    return render_template('forum/new_topic.html', categories=categories, session=session)

@app.route('/forum/topic/<int:topic_id>/reply', methods=['POST'])
@login_required
def forum_reply(topic_id):
    """Reply to topic"""
    content = request.form.get('content')
    
    if not content:
        flash('Content is required', 'error')
        return redirect(url_for('forum_topic', topic_id=topic_id))
    
    conn = get_db()
    cursor = conn.cursor()
    
    # Create post
    cursor.execute('''
        INSERT INTO forum_posts (topic_id, author_id, content)
        VALUES (?, ?, ?)
    ''', (topic_id, session['user_id'], content))
    
    post_id = cursor.lastrowid
    
    # Update topic
    cursor.execute('''
        UPDATE forum_topics 
        SET reply_count = reply_count + 1,
            last_reply_id = ?,
            last_reply_time = CURRENT_TIMESTAMP,
            updated_at = CURRENT_TIMESTAMP
        WHERE topic_id = ?
    ''', (post_id, topic_id))
    
    # Update category
    cursor.execute('''
        SELECT category_id FROM forum_topics WHERE topic_id = ?
    ''', (topic_id,))
    category_id = cursor.fetchone()['category_id']
    
    cursor.execute('''
        UPDATE forum_categories 
        SET post_count = post_count + 1,
            last_post_time = CURRENT_TIMESTAMP
        WHERE category_id = ?
    ''', (category_id,))
    
    # Update user
    cursor.execute('''
        UPDATE forum_users 
        SET posts_count = posts_count + 1
        WHERE user_id = ?
    ''', (session['user_id'],))
    
    conn.commit()
    conn.close()
    
    flash('Reply posted!', 'success')
    return redirect(url_for('forum_topic', topic_id=topic_id))

@app.route('/forum/topic/<int:topic_id>/delete', methods=['POST'])
@login_required
def forum_delete_topic(topic_id):
    """Delete a topic"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Get topic
    cursor.execute('SELECT author_id FROM forum_topics WHERE topic_id = ?', (topic_id,))
    topic = cursor.fetchone()
    
    if not topic:
        flash('Topic not found', 'error')
        conn.close()
        return redirect(url_for('forum_index'))
    
    # Check permissions (author or admin/moderator)
    if topic['author_id'] != session['user_id'] and not session.get('is_admin') and not session.get('is_moderator'):
        flash('You do not have permission to delete this topic', 'error')
        conn.close()
        return redirect(url_for('forum_topic', topic_id=topic_id))
    
    # Get category_id before deletion
    cursor.execute('SELECT category_id FROM forum_topics WHERE topic_id = ?', (topic_id,))
    category_row = cursor.fetchone()
    category_id = category_row['category_id'] if category_row else None
    
    # Delete all posts in topic
    cursor.execute('DELETE FROM forum_posts WHERE topic_id = ?', (topic_id,))
    
    # Delete topic
    cursor.execute('DELETE FROM forum_topics WHERE topic_id = ?', (topic_id,))
    
    # Update category stats
    if category_id:
        cursor.execute('''
            UPDATE forum_categories 
            SET topic_count = (SELECT COUNT(*) FROM forum_topics WHERE category_id = ?),
                post_count = (SELECT COUNT(*) FROM forum_posts 
                             WHERE topic_id IN (SELECT topic_id FROM forum_topics WHERE category_id = ?))
            WHERE category_id = ?
        ''', (category_id, category_id, category_id))
    
    conn.commit()
    conn.close()
    
    flash('Topic deleted successfully', 'success')
    return redirect(url_for('forum_index'))

@app.route('/forum/post/<int:post_id>/delete', methods=['POST'])
@login_required
def forum_delete_post(post_id):
    """Delete a post/reply"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Get post
    cursor.execute('SELECT author_id, topic_id FROM forum_posts WHERE post_id = ?', (post_id,))
    post = cursor.fetchone()
    
    if not post:
        flash('Post not found', 'error')
        conn.close()
        return redirect(url_for('forum_index'))
    
    # Check permissions (author or admin/moderator)
    if post['author_id'] != session['user_id'] and not session.get('is_admin') and not session.get('is_moderator'):
        flash('You do not have permission to delete this post', 'error')
        conn.close()
        return redirect(url_for('forum_topic', topic_id=post['topic_id']))
    
    topic_id = post['topic_id']
    
    # Check if this is the first post (topic content)
    cursor.execute('SELECT post_id FROM forum_posts WHERE topic_id = ? ORDER BY created_at ASC LIMIT 1', (topic_id,))
    first_post = cursor.fetchone()
    
    if first_post and first_post['post_id'] == post_id:
        # Cannot delete the first post (topic content), must delete topic instead
        flash('Cannot delete the first post. Delete the topic instead.', 'error')
        conn.close()
        return redirect(url_for('forum_topic', topic_id=topic_id))
    
    # Delete post
    cursor.execute('DELETE FROM forum_posts WHERE post_id = ?', (post_id,))
    
    # Update topic reply count
    cursor.execute('''
        UPDATE forum_topics 
        SET reply_count = (SELECT COUNT(*) - 1 FROM forum_posts WHERE topic_id = ?),
            last_reply_time = (SELECT MAX(created_at) FROM forum_posts WHERE topic_id = ?)
        WHERE topic_id = ?
    ''', (topic_id, topic_id, topic_id))
    
    # Update category stats
    cursor.execute('SELECT category_id FROM forum_topics WHERE topic_id = ?', (topic_id,))
    category_row = cursor.fetchone()
    if category_row:
        category_id = category_row['category_id']
        cursor.execute('''
            UPDATE forum_categories 
            SET post_count = post_count - 1
            WHERE category_id = ?
        ''', (category_id,))
    
    # Update user post count
    cursor.execute('''
        UPDATE forum_users 
        SET posts_count = posts_count - 1
        WHERE user_id = ?
    ''', (post['author_id'],))
    
    conn.commit()
    conn.close()
    
    flash('Post deleted successfully', 'success')
    return redirect(url_for('forum_topic', topic_id=topic_id))

@app.route('/forum/register', methods=['GET', 'POST'])
def forum_register():
    """User registration"""
    if request.method == 'POST':
        username = request.form.get('username')
        email = request.form.get('email')
        password = request.form.get('password')
        
        if not all([username, email, password]):
            flash('All fields are required', 'error')
            return redirect(url_for('forum_register'))
        
        conn = get_db()
        cursor = conn.cursor()
        
        # Check if username exists
        if cursor.execute('SELECT user_id FROM forum_users WHERE username = ?', (username,)).fetchone():
            flash('Username already exists', 'error')
            conn.close()
            return redirect(url_for('forum_register'))
        
        # Check if email exists
        if cursor.execute('SELECT user_id FROM forum_users WHERE email = ?', (email,)).fetchone():
            flash('Email already registered', 'error')
            conn.close()
            return redirect(url_for('forum_register'))
        
        # Create user
        password_hash = generate_password_hash(password)
        cursor.execute('''
            INSERT INTO forum_users (username, email, password_hash)
            VALUES (?, ?, ?)
        ''', (username, email, password_hash))
        
        user_id = cursor.lastrowid
        conn.commit()
        conn.close()
        
        session['user_id'] = user_id
        session['username'] = username
        flash('Registration successful!', 'success')
        return redirect(url_for('forum_index'))
    
    return render_template('forum/register.html', session=session)

@app.route('/forum/login', methods=['GET', 'POST'])
def forum_login():
    """User login"""
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')
        
        conn = get_db()
        cursor = conn.cursor()
        
        cursor.execute('SELECT * FROM forum_users WHERE username = ?', (username,))
        user = cursor.fetchone()
        
        if user and check_password_hash(user['password_hash'], password):
            if user['is_banned']:
                flash('Your account has been banned', 'error')
                conn.close()
                return redirect(url_for('forum_login'))
            
            session['user_id'] = user['user_id']
            session['username'] = user['username']
            session['is_moderator'] = user['is_moderator']
            session['is_admin'] = user['is_admin']
            
            # Update last seen
            cursor.execute('UPDATE forum_users SET last_seen = CURRENT_TIMESTAMP WHERE user_id = ?', 
                          (user['user_id'],))
            conn.commit()
            
            flash('Login successful!', 'success')
            return redirect(request.args.get('next') or url_for('forum_index'))
        else:
            flash('Invalid username or password', 'error')
        
        conn.close()
    
    return render_template('forum/login.html', session=session)

@app.route('/forum/logout')
def forum_logout():
    """User logout"""
    session.clear()
    flash('Logged out successfully', 'success')
    return redirect(url_for('forum_index'))

@app.route('/forum/chat')
def forum_chat():
    """Real-time chat page"""
    return render_template('forum/chat.html', session=session)

@app.route('/forum/profile/<int:user_id>')
def forum_profile(user_id):
    """User profile page"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Get user info
    cursor.execute('''
        SELECT user_id, username, email, avatar_url, signature, reputation,
               posts_count, created_at, last_seen, gxc_address
        FROM forum_users
        WHERE user_id = ?
    ''', (user_id,))
    
    user = cursor.fetchone()
    if not user:
        flash('User not found', 'error')
        return redirect(url_for('forum_index'))
    
    user_dict = dict(user)
    
    # Get user's recent topics
    cursor.execute('''
        SELECT t.*, t.author_id, c.name as category_name, c.slug as category_slug
        FROM forum_topics t
        JOIN forum_categories c ON t.category_id = c.category_id
        WHERE t.author_id = ?
        ORDER BY t.created_at DESC
        LIMIT 10
    ''', (user_id,))
    
    user_topics = [dict(row) for row in cursor.fetchall()]
    
    # Get user's recent posts
    cursor.execute('''
        SELECT p.*, t.title as topic_title, t.topic_id, t.slug as topic_slug
        FROM forum_posts p
        JOIN forum_topics t ON p.topic_id = t.topic_id
        WHERE p.author_id = ?
        ORDER BY p.created_at DESC
        LIMIT 10
    ''', (user_id,))
    
    user_posts = [dict(row) for row in cursor.fetchall()]
    
    conn.close()
    
    return render_template('forum/profile.html',
                         user=user_dict,
                         user_topics=user_topics,
                         user_posts=user_posts,
                         session=session)

@app.route('/forum/settings', methods=['GET', 'POST'])
@login_required
def forum_settings():
    """User settings page"""
    if request.method == 'POST':
        conn = get_db()
        cursor = conn.cursor()
        
        # Update user settings
        signature = request.form.get('signature', '')
        gxc_address = request.form.get('gxc_address', '')
        email = request.form.get('email', '')
        
        # Handle avatar upload
        if 'avatar' in request.files:
            file = request.files['avatar']
            if file and file.filename:
                if allowed_file(file.filename):
                    filename = secure_filename(file.filename)
                    upload_dir = os.path.join(os.path.dirname(__file__), 'static', 'forum', 'uploads')
                    os.makedirs(upload_dir, exist_ok=True)
                    import uuid
                    file_ext = filename.rsplit('.', 1)[1].lower()
                    unique_filename = f"{uuid.uuid4().hex}.{file_ext}"
                    filepath = os.path.join(upload_dir, unique_filename)
                    file.save(filepath)
                    avatar_url = f"/static/forum/uploads/{unique_filename}"
                    cursor.execute('UPDATE forum_users SET avatar_url = ? WHERE user_id = ?', 
                                 (avatar_url, session['user_id']))
        
        # Update email if changed
        if email:
            cursor.execute('SELECT user_id FROM forum_users WHERE email = ? AND user_id != ?', 
                          (email, session['user_id']))
            if cursor.fetchone():
                flash('Email already in use', 'error')
                conn.close()
                return redirect(url_for('forum_settings'))
            cursor.execute('UPDATE forum_users SET email = ? WHERE user_id = ?', 
                         (email, session['user_id']))
        
        cursor.execute('''
            UPDATE forum_users
            SET signature = ?, gxc_address = ?
            WHERE user_id = ?
        ''', (signature, gxc_address, session['user_id']))
        
        conn.commit()
        conn.close()
        
        flash('Settings updated successfully!', 'success')
        return redirect(url_for('forum_settings'))
    
    # GET request
    conn = get_db()
    cursor = conn.cursor()
    cursor.execute('''
        SELECT username, email, avatar_url, signature, gxc_address
        FROM forum_users
        WHERE user_id = ?
    ''', (session['user_id'],))
    
    user = dict(cursor.fetchone())
    conn.close()
    
    return render_template('forum/settings.html', user=user, session=session)

@app.route('/forum/change-password', methods=['GET', 'POST'])
@login_required
def forum_change_password():
    """Change password page"""
    if request.method == 'POST':
        current_password = request.form.get('current_password')
        new_password = request.form.get('new_password')
        confirm_password = request.form.get('confirm_password')
        
        if not all([current_password, new_password, confirm_password]):
            flash('All fields are required', 'error')
            return redirect(url_for('forum_change_password'))
        
        if new_password != confirm_password:
            flash('New passwords do not match', 'error')
            return redirect(url_for('forum_change_password'))
        
        if len(new_password) < 6:
            flash('Password must be at least 6 characters', 'error')
            return redirect(url_for('forum_change_password'))
        
        conn = get_db()
        cursor = conn.cursor()
        
        # Verify current password
        cursor.execute('SELECT password_hash FROM forum_users WHERE user_id = ?', (session['user_id'],))
        user = cursor.fetchone()
        
        if not user or not check_password_hash(user['password_hash'], current_password):
            flash('Current password is incorrect', 'error')
            conn.close()
            return redirect(url_for('forum_change_password'))
        
        # Update password
        new_password_hash = generate_password_hash(new_password)
        cursor.execute('UPDATE forum_users SET password_hash = ? WHERE user_id = ?', 
                      (new_password_hash, session['user_id']))
        
        conn.commit()
        conn.close()
        
        flash('Password changed successfully!', 'success')
        return redirect(url_for('forum_settings'))
    
    return render_template('forum/change_password.html', session=session)

@app.route('/forum/live-chat')
@login_required
def forum_live_chat():
    """Live chat with AI support"""
    conn = get_db()
    cursor = conn.cursor()
    
    # Get chat history for this user
    cursor.execute('''
        SELECT chat_id, message, response, response_type, responder_username,
               created_at, responded_at, is_resolved
        FROM forum_support_chat
        WHERE user_id = ?
        ORDER BY created_at DESC
        LIMIT 50
    ''', (session['user_id'],))
    
    chat_history = [dict(row) for row in cursor.fetchall()]
    chat_history.reverse()  # Oldest first
    
    conn.close()
    
    return render_template('forum/live_chat.html', 
                         chat_history=chat_history,
                         session=session)

@app.route('/forum/admin/chat', methods=['GET', 'POST'])
@login_required
def forum_admin_chat():
    """Admin chat management"""
    if not session.get('is_admin'):
        flash('Access denied. Admin only.', 'error')
        return redirect(url_for('forum_index'))
    
    conn = get_db()
    cursor = conn.cursor()
    
    if request.method == 'POST':
        # Admin responding to a chat
        chat_id = request.form.get('chat_id')
        response = request.form.get('response', '').strip()
        
        if chat_id and response:
            cursor.execute('''
                UPDATE forum_support_chat
                SET response = ?,
                    response_type = 'admin',
                    responder_id = ?,
                    responder_username = ?,
                    responded_at = CURRENT_TIMESTAMP
                WHERE chat_id = ?
            ''', (response, session['user_id'], session['username'], chat_id))
            
            conn.commit()
            
            # Notify user via SocketIO
            cursor.execute('SELECT user_id FROM forum_support_chat WHERE chat_id = ?', (chat_id,))
            chat = cursor.fetchone()
            if chat:
                socketio.emit('admin_response', {
                    'chat_id': chat_id,
                    'response': response,
                    'responder': session['username']
                }, room=f"user_{chat['user_id']}")
            
            flash('Response sent successfully!', 'success')
    
    # Get all pending/unresolved chats
    cursor.execute('''
        SELECT c.*, u.username, u.email
        FROM forum_support_chat c
        JOIN forum_users u ON c.user_id = u.user_id
        WHERE c.is_resolved = 0 OR c.response IS NULL
        ORDER BY c.created_at DESC
        LIMIT 100
    ''')
    
    pending_chats = [dict(row) for row in cursor.fetchall()]
    
    # Get all recent chats
    cursor.execute('''
        SELECT c.*, u.username, u.email
        FROM forum_support_chat c
        JOIN forum_users u ON c.user_id = u.user_id
        ORDER BY c.created_at DESC
        LIMIT 200
    ''')
    
    all_chats = [dict(row) for row in cursor.fetchall()]
    
    conn.close()
    
    return render_template('forum/admin_chat.html',
                         pending_chats=pending_chats,
                         all_chats=all_chats,
                         session=session)

@app.route('/forum/api/chat/send', methods=['POST'])
@login_required
def api_chat_send():
    """API endpoint to send chat message and get AI response"""
    data = request.json
    message = data.get('message', '').strip()
    
    if not message:
        return jsonify({'error': 'Message is required'}), 400
    
    conn = get_db()
    cursor = conn.cursor()
    
    # Save user message
    cursor.execute('''
        INSERT INTO forum_support_chat (user_id, username, message)
        VALUES (?, ?, ?)
    ''', (session['user_id'], session['username'], message))
    
    chat_id = cursor.lastrowid
    conn.commit()
    
    # Return chat_id - AI response will be handled client-side with Puter.js
    conn.close()
    
    return jsonify({
        'chat_id': chat_id,
        'message': message,
        'timestamp': datetime.now().isoformat()
    })

@app.route('/forum/api/chat/ai-response', methods=['POST'])
@login_required
def api_chat_ai_response():
    """API endpoint to save AI response"""
    data = request.json
    chat_id = data.get('chat_id')
    response = data.get('response', '').strip()
    
    if not chat_id or not response:
        return jsonify({'error': 'Chat ID and response are required'}), 400
    
    conn = get_db()
    cursor = conn.cursor()
    
    cursor.execute('''
        UPDATE forum_support_chat
        SET response = ?,
            response_type = 'ai',
            responded_at = CURRENT_TIMESTAMP
        WHERE chat_id = ?
    ''', (response, chat_id))
    
    conn.commit()
    conn.close()
    
    return jsonify({'success': True})

@app.route('/forum/upload', methods=['POST'])
@login_required
def forum_upload():
    """Handle media uploads"""
    if 'file' not in request.files:
        return jsonify({'error': 'No file provided'}), 400
    
    file = request.files['file']
    if file.filename == '':
        return jsonify({'error': 'No file selected'}), 400
    
    # Allowed file types
    allowed_extensions = {'png', 'jpg', 'jpeg', 'gif', 'webp', 'svg', 'pdf', 'txt', 'md'}
    file_ext = file.filename.rsplit('.', 1)[1].lower() if '.' in file.filename else ''
    
    if file_ext not in allowed_extensions:
        return jsonify({'error': f'File type .{file_ext} not allowed'}), 400
    
    # Create uploads directory
    upload_dir = os.path.join(os.path.dirname(__file__), 'static', 'forum', 'uploads')
    os.makedirs(upload_dir, exist_ok=True)
    
    # Generate unique filename
    import uuid
    filename = f"{uuid.uuid4().hex}.{file_ext}"
    filepath = os.path.join(upload_dir, filename)
    
    # Save file
    file.save(filepath)
    
    # Return URL
    file_url = f"/static/forum/uploads/{filename}"
    
    return jsonify({
        'url': file_url,
        'filename': file.filename
    })

# SocketIO Events
@socketio.on('connect')
def handle_connect(auth):
    """Handle client connection"""
    emit('connected', {'message': 'Welcome to GXC Forum Chat! Please login to participate.'})

@socketio.on('join_room')
def handle_join_room(data):
    """Join a chat room"""
    room = data.get('room', 'general')
    username = data.get('username', 'Guest')
    join_room(room)
    if username != 'Guest':
        emit('user_joined', {
            'username': username,
            'room': room
        }, room=room)

@socketio.on('leave_room')
def handle_leave_room(data):
    """Leave a chat room"""
    room = data.get('room', 'general')
    username = data.get('username', 'Guest')
    leave_room(room)
    if username != 'Guest':
        emit('user_left', {
            'username': username,
            'room': room
        }, room=room)

@socketio.on('chat_message')
def handle_chat_message(data):
    """Handle chat message"""
    # Get user info from data (sent from client)
    user_id = data.get('user_id')
    username = data.get('username')
    
    if not user_id or not username:
        emit('error', {'message': 'Please login to send messages'})
        return
    
    message = data.get('message', '').strip()
    room = data.get('room', 'general')
    
    if not message:
        return
    
    # Save to database
    conn = get_db()
    cursor = conn.cursor()
    cursor.execute('''
        INSERT INTO forum_chat (user_id, username, message, room)
        VALUES (?, ?, ?, ?)
    ''', (user_id, username, message, room))
    conn.commit()
    conn.close()
    
    # Broadcast to room
    emit('chat_message', {
        'username': username,
        'message': message,
        'timestamp': datetime.now().isoformat(),
        'room': room
    }, room=room)

@socketio.on('get_chat_history')
def handle_get_history(data):
    """Get chat history"""
    room = data.get('room', 'general')
    limit = data.get('limit', 50)
    
    conn = get_db()
    cursor = conn.cursor()
    cursor.execute('''
        SELECT username, message, created_at
        FROM forum_chat
        WHERE room = ?
        ORDER BY created_at DESC
        LIMIT ?
    ''', (room, limit))
    
    messages = [dict(row) for row in cursor.fetchall()]
    messages.reverse()  # Oldest first
    conn.close()
    
    emit('chat_history', {'messages': messages})

# API Routes
@app.route('/forum/api/categories')
def api_categories():
    """API: Get categories"""
    conn = get_db()
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM forum_categories WHERE is_active = 1 ORDER BY display_order')
    categories = [dict(row) for row in cursor.fetchall()]
    conn.close()
    return jsonify(categories)

@app.route('/forum/api/topics')
def api_topics():
    """API: Get recent topics"""
    limit = request.args.get('limit', 10, type=int)
    category_id = request.args.get('category_id', type=int)
    
    conn = get_db()
    cursor = conn.cursor()
    
    if category_id:
        cursor.execute('''
            SELECT t.*, u.username, c.name as category_name
            FROM forum_topics t
            JOIN forum_users u ON t.author_id = u.user_id
            JOIN forum_categories c ON t.category_id = c.category_id
            WHERE t.category_id = ?
            ORDER BY t.created_at DESC
            LIMIT ?
        ''', (category_id, limit))
    else:
        cursor.execute('''
            SELECT t.*, u.username, c.name as category_name
            FROM forum_topics t
            JOIN forum_users u ON t.author_id = u.user_id
            JOIN forum_categories c ON t.category_id = c.category_id
            ORDER BY t.created_at DESC
            LIMIT ?
        ''', (limit,))
    
    topics = [dict(row) for row in cursor.fetchall()]
    conn.close()
    return jsonify(topics)

@app.route('/forum/api/posts')
def api_posts():
    """API: Get forum posts for AI context"""
    limit = request.args.get('limit', 50, type=int)
    topic_id = request.args.get('topic_id', type=int)
    search = request.args.get('search', '').strip()
    
    conn = get_db()
    cursor = conn.cursor()
    
    if topic_id:
        cursor.execute('''
            SELECT p.post_id, p.content, p.created_at, u.username, t.title as topic_title, c.name as category_name
            FROM forum_posts p
            JOIN forum_users u ON p.author_id = u.user_id
            JOIN forum_topics t ON p.topic_id = t.topic_id
            JOIN forum_categories c ON t.category_id = c.category_id
            WHERE p.topic_id = ?
            ORDER BY p.created_at DESC
            LIMIT ?
        ''', (topic_id, limit))
    elif search:
        cursor.execute('''
            SELECT p.post_id, p.content, p.created_at, u.username, t.title as topic_title, c.name as category_name
            FROM forum_posts p
            JOIN forum_users u ON p.author_id = u.user_id
            JOIN forum_topics t ON p.topic_id = t.topic_id
            JOIN forum_categories c ON t.category_id = c.category_id
            WHERE p.content LIKE ? OR t.title LIKE ?
            ORDER BY p.created_at DESC
            LIMIT ?
        ''', (f'%{search}%', f'%{search}%', limit))
    else:
        cursor.execute('''
            SELECT p.post_id, p.content, p.created_at, u.username, t.title as topic_title, c.name as category_name
            FROM forum_posts p
            JOIN forum_users u ON p.author_id = u.user_id
            JOIN forum_topics t ON p.topic_id = t.topic_id
            JOIN forum_categories c ON t.category_id = c.category_id
            ORDER BY p.created_at DESC
            LIMIT ?
        ''', (limit,))
    
    posts = [dict(row) for row in cursor.fetchall()]
    conn.close()
    return jsonify(posts)

@app.route('/forum/api/post/<int:post_id>/reaction', methods=['POST'])
@login_required
def api_post_reaction(post_id):
    """API: Add/remove reaction (upvote/downvote) to a post"""
    data = request.json
    reaction_type = data.get('type')  # 'upvote' or 'downvote'
    action = data.get('action', 'toggle')  # 'add', 'remove', or 'toggle'
    
    if reaction_type not in ['upvote', 'downvote']:
        return jsonify({'error': 'Invalid reaction type'}), 400
    
    conn = get_db()
    cursor = conn.cursor()
    
    # Check if user already reacted
    cursor.execute('''
        SELECT reaction_type FROM forum_reactions
        WHERE post_id = ? AND user_id = ?
    ''', (post_id, session['user_id']))
    
    existing = cursor.fetchone()
    
    if existing:
        existing_type = existing['reaction_type']
        
        if action == 'remove' or (action == 'toggle' and existing_type == reaction_type):
            # Remove reaction
            cursor.execute('''
                DELETE FROM forum_reactions
                WHERE post_id = ? AND user_id = ? AND reaction_type = ?
            ''', (post_id, session['user_id'], reaction_type))
            
            # Update post counts
            if reaction_type == 'upvote':
                cursor.execute('UPDATE forum_posts SET upvotes = upvotes - 1 WHERE post_id = ?', (post_id,))
            else:
                cursor.execute('UPDATE forum_posts SET downvotes = downvotes - 1 WHERE post_id = ?', (post_id,))
            
            conn.commit()
            
            # Get updated counts
            cursor.execute('''
                SELECT 
                    (SELECT COUNT(*) FROM forum_reactions WHERE post_id = ? AND reaction_type = 'upvote') as upvotes,
                    (SELECT COUNT(*) FROM forum_reactions WHERE post_id = ? AND reaction_type = 'downvote') as downvotes
            ''', (post_id, post_id))
            counts = cursor.fetchone()
            
            conn.close()
            return jsonify({
                'success': True,
                'action': 'removed',
                'upvotes': counts['upvotes'],
                'downvotes': counts['downvotes']
            })
        elif existing_type != reaction_type:
            # Change reaction type (e.g., from upvote to downvote)
            cursor.execute('''
                UPDATE forum_reactions
                SET reaction_type = ?
                WHERE post_id = ? AND user_id = ?
            ''', (reaction_type, post_id, session['user_id']))
            
            # Update post counts
            if existing_type == 'upvote':
                cursor.execute('UPDATE forum_posts SET upvotes = upvotes - 1, downvotes = downvotes + 1 WHERE post_id = ?', (post_id,))
            else:
                cursor.execute('UPDATE forum_posts SET upvotes = upvotes + 1, downvotes = downvotes - 1 WHERE post_id = ?', (post_id,))
            
            conn.commit()
            
            # Get updated counts
            cursor.execute('''
                SELECT 
                    (SELECT COUNT(*) FROM forum_reactions WHERE post_id = ? AND reaction_type = 'upvote') as upvotes,
                    (SELECT COUNT(*) FROM forum_reactions WHERE post_id = ? AND reaction_type = 'downvote') as downvotes
            ''', (post_id, post_id))
            counts = cursor.fetchone()
            
            conn.close()
            return jsonify({
                'success': True,
                'action': 'changed',
                'upvotes': counts['upvotes'],
                'downvotes': counts['downvotes']
            })
    else:
        # Add new reaction
        cursor.execute('''
            INSERT INTO forum_reactions (post_id, user_id, reaction_type)
            VALUES (?, ?, ?)
        ''', (post_id, session['user_id'], reaction_type))
        
        # Update post counts
        if reaction_type == 'upvote':
            cursor.execute('UPDATE forum_posts SET upvotes = upvotes + 1 WHERE post_id = ?', (post_id,))
        else:
            cursor.execute('UPDATE forum_posts SET downvotes = downvotes + 1 WHERE post_id = ?', (post_id,))
        
        conn.commit()
        
        # Get updated counts
        cursor.execute('''
            SELECT 
                (SELECT COUNT(*) FROM forum_reactions WHERE post_id = ? AND reaction_type = 'upvote') as upvotes,
                (SELECT COUNT(*) FROM forum_reactions WHERE post_id = ? AND reaction_type = 'downvote') as downvotes
        ''', (post_id, post_id))
        counts = cursor.fetchone()
        
        conn.close()
        return jsonify({
            'success': True,
            'action': 'added',
            'upvotes': counts['upvotes'],
            'downvotes': counts['downvotes']
        })

if __name__ == '__main__':
    # Auto-migrate if on Vercel
    try:
        from auto_migrate import auto_migrate
        auto_migrate()
    except Exception as e:
        print(f"Note: Auto-migration check: {e}")
    
    # Initialize database
    init_database()
    
    # Run forum web server (real-time server runs separately)
    app.run(host='0.0.0.0', port=3001, debug=False)

