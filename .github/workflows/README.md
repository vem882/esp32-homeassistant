# GitHub Actions Workflows

This directory contains automated workflows for building and releasing the ESP32 Home Assistant thermostat firmware.

## Workflows

### 1. Automatic Build and Release (`build-and-release.yml`)

**Triggers:**
- Push to `main` branch with changes in `sketch_jul3a/` directory
- Manual workflow dispatch

**What it does:**
- Automatically generates version numbers based on date and commit count
- Updates `FIRMWARE_VERSION` in the source code
- Builds the firmware using Arduino CLI
- Creates `version.json` with release information
- Commits and pushes the updated files
- Creates a GitHub release with the firmware binary
- Updates devices via OTA within 6 hours

**Version Format:** `YYYY.MM.DD.BUILD_NUMBER`
- Example: `2025.01.15.42` (42nd build on January 15, 2025)

### 2. Build Test (`build-test.yml`)

**Triggers:**
- Pull requests to `main` branch with changes in `sketch_jul3a/` directory

**What it does:**
- Tests that the code compiles successfully
- Checks firmware size
- Provides build status in PR comments
- Does NOT create releases or push changes

### 3. Manual Release (`manual-release.yml`)

**Triggers:**
- Manual workflow dispatch from GitHub Actions tab

**What it does:**
- Allows manual version specification (e.g., `1.2.3`)
- Custom release notes
- Force update option for critical updates
- Pre-release marking option
- Same build and release process as automatic workflow

**Input Parameters:**
- `version`: Version number (required, format: X.Y.Z)
- `release_notes`: Custom release notes (optional)
- `force_update`: Force update on all devices (optional)
- `prerelease`: Mark as pre-release (optional)

## Usage

### Automatic Releases
1. Make changes to your code in the `sketch_jul3a/` directory
2. Commit and push to the `main` branch
3. GitHub Actions will automatically:
   - Generate a new version number
   - Build the firmware
   - Create a release
   - Update the OTA files

### Manual Releases
1. Go to the "Actions" tab in your GitHub repository
2. Click "Manual Release" workflow
3. Click "Run workflow"
4. Fill in the required information:
   - Version number (e.g., `1.2.3`)
   - Release notes (optional)
   - Force update checkbox (for critical updates)
   - Pre-release checkbox (for beta releases)
5. Click "Run workflow"

### Testing Pull Requests
1. Create a pull request with your changes
2. GitHub Actions will automatically test the build
3. Check the build status in the PR before merging

## Configuration

The workflows are configured to:
- Use the latest Arduino CLI
- Install required ESP32 core and libraries
- Target the `esp32:esp32:esp32` board
- Generate releases with proper versioning
- Update OTA configuration automatically

## Required Libraries

The workflows automatically install these libraries:
- `TFT_eSPI` - Display library
- `XPT2046_Touchscreen` - Touch screen library
- `ArduinoJson` - JSON parsing library

## Files Generated

Each successful build generates:
- `firmware.bin` - The compiled firmware binary
- `version.json` - OTA update configuration
- Updated source code with new version number

## Monitoring

### Build Status
- Check the "Actions" tab to see build status
- Failed builds will show error details
- Successful builds will show firmware size and version

### Release Notifications
- GitHub will create releases automatically
- Each release includes:
  - Firmware binary download
  - Version information
  - Release notes
  - OTA update details

### Device Updates
- Devices check for updates every 6 hours
- Updates are applied automatically
- Check device serial output for update status

## Troubleshooting

### Build Failures
1. Check the Actions tab for error details
2. Common issues:
   - Syntax errors in code
   - Missing libraries
   - Compilation errors
   - File size too large

### Version Conflicts
- Automatic builds use date-based versioning
- Manual releases use semantic versioning
- Both systems are compatible

### OTA Update Issues
- Verify `version.json` is accessible
- Check firmware binary size
- Ensure devices have internet connectivity
- Monitor device serial output for errors

## Security

### Permissions
- Workflows use `GITHUB_TOKEN` for authentication
- No additional secrets required for public repositories
- Workflows can push to the repository and create releases

### Safety Features
- `[skip ci]` in commit messages prevents recursive builds
- Build tests run on pull requests before merging
- Manual release workflow requires explicit approval

## Best Practices

1. **Development Workflow:**
   - Create feature branches for development
   - Use pull requests for code review
   - Let automatic builds handle releases

2. **Emergency Releases:**
   - Use manual release workflow for critical updates
   - Enable force update for security fixes
   - Test thoroughly before forcing updates

3. **Version Management:**
   - Use semantic versioning for manual releases
   - Let automatic builds handle routine updates
   - Document breaking changes in release notes

4. **Testing:**
   - Always test builds locally first
   - Use pull requests to test builds in CI
   - Monitor device behavior after releases
