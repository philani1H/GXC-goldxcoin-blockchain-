# 🔧 Vercel Forum Persistence Fix

## Problem

When deployed on Vercel, the forum data doesn't persist because:
- Vercel uses serverless functions (ephemeral)
- SQLite database stored in `/tmp` gets cleared
- No persistent file storage available

## Solution

Use an external database (PostgreSQL) for persistent storage.

---

## 🎯 Quick Fix (3 Options)

### Option 1: Vercel Postgres (Recommended)

**Steps:**

1. **Add Vercel Postgres to your project:**
   ```bash
   # In Vercel dashboard
   # Go to your project → Storage → Create Database → Postgres
   ```

2. **Vercel automatically sets `DATABASE_URL` environment variable**

3. **Add psycopg2 to requirements:**
   ```bash
   echo "psycopg2-binary" >> requirements.txt
   ```

4. **Redeploy:**
   ```bash
   vercel --prod
   ```

**Cost:** Free tier: 256 MB storage, 60 hours compute/month

---

### Option 2: Supabase (Free, Easy)

**Steps:**

1. **Create Supabase account:**
   - Go to https://supabase.com
   - Create new project
   - Get connection string

2. **Add to Vercel environment variables:**
   ```
   DATABASE_URL=postgresql://user:pass@host:5432/database
   USE_POSTGRES=true
   ```

3. **Add psycopg2 to requirements:**
   ```bash
   echo "psycopg2-binary" >> requirements.txt
   ```

4. **Redeploy:**
   ```bash
   vercel --prod
   ```

**Cost:** Free tier: 500 MB database, unlimited API requests

---

### Option 3: Neon (Serverless Postgres)

**Steps:**

1. **Create Neon account:**
   - Go to https://neon.tech
   - Create new project
   - Get connection string

2. **Add to Vercel environment variables:**
   ```
   DATABASE_URL=postgresql://user:pass@host/database
   USE_POSTGRES=true
   ```

3. **Add psycopg2 to requirements:**
   ```bash
   echo "psycopg2-binary" >> requirements.txt
   ```

4. **Redeploy:**
   ```bash
   vercel --prod
   ```

**Cost:** Free tier: 3 GB storage, 100 hours compute/month

---

## 📋 Detailed Setup Guide

### Step 1: Choose Database Provider

| Provider | Free Tier | Best For |
|----------|-----------|----------|
| **Vercel Postgres** | 256 MB | Vercel projects |
| **Supabase** | 500 MB | Easy setup |
| **Neon** | 3 GB | Generous limits |
| **Railway** | 1 GB | Full features |

### Step 2: Get Database URL

**Vercel Postgres:**
```bash
# In Vercel dashboard
# Storage → Create Database → Postgres
# Copy connection string
```

**Supabase:**
```bash
# Project Settings → Database → Connection string
# Use "Connection pooling" string
postgresql://postgres:[password]@[host]:6543/postgres
```

**Neon:**
```bash
# Dashboard → Connection Details
# Copy connection string
postgresql://[user]:[password]@[host]/[database]
```

### Step 3: Add Environment Variables

**In Vercel Dashboard:**
```
Settings → Environment Variables → Add

Name: DATABASE_URL
Value: postgresql://user:pass@host:5432/database

Name: USE_POSTGRES
Value: true
```

**Or via CLI:**
```bash
vercel env add DATABASE_URL
# Paste your connection string

vercel env add USE_POSTGRES
# Enter: true
```

### Step 4: Update Requirements

Add to `requirements.txt`:
```
psycopg2-binary>=2.9.0
```

### Step 5: Deploy

```bash
vercel --prod
```

---

## 🔄 Migration Script

If you have existing data in SQLite, migrate it:

```python
#!/usr/bin/env python3
"""
Migrate forum data from SQLite to PostgreSQL
"""

import sqlite3
import psycopg2
import os

# Source (SQLite)
sqlite_db = 'gxc_forum.db'

# Destination (PostgreSQL)
postgres_url = os.environ.get('DATABASE_URL')

def migrate():
    # Connect to both databases
    sqlite_conn = sqlite3.connect(sqlite_db)
    sqlite_conn.row_factory = sqlite3.Row
    pg_conn = psycopg2.connect(postgres_url)
    
    sqlite_cur = sqlite_conn.cursor()
    pg_cur = pg_conn.cursor()
    
    # Migrate users
    print("Migrating users...")
    sqlite_cur.execute("SELECT * FROM forum_users")
    users = sqlite_cur.fetchall()
    for user in users:
        pg_cur.execute("""
            INSERT INTO forum_users 
            (username, email, password_hash, display_name, avatar_url, bio, reputation, is_moderator, is_admin, created_at, last_seen)
            VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
            ON CONFLICT (username) DO NOTHING
        """, tuple(user))
    
    # Migrate categories
    print("Migrating categories...")
    sqlite_cur.execute("SELECT * FROM forum_categories")
    categories = sqlite_cur.fetchall()
    for cat in categories:
        pg_cur.execute("""
            INSERT INTO forum_categories 
            (name, description, slug, icon, color, display_order, created_at)
            VALUES (%s, %s, %s, %s, %s, %s, %s)
            ON CONFLICT (slug) DO NOTHING
        """, tuple(cat))
    
    # Migrate topics
    print("Migrating topics...")
    sqlite_cur.execute("SELECT * FROM forum_topics")
    topics = sqlite_cur.fetchall()
    for topic in topics:
        pg_cur.execute("""
            INSERT INTO forum_topics 
            (category_id, user_id, title, content, is_pinned, is_locked, view_count, reply_count, created_at, updated_at, last_reply_at)
            VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """, tuple(topic))
    
    # Migrate posts
    print("Migrating posts...")
    sqlite_cur.execute("SELECT * FROM forum_posts")
    posts = sqlite_cur.fetchall()
    for post in posts:
        pg_cur.execute("""
            INSERT INTO forum_posts 
            (topic_id, user_id, content, is_edited, edited_at, created_at)
            VALUES (%s, %s, %s, %s, %s, %s)
        """, tuple(post))
    
    # Commit and close
    pg_conn.commit()
    sqlite_conn.close()
    pg_conn.close()
    
    print("Migration complete!")

if __name__ == '__main__':
    migrate()
```

