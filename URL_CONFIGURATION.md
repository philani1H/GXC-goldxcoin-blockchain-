# 🔗 URL Configuration - Environment-Aware Navigation

## ✅ Implementation Complete

All navigation links are now environment-aware and will automatically use:
- **Production URLs** when deployed on Vercel
- **Localhost URLs** when running locally

## 📋 URL Variables

Three template variables are now available in all templates:

1. **`explorer_url`** 
   - Production: `https://gxc-blockchain.vercel.app`
   - Local: `http://localhost:3000`

2. **`market_maker_url`**
   - Production: `https://gxc-blockchain-market-maker.vercel.app`
   - Local: `http://localhost:4000`

3. **`forum_url`**
   - Production: `https://gxc-blockchain-forum.vercel.app`
   - Local: `http://localhost:3001`

## 🔧 How It Works

### Environment Detection

The code automatically detects if running on Vercel by checking:
- `VERCEL` environment variable
- `VERCEL_URL` environment variable

### Template Context Processors

Both `web/forum.py` and `web/blockchain_explorer.py` now have context processors that inject these URLs into all templates:

```python
@app.context_processor
def inject_urls():
    return {
        'explorer_url': EXPLORER_URL,
        'market_maker_url': MARKET_MAKER_URL,
        'forum_url': FORUM_URL
    }
```

### Usage in Templates

All hardcoded URLs have been replaced with template variables:

**Before:**
```html
<a href="https://gxc-blockchain.vercel.app">Explorer</a>
```

**After:**
```html
<a href="{{ explorer_url }}">Explorer</a>
```

## 📝 Files Updated

### Backend Files:
- ✅ `web/forum.py` - Added URL detection and context processor
- ✅ `web/blockchain_explorer.py` - Added URL detection and context processor
- ✅ `market_maker/stock_market_maker.py` - Added URL detection and context processor

### Template Files:
- ✅ `web/templates/forum/base.html` - Updated all navigation links
- ✅ `web/templates/explorer_index.html` - Updated forum links
- ✅ `web/templates/stocks.html` - Updated forum and market maker links
- ✅ `web/templates/stocks_guide.html` - Updated market maker links
- ✅ `web/templates/forum/live_chat.html` - Updated context URLs

## 🎯 Result

- **In Production (Vercel)**: All links point to production URLs
- **In Local Development**: All links point to localhost URLs
- **No manual changes needed**: Automatically adapts to environment

## 🔄 Override with Environment Variables

You can override URLs using environment variables:
- `EXPLORER_URL` - Override explorer URL
- `MARKET_MAKER_URL` - Override market maker URL
- `FORUM_URL` - Override forum URL

Example:
```bash
export EXPLORER_URL=https://custom-explorer.com
export MARKET_MAKER_URL=https://custom-market-maker.com
export FORUM_URL=https://custom-forum.com
```

