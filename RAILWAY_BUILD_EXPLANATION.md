# Railway Build Explanation

## Current Setup: Build on Railway

The current `Dockerfile.testnet` **builds the C++ node on Railway** during deployment. This is the recommended approach because:

### ✅ Advantages:
1. **No local build needed** - Railway handles everything
2. **Consistent environment** - Same build environment every time
3. **Automatic updates** - Rebuilds when code changes
4. **No binary compatibility issues** - Built for Railway's environment

### How It Works:

1. **Railway receives your code**
2. **Runs `Dockerfile.testnet`** which:
   - Installs build dependencies
   - Copies source files
   - Builds the C++ node
   - Creates configuration
   - Sets up the runtime
3. **Starts the node** with Railway's PORT

### Build Time:
- **First build**: ~5-10 minutes (downloads dependencies, compiles)
- **Subsequent builds**: ~3-5 minutes (Docker layer caching)

## Alternative: Pre-built Binary (Optional)

If you want to build locally and just copy the binary:

### Option 1: Use Runtime Dockerfile

```bash
# 1. Build locally (when build environment is available)
./build_and_deploy_testnet.sh

# 2. Update railway.toml to use runtime Dockerfile
# Change: dockerfilePath = "Dockerfile.testnet.runtime"

# 3. Deploy
railway up
```

**Note:** This requires:
- Local build environment with all dependencies
- Binary compatibility with Railway's runtime (same OS/arch)
- Manual rebuild and push for each update

### Option 2: Keep Build Dockerfile (Current - Recommended)

Railway builds on deployment:
- ✅ Automatic
- ✅ Always up-to-date
- ✅ No local build needed
- ✅ Works from any machine

## Current Configuration

**railway.toml:**
```toml
[build]
builder = "DOCKERFILE"
dockerfilePath = "Dockerfile.testnet"  # Builds on Railway
```

**Dockerfile.testnet:**
- Builds C++ node from source
- Creates testnet config
- Runs the node

## Summary

**Current setup (recommended):**
- Railway builds the node automatically
- No local build required
- Just run `railway up`

**If you want to build locally:**
- Fix local build environment (install libstdc++)
- Build with `./build_and_deploy_testnet.sh`
- Switch to `Dockerfile.testnet.runtime` in railway.toml
- Deploy with `railway up`

**For now, Railway will build it automatically!** 🚀
