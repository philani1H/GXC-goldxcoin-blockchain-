#!/usr/bin/env python3
"""
GXC Forum Real-Time Server
Dedicated real-time server for forum chat and notifications with database persistence
"""

import os
import sys
import sqlite3
import json
import threading
import time
from datetime import datetime
from flask import Flask, request
from flask_socketio import SocketIO, emit, join_room, leave_room
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Database configuration - use same database as forum
# Use /tmp for Vercel (writable), otherwise use local path
IS_VERCEL = os.environ.get('VERCEL', '0') == '1' or 'vercel' in os.environ.get('VERCEL_URL', '').lower()
DATABASE_PATH = os.environ.get('FORUM_DATABASE_PATH', '/tmp/gxc_forum.db' if IS_VERCEL or os.path.exists('/tmp') else 'gxc_forum.db')
REALTIME_PORT = int(os.environ.get('FORUM_REALTIME_PORT', 3002))

# Create Flask app for SocketIO
app = Flask(__name__)
app.config['SECRET_KEY'] = os.environ.get('FORUM_SECRET_KEY', 'forum_realtime_secret_key_change_in_production')
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='threading', logger=True, engineio_logger=True)

# Database connection pool
db_lock = threading.Lock()

def get_db():
    """Get database connection"""
    conn = sqlite3.connect(DATABASE_PATH, check_same_thread=False)
    conn.row_factory = sqlite3.Row
    return conn

