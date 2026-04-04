## Build and Run

Use the build script from the repository root:
```
build_and_run.bat
```

This builds with MSYS2 UCRT64 and runs the compiled binary. Required dependencies (SDL3, OpenCV DLLs) are copied to `bin/` automatically.

## Development

- **Compiler**: g++ via MSYS2 UCRT64
- **Build configuration**: See `build_and_run.bat` for compiler flags and library paths
- **Editor tooling**: `compile_flags.txt` is configured for your IDE
- **Code style**: C++17, small focused changes, consistent with existing patterns
- **Structure**: Entry point is `src/main.cpp`; subsystems are separated into distinct modules (camera capture, game logic, rendering, tracking)

Build artifacts go to `bin/`. Modify the build script if changing include paths or libraries.

## Contributing

For now, keep changes localized to existing subsystems. Do not spread logic across `main.cpp`. If you modify include paths or library usage, update both `build_and_run.bat` and `compile_flags.txt`." 
