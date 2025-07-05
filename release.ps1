# ESP32 Home Assistant OTA Release Script
# This PowerShell script helps automate the release process

param(
    [Parameter(Mandatory=$true)]
    [string]$Version,
    
    [Parameter(Mandatory=$false)]
    [string]$ReleaseNotes = "Bug fixes and improvements",
    
    [Parameter(Mandatory=$false)]
    [string]$MinVersion = "1.0.0",
    
    [Parameter(Mandatory=$false)]
    [bool]$ForceUpdate = $false
)

Write-Host "ESP32 Home Assistant OTA Release Script" -ForegroundColor Green
Write-Host "=======================================" -ForegroundColor Green

# Check if Arduino IDE is installed
$arduinoPath = Get-Command arduino-cli -ErrorAction SilentlyContinue
if (-not $arduinoPath) {
    Write-Host "Arduino CLI not found. Please install Arduino CLI first." -ForegroundColor Red
    exit 1
}

# Check if git is available
$gitPath = Get-Command git -ErrorAction SilentlyContinue
if (-not $gitPath) {
    Write-Host "Git not found. Please install Git first." -ForegroundColor Red
    exit 1
}

# Paths
$sketchPath = "sketch_jul3a\sketch_jul3a.ino"
$firmwarePath = "firmware.bin"
$versionPath = "version.json"

Write-Host "Building firmware..." -ForegroundColor Yellow

# Compile the sketch
arduino-cli compile --fqbn esp32:esp32:esp32 $sketchPath --output-dir build

if ($LASTEXITCODE -ne 0) {
    Write-Host "Compilation failed!" -ForegroundColor Red
    exit 1
}

# Copy firmware binary
$builtFirmware = "build\sketch_jul3a.ino.bin"
if (Test-Path $builtFirmware) {
    Copy-Item $builtFirmware $firmwarePath
    Write-Host "Firmware binary created: $firmwarePath" -ForegroundColor Green
} else {
    Write-Host "Firmware binary not found at: $builtFirmware" -ForegroundColor Red
    exit 1
}

# Get firmware size
$firmwareSize = (Get-Item $firmwarePath).Length
Write-Host "Firmware size: $firmwareSize bytes" -ForegroundColor Cyan

# Create version.json
$versionData = @{
    version = $Version
    firmware_url = "https://raw.githubusercontent.com/vem882/esp32-homeassistant/main/firmware.bin"
    release_notes = $ReleaseNotes
    min_version = $MinVersion
    force_update = $ForceUpdate
    build_date = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    firmware_size = $firmwareSize
}

$versionJson = $versionData | ConvertTo-Json -Depth 3
$versionJson | Out-File -FilePath $versionPath -Encoding utf8

Write-Host "Version file created: $versionPath" -ForegroundColor Green

# Display summary
Write-Host "`nRelease Summary:" -ForegroundColor Cyan
Write-Host "Version: $Version" -ForegroundColor White
Write-Host "Release Notes: $ReleaseNotes" -ForegroundColor White
Write-Host "Min Version: $MinVersion" -ForegroundColor White
Write-Host "Force Update: $ForceUpdate" -ForegroundColor White
Write-Host "Firmware Size: $firmwareSize bytes" -ForegroundColor White

# Git operations
Write-Host "`nGit Operations:" -ForegroundColor Yellow

# Check git status
$gitStatus = git status --porcelain
if ($gitStatus) {
    Write-Host "Adding files to git..." -ForegroundColor Yellow
    git add $firmwarePath
    git add $versionPath
    
    # Commit changes
    $commitMessage = "Release version $Version - $ReleaseNotes"
    git commit -m $commitMessage
    
    Write-Host "Committed changes with message: $commitMessage" -ForegroundColor Green
    
    # Ask user if they want to push
    $pushChoice = Read-Host "Push to GitHub? (y/n)"
    if ($pushChoice -eq "y" -or $pushChoice -eq "Y") {
        git push origin main
        Write-Host "Pushed to GitHub!" -ForegroundColor Green
        Write-Host "OTA update will be available to devices within 6 hours." -ForegroundColor Cyan
    } else {
        Write-Host "Changes committed locally. Run 'git push' to publish the update." -ForegroundColor Yellow
    }
} else {
    Write-Host "No changes detected in git." -ForegroundColor Yellow
}

Write-Host "`nRelease process completed!" -ForegroundColor Green

# Instructions
Write-Host "`nNext Steps:" -ForegroundColor Cyan
Write-Host "1. Update the GitHub repository URLs in your code if needed" -ForegroundColor White
Write-Host "2. Test the OTA update on a development device" -ForegroundColor White
Write-Host "3. Monitor device logs for successful updates" -ForegroundColor White
Write-Host "4. Keep this version of firmware.bin as a backup" -ForegroundColor White