def init_database():
    """Ensure database tables exist"""
    conn = get_db()
    cursor = conn.cursor()
    
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
    
    # Online users tracking
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS forum_online_users (
            session_id TEXT PRIMARY KEY,
            user_id INTEGER,
            username TEXT,
            room TEXT,
            last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES forum_users (user_id)
        )
    ''')
    
    # Real-time events log
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS forum_realtime_events (
            event_id INTEGER PRIMARY KEY AUTOINCREMENT,
            event_type TEXT NOT NULL,
            user_id INTEGER,
            data TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    
    conn.commit()
    conn.close()
    logger.info("Database initialized successfully")

# Initialize database on startup
init_database()

# Store active connections
active_connections = {}
user_rooms = {}

@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    logger.info(f"Client connected: {request.sid}")
    active_connections[request.sid] = {
        'connected_at': datetime.now().isoformat(),
        'user_id': None,
        'username': None
    }
    emit('connected', {'status': 'connected', 'server_time': datetime.now().isoformat()})

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    logger.info(f"Client disconnected: {request.sid}")
    
    # Remove from active connections
    if request.sid in active_connections:
        user_info = active_connections[request.sid]
        user_id = user_info.get('user_id')
        
        # Remove from online users table
        if user_id:
            conn = get_db()
            cursor = conn.cursor()
            cursor.execute('DELETE FROM forum_online_users WHERE session_id = ?', (request.sid,))
            conn.commit()
            conn.close()
            
            # Notify others in rooms
            for room in user_rooms.get(request.sid, []):
                leave_room(room)
                socketio.emit('user_left', {
                    'user_id': user_id,
                    'username': user_info.get('username'),
                    'room': room
                }, room=room)
        
        del active_connections[request.sid]
        if request.sid in user_rooms:
            del user_rooms[request.sid]

@socketio.on('authenticate')
def handle_authenticate(data):
    """Authenticate user and associate with session"""
    try:
        user_id = data.get('user_id')
        username = data.get('username')
        
        if user_id and username:
            active_connections[request.sid]['user_id'] = user_id
            active_connections[request.sid]['username'] = username
            
            # Add to online users table
            conn = get_db()
            cursor = conn.cursor()
            cursor.execute('''
                INSERT OR REPLACE INTO forum_online_users 
                (session_id, user_id, username, last_seen)
                VALUES (?, ?, ?, ?)
            ''', (request.sid, user_id, username, datetime.now().isoformat()))
            conn.commit()
            conn.close()
            
            emit('authenticated', {'status': 'success', 'user_id': user_id, 'username': username})
            logger.info(f"User authenticated: {username} (ID: {user_id})")
        else:
            emit('authenticated', {'status': 'error', 'message': 'Missing user_id or username'})
    except Exception as e:
        logger.error(f"Authentication error: {e}")
        emit('authenticated', {'status': 'error', 'message': str(e)})

@socketio.on('join_room')
def handle_join_room(data):
    """Join a chat room"""
    try:
        room = data.get('room', 'general')
        join_room(room)
        
        if request.sid not in user_rooms:
            user_rooms[request.sid] = []
        if room not in user_rooms[request.sid]:
            user_rooms[request.sid].append(room)
        
        # Update online users table
        user_info = active_connections.get(request.sid, {})
        if user_info.get('user_id'):
            conn = get_db()
            cursor = conn.cursor()
            cursor.execute('''
                UPDATE forum_online_users 
                SET room = ?, last_seen = ?
                WHERE session_id = ?
            ''', (room, datetime.now().isoformat(), request.sid))
            conn.commit()
            conn.close()
        
        # Get recent messages from database
        conn = get_db()
        cursor = conn.cursor()
        cursor.execute('''
            SELECT message_id, user_id, username, message, created_at
            FROM forum_chat
            WHERE room = ?
            ORDER BY created_at DESC
            LIMIT 50
        ''', (room,))
        
        recent_messages = []
        for row in cursor.fetchall():
            recent_messages.append({
                'message_id': row['message_id'],
                'user_id': row['user_id'],
                'username': row['username'],
                'message': row['message'],
                'created_at': row['created_at']
            })
        conn.close()
        
        # Send recent messages to user
        emit('room_history', {'room': room, 'messages': list(reversed(recent_messages))})
        
        # Notify others
        socketio.emit('user_joined', {
            'user_id': user_info.get('user_id'),
            'username': user_info.get('username'),
            'room': room
        }, room=room)
        
        logger.info(f"User {user_info.get('username', 'unknown')} joined room: {room}")
    except Exception as e:
        logger.error(f"Join room error: {e}")
        emit('error', {'message': str(e)})

@socketio.on('leave_room')
def handle_leave_room(data):
    """Leave a chat room"""
    try:
        room = data.get('room', 'general')
        leave_room(room)
        
        if request.sid in user_rooms and room in user_rooms[request.sid]:
            user_rooms[request.sid].remove(room)
        
        user_info = active_connections.get(request.sid, {})
        socketio.emit('user_left', {
            'user_id': user_info.get('user_id'),
            'username': user_info.get('username'),
            'room': room
        }, room=room)
        
        logger.info(f"User {user_info.get('username', 'unknown')} left room: {room}")
    except Exception as e:
        logger.error(f"Leave room error: {e}")

@socketio.on('chat_message')
def handle_chat_message(data):
    """Handle chat message and persist to database"""
    try:
        user_info = active_connections.get(request.sid, {})
        user_id = user_info.get('user_id')
        username = user_info.get('username')
        message = data.get('message', '').strip()
        room = data.get('room', 'general')
        
        if not user_id or not username:
            emit('error', {'message': 'Not authenticated'})
            return
        
        if not message:
            emit('error', {'message': 'Message cannot be empty'})
            return
        
        # Save to database
        conn = get_db()
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO forum_chat (user_id, username, message, room)
            VALUES (?, ?, ?, ?)
        ''', (user_id, username, message, room))
        message_id = cursor.lastrowid
        conn.commit()
        conn.close()
        
        # Log event
        conn = get_db()
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO forum_realtime_events (event_type, user_id, data)
            VALUES (?, ?, ?)
        ''', ('chat_message', user_id, json.dumps({'message_id': message_id, 'room': room})))
        conn.commit()
        conn.close()
        
        # Broadcast to room
        message_data = {
            'message_id': message_id,
            'user_id': user_id,
            'username': username,
            'message': message,
            'room': room,
            'created_at': datetime.now().isoformat()
        }
        
        socketio.emit('chat_message', message_data, room=room)
        logger.info(f"Message from {username} in {room}: {message[:50]}")
        
    except Exception as e:
        logger.error(f"Chat message error: {e}")
        emit('error', {'message': str(e)})

@socketio.on('get_online_users')
def handle_get_online_users(data):
    """Get list of online users in a room"""
    try:
        room = data.get('room', 'general')
        
        conn = get_db()
        cursor = conn.cursor()
        cursor.execute('''
            SELECT DISTINCT user_id, username
            FROM forum_online_users
            WHERE room = ? OR room IS NULL
            ORDER BY username
        ''', (room,))
        
        online_users = []
        for row in cursor.fetchall():
            online_users.append({
                'user_id': row['user_id'],
                'username': row['username']
            })
        conn.close()
        
        emit('online_users', {'room': room, 'users': online_users})
    except Exception as e:
        logger.error(f"Get online users error: {e}")
        emit('error', {'message': str(e)})

@socketio.on('typing')
def handle_typing(data):
    """Handle typing indicator"""
    try:
        user_info = active_connections.get(request.sid, {})
        room = data.get('room', 'general')
        is_typing = data.get('typing', False)
        
        socketio.emit('user_typing', {
            'user_id': user_info.get('user_id'),
            'username': user_info.get('username'),
            'typing': is_typing,
            'room': room
        }, room=room, include_self=False)
    except Exception as e:
        logger.error(f"Typing indicator error: {e}")

def background_task():
    """Background task to clean up old data and update online status"""
    while True:
        try:
            time.sleep(60)  # Run every minute
            
            # Update last_seen for active connections
            conn = get_db()
            cursor = conn.cursor()
            for sid, info in active_connections.items():
                if info.get('user_id'):
                    cursor.execute('''
                        UPDATE forum_online_users
                        SET last_seen = ?
                        WHERE session_id = ?
                    ''', (datetime.now().isoformat(), sid))
            
            # Remove stale online users (not seen in 5 minutes)
            cursor.execute('''
                DELETE FROM forum_online_users
                WHERE last_seen < datetime('now', '-5 minutes')
            ''')
            
            conn.commit()
            conn.close()
            
            logger.debug("Background task completed: cleaned up stale connections")
        except Exception as e:
            logger.error(f"Background task error: {e}")

# Start background task
background_thread = threading.Thread(target=background_task, daemon=True)
background_thread.start()

# For Vercel deployment, use polling transport
if os.environ.get('VERCEL', '0') == '1':
    socketio.init_app(app, cors_allowed_origins="*", async_mode='threading', 
                     transports=['polling', 'websocket'], logger=False, engineio_logger=False)

if __name__ == '__main__':
    logger.info(f"Starting GXC Forum Real-Time Server on port {REALTIME_PORT}")
    logger.info(f"Database: {DATABASE_PATH}")
    socketio.run(app, host='0.0.0.0', port=REALTIME_PORT, debug=False, allow_unsafe_werkzeug=True)

