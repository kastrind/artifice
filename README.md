# Artifice Engine v.0.0.1

## Development Environment Setup in Windows

### Instructions

1. Install MSYS2: https://www.msys2.org/

2. In MSYS2, install these packages:
```
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-SDL2
pacman -S mingw-w64-x86_64-glew
pacman -S mingw-w64-x86_64-glm
```

3. Add this Terminal Profile in VS Code:

```
"MSYS2-MINGW64": {
    "path": "C:\\msys64\\msys2_shell.cmd",
    "args": [
        "-defterm",
        "-here",
        "-no-start",
        "-mingw64"
    ],
    "icon": "terminal-bash"
}
```

4. If `make` does not work, open an elevated CMD prompt and enter:
```
C:\msys64\mingw64\bin>mklink make mingw32-make.exe
```

5. Enter `make all`
