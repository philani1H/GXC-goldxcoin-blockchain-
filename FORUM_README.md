# GXC Blockchain Forum

A full-featured discussion forum with real-time chat for the GXC blockchain community.

## Features

### 🎯 Core Features
- **Discussion Forums**: Multiple categories for different topics
- **Real-Time Chat**: Live chat with SocketIO
- **User Authentication**: Registration, login, profiles
- **Post Management**: Create topics, reply, edit posts
- **Voting System**: Upvote/downvote posts
- **Moderation**: Admin and moderator roles
- **Notifications**: User notifications system
- **Search**: Search topics and posts
- **Reputation System**: User reputation based on activity

### 📁 Categories
1. **General Discussion** - General blockchain discussions
2. **Mining** - Mining guides and support
3. **Trading & Markets** - Price discussions
4. **Development** - Technical discussions
5. **Stock Contracts** - Tokenized stocks discussions
6. **Governance** - Governance proposals
7. **Support** - User help and support
8. **Announcements** - Official announcements

## Installation

### Requirements
```bash
pip install flask flask-socketio werkzeug
```

### Start Forum
```bash
# Windows
start_forum.bat

# Linux/macOS
chmod +x start_forum.sh
./start_forum.sh

# Or directly
cd web
python forum.py
```

The forum will be available at: **http://localhost:3001/forum**

## Default Admin Account

- **Username**: `admin`
- **Password**: `admin123`
- **⚠️ Change this password in production!**

## Database

The forum uses SQLite database (`gxc_forum.db`) with the following tables:
- `forum_users` - User accounts
- `forum_categories` - Forum categories
- `forum_topics` - Discussion topics
- `forum_posts` - Posts/replies
- `forum_reactions` - Upvotes/downvotes
- `forum_chat` - Chat messages
- `forum_notifications` - User notifications

## API Endpoints

- `GET /forum` - Forum homepage
- `GET /forum/category/<slug>` - Category page
- `GET /forum/topic/<id>` - Topic/thread page
- `GET /forum/new-topic` - Create new topic (login required)
- `POST /forum/topic/<id>/reply` - Reply to topic (login required)
- `GET /forum/chat` - Real-time chat page
- `GET /forum/register` - User registration
- `GET /forum/login` - User login
- `GET /forum/logout` - User logout
- `GET /forum/api/categories` - API: Get categories
- `GET /forum/api/topics` - API: Get topics

## SocketIO Events

### Client → Server
- `join_room` - Join a chat room
- `leave_room` - Leave a chat room
- `chat_message` - Send chat message
- `get_chat_history` - Get chat history

### Server → Client
- `connected` - Connection confirmation
- `chat_message` - New chat message
- `chat_history` - Chat history response
- `user_joined` - User joined notification
- `user_left` - User left notification

## Integration

The forum is integrated with the GXC Explorer:
- Forum link in explorer navigation
- Redirect from `/forum` route in explorer
- Shared styling and theme

## Security Features

- Password hashing with Werkzeug
- Session management
- User authentication required for posting
- Admin/moderator roles
- User banning capability
- Content moderation tools

## Future Enhancements

- [ ] Email notifications
- [ ] Rich text editor
- [ ] Image uploads
- [ ] Private messages
- [ ] User profiles with avatars
- [ ] Advanced search
- [ ] Topic subscriptions
- [ ] Mobile app

