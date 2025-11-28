#!/usr/bin/env python3
"""
GXC Forum Database Adapter
Supports both SQLite (local) and PostgreSQL (Vercel/production)
"""

import os
import sqlite3
from contextlib import contextmanager

# Detect environment
IS_VERCEL = os.environ.get('VERCEL', '0') == '1' or 'vercel' in os.environ.get('VERCEL_URL', '').lower()
USE_POSTGRES = os.environ.get('USE_POSTGRES', 'false').lower() == 'true' or IS_VERCEL

# Database configuration
if USE_POSTGRES:
    # PostgreSQL for Vercel/production
    DATABASE_URL = os.environ.get('DATABASE_URL', '')
    if DATABASE_URL:
        try:
            import psycopg2
            from psycopg2.extras import RealDictCursor
            HAS_POSTGRES = True
        except ImportError:
            print("Warning: psycopg2 not installed, falling back to SQLite")
            HAS_POSTGRES = False
            USE_POSTGRES = False
    else:
        USE_POSTGRES = False
        HAS_POSTGRES = False
else:
    HAS_POSTGRES = False

# SQLite fallback
if not USE_POSTGRES:
    DATABASE_PATH = os.environ.get('DATABASE_PATH', '/tmp/gxc_forum.db' if os.path.exists('/tmp') else 'gxc_forum.db')