**Run migration:**
```bash
export DATABASE_URL="your-postgres-url"
python3 migrate_forum.py
```

---

## 🧪 Testing

### Test Locally with PostgreSQL

```bash
# Install PostgreSQL locally
# Ubuntu/Debian:
sudo apt install postgresql

# macOS:
brew install postgresql

# Start PostgreSQL
sudo service postgresql start  # Linux
brew services start postgresql  # macOS

# Create database
createdb gxc_forum

# Set environment variable
export DATABASE_URL="postgresql://localhost/gxc_forum"
export USE_POSTGRES=true

# Run forum
python3 web/forum.py
```

### Test on Vercel

```bash
# Deploy
vercel --prod

# Check logs
vercel logs

# Test forum
curl https://your-app.vercel.app/forum
```

---

## 🔍 Troubleshooting

### "psycopg2 not found"

**Solution:**
```bash
# Add to requirements.txt
echo "psycopg2-binary>=2.9.0" >> requirements.txt

# Redeploy
vercel --prod
```

### "Connection refused"

**Solution:**
- Check DATABASE_URL is correct
- Verify database is accessible
- Check firewall rules
- Ensure USE_POSTGRES=true is set

### "Table doesn't exist"

**Solution:**
```bash
# Database adapter auto-creates tables
# But you can manually initialize:

# Connect to your database
psql $DATABASE_URL

# Run schema from forum_db.py
# Or let the app create tables on first run
```

### "Data not showing"

**Solution:**
1. Check environment variables are set
2. Verify DATABASE_URL is correct
3. Check Vercel logs: `vercel logs`
4. Ensure tables were created
5. Try redeploying: `vercel --prod`

---

## 📊 Comparison

| Storage | Persistence | Cost | Setup | Best For |
|---------|-------------|------|-------|----------|
| **SQLite (Local)** | ❌ No (Vercel) | Free | Easy | Local dev |
| **Vercel Postgres** | ✅ Yes | Free tier | Easy | Vercel apps |
| **Supabase** | ✅ Yes | Free tier | Easy | Any platform |
| **Neon** | ✅ Yes | Free tier | Easy | Serverless |
| **Railway** | ✅ Yes | Free tier | Easy | Full stack |

---

## 🎯 Recommended Solution

**For Vercel deployment:**

1. **Use Vercel Postgres** (easiest integration)
2. **Or Supabase** (more generous free tier)

**Setup time:** 5 minutes

**Steps:**
```bash
# 1. Create database (Vercel dashboard or Supabase)
# 2. Add DATABASE_URL to Vercel env vars
# 3. Add psycopg2-binary to requirements.txt
# 4. Deploy: vercel --prod
```

**Done!** Your forum data will now persist.

---

## 📝 Environment Variables Checklist

```bash
# Required for persistence
DATABASE_URL=postgresql://user:pass@host:5432/database
USE_POSTGRES=true

# Optional
VERCEL=1  # Auto-set by Vercel
EXPLORER_URL=https://your-explorer.vercel.app
FORUM_URL=https://your-forum.vercel.app
```

---

## 🚀 Quick Start

**Fastest way to fix persistence:**

```bash
# 1. Go to Supabase.com
# 2. Create project (free)
# 3. Get connection string
# 4. Add to Vercel:
vercel env add DATABASE_URL
# Paste connection string

vercel env add USE_POSTGRES
# Enter: true

# 5. Add to requirements.txt:
echo "psycopg2-binary" >> requirements.txt

# 6. Deploy:
vercel --prod
```

**Done in 5 minutes!** ✅

---

## 📚 Additional Resources

- **Vercel Postgres:** https://vercel.com/docs/storage/vercel-postgres
- **Supabase:** https://supabase.com/docs
- **Neon:** https://neon.tech/docs
- **psycopg2:** https://www.psycopg.org/docs/

---

**Your forum will now persist data across deployments!** 🎉
