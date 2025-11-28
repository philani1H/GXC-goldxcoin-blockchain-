# 🚀 Vercel Deployment Guide

## Prerequisites

1. **Vercel Account**: Sign up at [vercel.com](https://vercel.com)
2. **Vercel CLI**: Install globally
   ```bash
   npm install -g vercel
   ```

## Deployment Steps

### 1. Install Dependencies

```bash
# Install Python dependencies
pip install -r requirements.txt
```

### 2. Configure Environment Variables

Create a `.env` file or set in Vercel dashboard:

```env
SECRET_KEY=your_secret_key_here
BLOCKCHAIN_NODE_URL=http://localhost:8545
DATABASE_PATH=/tmp/gxc_explorer.db
```

**Note**: SQLite databases on Vercel should use `/tmp` directory as it's writable. For production, consider using a cloud database like PostgreSQL.

### 3. Deploy to Vercel

#### Option A: Using Vercel CLI

```bash
# Login to Vercel
vercel login

# Deploy
vercel

# For production deployment
vercel --prod
```

#### Option B: Using GitHub Integration

1. Push your code to GitHub
2. Go to [vercel.com/new](https://vercel.com/new)
3. Import your GitHub repository
4. Vercel will automatically detect the configuration

### 4. Post-Deployment Configuration

#### Database Considerations

**Important**: Vercel's filesystem is read-only except `/tmp`. For production:

1. **Option 1**: Use `/tmp` for SQLite (data will be lost on each deployment)
   ```python
   DATABASE_PATH = '/tmp/gxc_explorer.db'
   ```

2. **Option 2**: Use external database (Recommended)
   - PostgreSQL (Vercel Postgres)
   - MongoDB Atlas
   - Supabase
   - PlanetScale

#### SocketIO Limitations

SocketIO requires persistent connections, which may not work perfectly in serverless environments. Consider:
- Using polling instead of WebSocket
- Using a separate service for real-time features (e.g., Pusher, Ably)
- Using Vercel's Edge Functions for better WebSocket support

### 5. Update URLs

After deployment, update hardcoded URLs in:
- `web/templates/forum/base.html` - Update localhost URLs
- `web/templates/explorer_index.html` - Update localhost URLs
- `web/blockchain_explorer.py` - Update RPC endpoint if needed

### 6. Custom Domain (Optional)

1. Go to your project settings in Vercel
2. Add your custom domain
3. Update DNS records as instructed

## Project Structure

```
.
├── api/
│   ├── explorer.py      # Serverless wrapper for explorer
│   └── forum.py         # Serverless wrapper for forum
├── web/
│   ├── blockchain_explorer.py
│   ├── forum.py
│   ├── static/          # Static files
│   └── templates/       # HTML templates
├── vercel.json          # Vercel configuration
├── requirements.txt     # Python dependencies
└── .vercelignore       # Files to exclude from deployment
```

## Troubleshooting

### Database Issues

If you see database errors:
- Ensure database path uses `/tmp` directory
- Consider migrating to a cloud database
- Check file permissions

### SocketIO Issues

If real-time features don't work:
- Check Vercel function logs
- Consider using polling transport
- Use external WebSocket service

### Import Errors

If you see import errors:
- Ensure all dependencies are in `requirements.txt`
- Check Python version (should be 3.9+)
- Verify file paths in serverless wrappers

## Production Recommendations

1. **Database**: Use Vercel Postgres or external database
2. **Caching**: Use Vercel Edge Caching
3. **CDN**: Static files are automatically served via CDN
4. **Monitoring**: Use Vercel Analytics
5. **Environment Variables**: Store secrets in Vercel dashboard

## Support

For issues:
- Check Vercel logs: `vercel logs`
- Review Vercel documentation: [vercel.com/docs](https://vercel.com/docs)
- Check function logs in Vercel dashboard