class DatabaseAdapter:
    """Database adapter that works with both SQLite and PostgreSQL"""
    
    def __init__(self):
        self.use_postgres = USE_POSTGRES and HAS_POSTGRES
        
    @contextmanager
    def get_connection(self):
        """Get database connection (context manager)"""
        if self.use_postgres:
            conn = psycopg2.connect(DATABASE_URL)
            try:
                yield conn
                conn.commit()
            except Exception as e:
                conn.rollback()
                raise e
            finally:
                conn.close()
        else:
            conn = sqlite3.connect(DATABASE_PATH)
            conn.row_factory = sqlite3.Row
            try:
                yield conn
                conn.commit()
            except Exception as e:
                conn.rollback()
                raise e
            finally:
                conn.close()
    
    def execute(self, query, params=None):
        """Execute a query and return results"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            if params:
                cursor.execute(query, params)
            else:
                cursor.execute(query)
            
            # For SELECT queries, fetch results
            if query.strip().upper().startswith('SELECT'):
                if self.use_postgres:
                    return cursor.fetchall()
                else:
                    return [dict(row) for row in cursor.fetchall()]
            
            return cursor.lastrowid if not self.use_postgres else cursor.rowcount
    
    def executemany(self, query, params_list):
        """Execute a query multiple times with different parameters"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.executemany(query, params_list)
            return cursor.rowcount
    
    def convert_query(self, query):
        """Convert SQLite query to PostgreSQL if needed"""
        if not self.use_postgres:
            return query
        
        # Convert SQLite syntax to PostgreSQL
        query = query.replace('AUTOINCREMENT', 'SERIAL')
        query = query.replace('INTEGER PRIMARY KEY AUTOINCREMENT', 'SERIAL PRIMARY KEY')
        query = query.replace('DATETIME DEFAULT CURRENT_TIMESTAMP', 'TIMESTAMP DEFAULT CURRENT_TIMESTAMP')
        query = query.replace('?', '%s')  # Parameter placeholder
        
        return query
    
    def init_schema(self):
        """Initialize database schema"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            
            # Users table
            if self.use_postgres:
                cursor.execute('''
                    CREATE TABLE IF NOT EXISTS forum_users (
                        id SERIAL PRIMARY KEY,
                        username VARCHAR(50) UNIQUE NOT NULL,
                        email VARCHAR(100) UNIQUE NOT NULL,
                        password_hash VARCHAR(255) NOT NULL,
                        display_name VARCHAR(100),
                        avatar_url TEXT,
                        bio TEXT,
                        reputation INTEGER DEFAULT 0,
                        is_moderator BOOLEAN DEFAULT FALSE,
                        is_admin BOOLEAN DEFAULT FALSE,
                        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                        last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                    )
                ''')
            else:
                cursor.execute('''
                    CREATE TABLE IF NOT EXISTS forum_users (
                        id INTEGER PRIMARY KEY AUTOINCREMENT,
                        username TEXT UNIQUE NOT NULL,
                        email TEXT UNIQUE NOT NULL,
                        password_hash TEXT NOT NULL,
                        display_name TEXT,
                        avatar_url TEXT,
                        bio TEXT,
                        reputation INTEGER DEFAULT 0,
                        is_moderator INTEGER DEFAULT 0,
                        is_admin INTEGER DEFAULT 0,
                        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                        last_seen DATETIME DEFAULT CURRENT_TIMESTAMP
                    )
                ''')
            
            # Categories table
            if self.use_postgres:
                cursor.execute('''
                    CREATE TABLE IF NOT EXISTS forum_categories (
                        id SERIAL PRIMARY KEY,
                        name VARCHAR(100) NOT NULL,
                        description TEXT,
                        slug VARCHAR(100) UNIQUE NOT NULL,
                        icon VARCHAR(50),
                        color VARCHAR(20),
                        display_order INTEGER DEFAULT 0,
                        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                    )
                ''')
            else:
                cursor.execute('''
                    CREATE TABLE IF NOT EXISTS forum_categories (
                        id INTEGER PRIMARY KEY AUTOINCREMENT,
                        name TEXT NOT NULL,
                        description TEXT,
                        slug TEXT UNIQUE NOT NULL,
                        icon TEXT,
                        color TEXT,
                        display_order INTEGER DEFAULT 0,
                        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                    )
                ''')
            
            # Topics table
            if self.use_postgres:
                cursor.execute('''
                    CREATE TABLE IF NOT EXISTS forum_topics (
                        id SERIAL PRIMARY KEY,
                        category_id INTEGER REFERENCES forum_categories(id),
                        user_id INTEGER REFERENCES forum_users(id),
                        title VARCHAR(200) NOT NULL,
                        content TEXT NOT NULL,
                        is_pinned BOOLEAN DEFAULT FALSE,
                        is_locked BOOLEAN DEFAULT FALSE,
                        view_count INTEGER DEFAULT 0,
                        reply_count INTEGER DEFAULT 0,
                        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                        last_reply_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                    )
                ''')
            else:
                cursor.execute('''
                    CREATE TABLE IF NOT EXISTS forum_topics (
                        id INTEGER PRIMARY KEY AUTOINCREMENT,
                        category_id INTEGER,
                        user_id INTEGER,
                        title TEXT NOT NULL,
                        content TEXT NOT NULL,
                        is_pinned INTEGER DEFAULT 0,
                        is_locked INTEGER DEFAULT 0,
                        view_count INTEGER DEFAULT 0,
                        reply_count INTEGER DEFAULT 0,
                        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                        last_reply_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                        FOREIGN KEY (category_id) REFERENCES forum_categories(id),
                        FOREIGN KEY (user_id) REFERENCES forum_users(id)
                    )
                ''')
            
            # Posts table
            if self.use_postgres:
                cursor.execute('''
                    CREATE TABLE IF NOT EXISTS forum_posts (
                        id SERIAL PRIMARY KEY,
                        topic_id INTEGER REFERENCES forum_topics(id),
                        user_id INTEGER REFERENCES forum_users(id),
                        content TEXT NOT NULL,
                        is_edited BOOLEAN DEFAULT FALSE,
                        edited_at TIMESTAMP,
                        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                    )
                ''')
            else:
                cursor.execute('''
                    CREATE TABLE IF NOT EXISTS forum_posts (
                        id INTEGER PRIMARY KEY AUTOINCREMENT,
                        topic_id INTEGER,
                        user_id INTEGER,
                        content TEXT NOT NULL,
                        is_edited INTEGER DEFAULT 0,
                        edited_at DATETIME,
                        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                        FOREIGN KEY (topic_id) REFERENCES forum_topics(id),
                        FOREIGN KEY (user_id) REFERENCES forum_users(id)
                    )
                ''')
            
            conn.commit()

# Global database adapter instance
db = DatabaseAdapter()

def get_db():
    """Get database adapter instance"""
    return db
