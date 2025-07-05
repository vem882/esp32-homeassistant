# GitHub Actions Permission Fix and Workflow Options

## Issue Identified
The GitHub Actions workflow failed with a 403 permission error when trying to push changes back to the repository:

```
remote: Permission to vem882/esp32-homeassistant.git denied to github-actions[bot].
fatal: unable to access 'https://github.com/vem882/esp32-homeassistant/': The requested URL returned error: 403
```

## Root Cause
The default `GITHUB_TOKEN` has limited permissions and cannot push to protected branches or repositories with certain security settings.

## Solutions Provided

### Option 1: Fixed Original Workflows (Recommended)
**Files:** `build-and-release.yml`, `manual-release.yml`

**Changes Made:**
- Added explicit permissions to workflows:
```yaml
permissions:
  contents: write
  packages: write
  actions: read
```

**Pros:**
- Maintains original workflow design
- Updates source code with version numbers
- Creates consistent releases

**Cons:**
- May still have permission issues depending on repository settings
- Requires write access to main branch

### Option 2: No-Push Workflow (Alternative)
**File:** `build-release-no-push.yml`

**How it works:**
- Builds firmware without modifying source repository
- Creates releases with version-specific firmware URLs
- Each release is self-contained
- No repository modifications required

**Pros:**
- No permission issues
- Cleaner separation of source and releases
- Works with any repository security settings
- Each release has unique download URLs

**Cons:**
- Source code version numbers aren't automatically updated
- Requires manual version management in code

## Repository Settings to Check

### 1. Actions Permissions
Go to: `Settings` → `Actions` → `General` → `Workflow permissions`

Ensure one of these is selected:
- ✅ **Read and write permissions** (Recommended)
- ❌ **Read repository contents and packages permissions** (Will cause 403 errors)

### 2. Branch Protection
Go to: `Settings` → `Branches` → `main` branch protection

If branch protection is enabled, you may need to:
- Allow GitHub Actions to bypass protection
- Add GitHub Actions as an exception
- Or use the no-push workflow option

## Recommended Setup

### For Most Users (Option 1):
1. **Update repository permissions:**
   - Go to Settings → Actions → General
   - Select "Read and write permissions"
   - Save changes

2. **Use the fixed workflows:**
   - `build-and-release.yml` - Main automatic builds
   - `manual-release.yml` - Manual version releases
   - `build-test.yml` - Pull request testing

### For Restricted Repositories (Option 2):
1. **Use the no-push workflow:**
   - Rename `build-release-no-push.yml` to `build-and-release.yml`
   - Remove or disable the original workflow

2. **Update OTA URLs in your code:**
   ```cpp
   // Use release-specific URLs
   #define OTA_UPDATE_URL "https://github.com/yourusername/esp32-homeassistant/releases/latest/download/version.json"
   #define OTA_FIRMWARE_URL "https://github.com/yourusername/esp32-homeassistant/releases/latest/download/firmware.bin"
   ```

## Testing the Fix

### Method 1: Push a small change
```bash
# Make a small change to trigger build
echo "// Build test" >> sketch_jul3a/sketch_jul3a.ino
git add sketch_jul3a/sketch_jul3a.ino
git commit -m "Test GitHub Actions fix"
git push origin main
```

### Method 2: Manually trigger workflow
1. Go to Actions tab in GitHub
2. Select "Manual Release" workflow
3. Click "Run workflow"
4. Fill in version number (e.g., "1.0.1")
5. Click "Run workflow"

## Expected Results

### If Fix Works:
- ✅ Build completes successfully
- ✅ GitHub release is created
- ✅ Firmware.bin and version.json are available for download
- ✅ No permission errors in logs

### If Still Issues:
- ❌ Permission denied errors
- ❌ Build fails with 403 errors
- **Solution:** Use Option 2 (no-push workflow)

## OTA Update Configuration

### Option 1 (Repository-based):
```cpp
#define OTA_UPDATE_URL "https://raw.githubusercontent.com/vem882/esp32-homeassistant/main/version.json"
#define OTA_FIRMWARE_URL "https://raw.githubusercontent.com/vem882/esp32-homeassistant/main/firmware.bin"
```

### Option 2 (Release-based):
```cpp
#define OTA_UPDATE_URL "https://github.com/vem882/esp32-homeassistant/releases/latest/download/version.json"
#define OTA_FIRMWARE_URL "https://github.com/vem882/esp32-homeassistant/releases/latest/download/firmware.bin"
```

## Summary

The permission issue has been addressed with:
1. **Added explicit permissions** to existing workflows
2. **Created alternative no-push workflow** as backup
3. **Provided clear documentation** for repository setup
4. **Multiple OTA URL strategies** for different setups

Choose the option that best fits your repository's security requirements and test thoroughly before deploying to production devices.
