# 🚀 Deploy GXC Services to Vercel

## Quick Deployment

### 1. Install Vercel CLI
```bash
npm install -g vercel
```

### 2. Login to Vercel
```bash
vercel login
```

### 3. Deploy
```bash
vercel --prod
```

## What Gets Deployed

### Services:
- ✅ **Blockchain Explorer** - `/` (Main explorer interface)
- ✅ **Forum** - `/forum` (Discussion forum)
- ✅ **Market Maker** - `/market-maker` (Stock market maker)
- ✅ **Forum Real-Time** - `/forum/realtime` (Real-time chat server)

### Files Structure:
```
api/
├── explorer.py          # Explorer serverless function
├── forum.py            # Forum serverless function
├── market_maker.py     # Market maker serverless function
└── forum_realtime.py  # Real-time server serverless function

web/
├── blockchain_explorer.py
├── forum.py
├── forum_realtime_server.py
└── templates/          # HTML templates

market_maker/
└── stock_market_maker.py

vercel.json            # Vercel configuration
requirements.txt       # Python dependencies
```

## Environment Variables

Set these in Vercel dashboard (Settings → Environment Variables):

```env
VERCEL=1
SECRET_KEY=your_secret_key_here
BLOCKCHAIN_NODE_URL=http://your-blockchain-node:8545
DATABASE_PATH=/tmp/gxc_explorer.db
FORUM_DATABASE_PATH=/tmp/gxc_forum.db
FORUM_REALTIME_PORT=3002
FORUM_SECRET_KEY=your_forum_secret_key
```

## Important Notes

### Database
- SQLite databases use `/tmp` directory (temporary)
- **Data is lost on each deployment** in `/tmp`
- **For production**: Use Vercel Postgres or external database (PostgreSQL, MongoDB, etc.)

### SocketIO Limitations
- Real-time features may have limitations in serverless
- Forum real-time server uses polling transport for better compatibility
- Consider using external WebSocket service (Pusher, Ably) for production

### Static Files
- Static files are automatically served via CDN
- Templates are included in deployment
- Cache headers are configured for optimal performance

## After Deployment

1. **Test all services:**
   - Visit your Vercel URL
   - Test explorer: `https://your-project.vercel.app/`
   - Test forum: `https://your-project.vercel.app/forum`
   - Test market maker: `https://your-project.vercel.app/market-maker`

2. **Update URLs:**
   - Remove any localhost references in templates
   - Update API endpoints if needed

3. **Configure Database (Production):**
   - Set up Vercel Postgres or external database
   - Update database connection strings
   - Migrate data from SQLite if needed

4. **Monitor:**
   - Check Vercel logs: `vercel logs`
   - Monitor function execution in Vercel dashboard
   - Set up alerts for errors

## Troubleshooting

### Import Errors
- Ensure all dependencies are in `requirements.txt`
- Check Python version (should be 3.9+)
- Verify file paths in serverless wrappers

### Database Errors
- Ensure database path uses `/tmp` directory
- Check file permissions
- Consider migrating to cloud database

### SocketIO Issues
- Check Vercel function logs
- Real-time features may need external service
- Use polling transport (already configured)

## Production Recommendations

1. **Database**: Use Vercel Postgres or external database
2. **Caching**: Use Vercel Edge Caching
3. **CDN**: Static files automatically served via CDN
4. **Monitoring**: Use Vercel Analytics
5. **Environment Variables**: Store secrets in Vercel dashboard
6. **Custom Domain**: Configure in Vercel project settings

## Support

- Vercel Docs: https://vercel.com/docs
- Check logs: `vercel logs`
- Function logs in Vercel dashboard

