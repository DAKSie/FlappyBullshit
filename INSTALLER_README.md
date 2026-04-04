# Flappy Bird - Installer Setup Guide

## System Requirements
- **OS:** Windows 7 or later (64-bit recommended)
- **Camera:** Webcam or camera device capable of capturing video
- **Architecture:** x64 (Intel/AMD)

## Building the Installer

### Prerequisites
1. **NSIS** - Download from https://nsis.sourceforge.io/
   - Run the installer and choose default options
   - The script assumes NSIS is installed in Program Files

2. **Game files** - Ensure you've built the game first:
   ```bash
   .\build_and_run.bat
   ```
   This creates `bin\jump.exe` and copies all required DLLs.

### Creating the Installer

Simply run:
```bash
.\build_installer.bat
```

This generates `FlappyBird-Installer.exe` in the repository root. The installer will:
- Install the game to `C:\Program Files\Flappy Bird`
- Create Start Menu shortcuts
- Add a Desktop shortcut
- Register uninstall functionality in Windows Control Panel

## Distribution

### Option 1: Distribute the Installer
- Share `FlappyBird-Installer.exe` directly
- Users run it and select installation location
- Professional installation experience

### Option 2: Portable Distribution (Alternative)
If you prefer no installation:
1. Copy the contents of `bin\` to a folder
2. Add `scoreboard.txt` (create empty file)
3. ZIP the folder
4. Share the ZIP file
Users extract and run `jump.exe` directly.

## How the Game Works

- **Launch:** Run `Flappy Bird` from Start Menu or Desktop
- **Player Name:** Enter your name when prompted (A, B, P, L characters)
- **Camera Control:** Move left/right to control the bird
- **Scoreboard:** High scores are saved in the installation directory

## Uninstall

Users can uninstall via Windows Control Panel:
- Settings → Apps → Apps & features
- Find "Flappy Bird" and click Uninstall

Or via Start Menu shortcut if created.

## Troubleshooting

### "Missing DLL" error
- Reinstall using the installer (ensures all DLLs are properly copied)
- Verify Windows is up to date

### Camera not detected
- Check Device Manager for webcam
- Ensure camera permissions are granted to the app
- Try reconnecting the camera

### Installer fails to build
- Verify NSIS is installed in Program Files
- Ensure `build_and_run.bat` was run successfully
- Check that `bin\jump.exe` exists

## Technical Details

The installer bundles:
- `jump.exe` - Main game executable
- SDL3 library for rendering
- OpenCV libraries for camera/face detection
- MSYS2 runtime libraries
- Entry point: Direct execution of `jump.exe`
