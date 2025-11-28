# 🚀 Quick Vercel Deployment

## Deploy in 3 Steps

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

- ✅ **Blockchain Explorer** (`/`) - Main explorer interface
- ✅ **Forum** (`/forum`) - Discussion forum with chat
- ✅ **Static Files** - All CSS, JS, images
- ✅ **Templates** - All HTML templates

## Important Notes

### Database
- SQLite databases will use `/tmp` directory (temporary)
- **For production**: Consider using Vercel Postgres or external database
- Data in `/tmp` is lost on each deployment

### SocketIO
- Real-time features may have limitations in serverless
- Consider using polling transport or external WebSocket service

### Environment Variables
Set these in Vercel dashboard:
- `SECRET_KEY` - Flask secret key
- `BLOCKCHAIN_NODE_URL` - Your blockchain node URL
- `DATABASE_PATH` - Database path (default: `/tmp/gxc_*.db`)

## After Deployment

1. Update URLs in templates (remove localhost references)
2. Configure custom domain (optional)
3. Set up external database (recommended for production)
4. Test all features

## Support

Check `DEPLOYMENT.md` for detailed instructions and troubleshooting.

