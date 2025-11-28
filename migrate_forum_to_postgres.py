#!/usr/bin/env python3
"""
Migrate GXC Forum data from SQLite to PostgreSQL
Run this before deploying to Vercel with PostgreSQL
"""

import sqlite3
import os
import sys

try:
    import psycopg2
    from psycopg2.extras import execute_values
except ImportError:
    print("Error: psycopg2 not installed")
    print("Install with: pip install psycopg2-binary")
    sys.exit(1)

# Configuration
SQLITE_DB = os.environ.get('SQLITE_DB', 'gxc_forum.db')
POSTGRES_URL = os.environ.get('DATABASE_URL', '')

if not POSTGRES_URL:
    print("Error: DATABASE_URL environment variable not set")
    print("Usage: export DATABASE_URL='postgresql://user:pass@host:5432/database'")
    sys.exit(1)

def migrate():
    """Migrate all forum data from SQLite to PostgreSQL"""
    
    print("🔄 Starting migration...")
    print(f"Source: {SQLITE_DB}")
    print(f"Destination: PostgreSQL")
    print()
    
    # Check if SQLite database exists
    if not os.path.exists(SQLITE_DB):
        print(f"Error: SQLite database '{SQLITE_DB}' not found")
        sys.exit(1)
    
    # Connect to databases
    try:
        sqlite_conn = sqlite3.connect(SQLITE_DB)
        sqlite_conn.row_factory = sqlite3.Row
        print("✅ Connected to SQLite")
    except Exception as e:
        print(f"❌ Error connecting to SQLite: {e}")
        sys.exit(1)
    
    try:
        pg_conn = psycopg2.connect(POSTGRES_URL)
        print("✅ Connected to PostgreSQL")
    except Exception as e:
        print(f"❌ Error connecting to PostgreSQL: {e}")
        sys.exit(1)
    
    sqlite_cur = sqlite_conn.cursor()
    pg_cur = pg_conn.cursor()
    
    try:
        # Migrate users
        print("\n📊 Migrating users...")
        sqlite_cur.execute("SELECT * FROM forum_users")
        users = sqlite_cur.fetchall()
        
        if users:
            for user in users:
                try:
                    pg_cur.execute("""
                        INSERT INTO forum_users 
                        (id, username, email, password_hash, display_name, avatar_url, bio, 
                         reputation, is_moderator, is_admin, created_at, last_seen)
                        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
                        ON CONFLICT (username) DO NOTHING
                    """, (
                        user['id'], user['username'], user['email'], user['password_hash'],
                        user['display_name'], user['avatar_url'], user['bio'],
                        user['reputation'], user['is_moderator'], user['is_admin'],
                        user['created_at'], user['last_seen']
                    ))
                except Exception as e:
                    print(f"  Warning: Could not migrate user {user['username']}: {e}")
            
            pg_conn.commit()
            print(f"  ✅ Migrated {len(users)} users")
        else:
            print("  ℹ️  No users to migrate")
        
        # Migrate categories
        print("\n📊 Migrating categories...")
        sqlite_cur.execute("SELECT * FROM forum_categories")
        categories = sqlite_cur.fetchall()
        
        if categories:
            for cat in categories:
                try:
                    pg_cur.execute("""
                        INSERT INTO forum_categories 
                        (id, name, description, slug, icon, color, display_order, created_at)
                        VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
                        ON CONFLICT (slug) DO NOTHING
                    """, (
                        cat['id'], cat['name'], cat['description'], cat['slug'],
                        cat['icon'], cat['color'], cat['display_order'], cat['created_at']
                    ))
                except Exception as e:
                    print(f"  Warning: Could not migrate category {cat['name']}: {e}")
            
            pg_conn.commit()
            print(f"  ✅ Migrated {len(categories)} categories")
        else:
            print("  ℹ️  No categories to migrate")
        
        # Migrate topics
        print("\n📊 Migrating topics...")
        sqlite_cur.execute("SELECT * FROM forum_topics")
        topics = sqlite_cur.fetchall()
        
        if topics:
            for topic in topics:
                try:
                    pg_cur.execute("""
                        INSERT INTO forum_topics 
                        (id, category_id, user_id, title, content, is_pinned, is_locked, 
                         view_count, reply_count, created_at, updated_at, last_reply_at)
                        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
                    """, (
                        topic['id'], topic['category_id'], topic['user_id'], topic['title'],
                        topic['content'], topic['is_pinned'], topic['is_locked'],
                        topic['view_count'], topic['reply_count'], topic['created_at'],
                        topic['updated_at'], topic['last_reply_at']
                    ))
                except Exception as e:
                    print(f"  Warning: Could not migrate topic {topic['title']}: {e}")
            
            pg_conn.commit()
            print(f"  ✅ Migrated {len(topics)} topics")
        else:
            print("  ℹ️  No topics to migrate")
        
        # Migrate posts
        print("\n📊 Migrating posts...")
        sqlite_cur.execute("SELECT * FROM forum_posts")
        posts = sqlite_cur.fetchall()
        
        if posts:
            for post in posts:
                try:
                    pg_cur.execute("""
                        INSERT INTO forum_posts 
                        (id, topic_id, user_id, content, is_edited, edited_at, created_at)
                        VALUES (%s, %s, %s, %s, %s, %s, %s)
                    """, (
                        post['id'], post['topic_id'], post['user_id'], post['content'],
                        post['is_edited'], post['edited_at'], post['created_at']
                    ))
                except Exception as e:
                    print(f"  Warning: Could not migrate post: {e}")
            
            pg_conn.commit()
            print(f"  ✅ Migrated {len(posts)} posts")
        else:
            print("  ℹ️  No posts to migrate")
        
        print("\n" + "="*50)
        print("✅ Migration completed successfully!")
        print("="*50)
        print()
        print("Summary:")
        print(f"  Users:      {len(users) if users else 0}")
        print(f"  Categories: {len(categories) if categories else 0}")
        print(f"  Topics:     {len(topics) if topics else 0}")
        print(f"  Posts:      {len(posts) if posts else 0}")
        print()
        print("Next steps:")
        print("  1. Set DATABASE_URL in Vercel environment variables")
        print("  2. Set USE_POSTGRES=true in Vercel environment variables")
        print("  3. Deploy to Vercel: vercel --prod")
        
    except Exception as e:
        print(f"\n❌ Migration failed: {e}")
        pg_conn.rollback()
        sys.exit(1)
    
    finally:
        sqlite_conn.close()
        pg_conn.close()

if __name__ == '__main__':
    print("="*50)
    print("GXC Forum Migration Tool")
    print("SQLite → PostgreSQL")
    print("="*50)
    print()
    
    # Confirm before proceeding
    response = input("This will migrate forum data to PostgreSQL. Continue? (yes/no): ")
    if response.lower() not in ['yes', 'y']:
        print("Migration cancelled")
        sys.exit(0)
    
    migrate()
