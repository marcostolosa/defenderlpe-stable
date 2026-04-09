# BlueHammer-Stable
 Stabilized PoC for the Windows Defender TOCTOU LPE. Original research by Tom Gallagher, Igor Tsyganskiy, and Jeremy Tinder with original PoC by Nightmare-Eclipose
 
This version seems to work everytime without hanging at random points.
 
## Fixes
 
- Configurable timeout with fallback
- Uses existing VSS snapshot
- Prevents folder locking due to cloud sync issues
- Headless Support
 
## Configuration
 
Check `config.h` and adjust for compatibility with C2 frameworks or whatever you desire to do.
 
## Build
 
Visual Studio 2019+, Release x64.
 
## Credits
 
https://github.com/Nightmare-Eclipse/BlueHammer

Original research: Tom Gallagher, Igor Tsyganskiy, Jeremy Tinder
