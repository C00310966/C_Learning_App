# CodeQuest — Learn C and C++ with Raylib

Native desktop learning app written in **C11 and Raylib**. Includes 24 sectioned lessons, 30 practice questions, 24 hands-on labs, answer explanations, XP/levels, learning streaks, completion tracking and focus-area feedback. The material follows the supplied C, pointers, memory management, linked lists, templates, operator-overloading and STL lectures/labs. **C++ is a learning topic, not the language used to build the application.**

## Structure

```text
CodeQuest-Raylib/
├── .vscode/
│   ├── settings.json      # GCC and Raylib paths (Windows)
│   ├── tasks.json         # Ctrl+Shift+B direct GCC build
│   ├── launch.json        # F5 GDB debug
│   └── extensions.json
├── include/
│   ├── app.h
│   ├── curriculum.h
│   └── progress.h
├── src/
│   ├── main.c             # Application entry point
│   ├── ui.c               # Raylib pages and interactions
│   ├── tracks.c           # Learning categories
│   ├── lessons.c          # Lesson notes and examples
│   ├── quizzes.c          # Quiz questions and explanations
│   ├── exercises.c        # Coursework labs
│   └── progress.c         # Saved XP, streak and completion data
├── Makefile
└── README.md
```

## Build on macOS

Install the Xcode Command Line Tools, Make, pkg-config and Raylib (for example with Homebrew: `brew install raylib pkg-config`). Open the project folder in the VS Code terminal and run:

```sh
make
./bin/CodeQuest
```

The Makefile discovers Raylib through `pkg-config`. If needed, override `RAYLIB_CFLAGS` and `RAYLIB_LIBS` with flags appropriate for your Raylib installation. To rebuild, use `make clean && make`.

## Build on Windows (VS Code and GCC)

1. Install MSYS2 and use the **UCRT64** environment. In its terminal install the GCC compiler, GDB and Raylib: `pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-raylib`.
2. Install VS Code with the Microsoft **C/C++** extension.
3. Open the **CodeQuest-Raylib** folder in VS Code (not an individual `.c` file).
4. Check `.vscode/settings.json`: `codequest.raylibInclude` and `codequest.raylibLib` should point to the actual Raylib `include` and `lib` directories. The defaults target `C:/msys64/ucrt64/include` and `C:/msys64/ucrt64/lib`. If your GCC installation lives elsewhere, also update the GCC paths in `.vscode/tasks.json` and `.vscode/launch.json`.
5. Press **Ctrl+Shift+B** and select **Build CodeQuest**. Output: `bin/CodeQuest.exe`.
6. Run in PowerShell from the project root:

```powershell
.\bin\CodeQuest.exe
```

Press **F5** (or **Fn+F5** if your keyboard requires Fn) to build and debug using GDB. No Make installation is required on Windows. GCC, Raylib and GDB must all have matching 64-bit architectures. If a Raylib DLL is required, the UCRT64 `bin` directory must be in PATH when launching; the supplied debugger configuration sets that PATH. For the standalone PowerShell command, add `C:\msys64\ucrt64\bin` to PATH or place the required DLL next to the executable.

## Window sizing, fullscreen and accessibility

CodeQuest automatically scales its **entire interface** to the window size and monitor resolution, including text, buttons and the sidebar. The fullscreen layout fix uses independent, consistently zoomed camera passes for fixed navigation and scrollable pages (rather than nested cameras, which previously shrank the content). It draws through a Raylib 2D camera at native output resolution for sharp text. The normal window uses a larger default UI scale than earlier builds, and fullscreen uses the available monitor space without shrinking the interface.

- **F11:** toggle fullscreen / windowed mode.
- **Ctrl + +** (or **Ctrl + =**): increase UI zoom.
- **Ctrl + -**: decrease UI zoom.
- **Ctrl + 0:** restore the default UI zoom.
- On macOS, **Command** can be used instead of Ctrl for zoom.

Zoom is limited when the window is too small to keep the navigation and lesson column usable. To enlarge further, increase the window size or enter fullscreen. Zoom changes last until you close the app.

## Learning progress

Progress is written to `codequest_progress.dat` in the current working directory, so launch from the project root for consistent save location. Labs are **self-checked**, not automatically compiled or graded, and the app does not execute untrusted code. The app does not require optional font or image assets: it tries a locally installed system font and falls back to the default Raylib font.

## Requirements and testing

The application compiles as C11 and uses the Raylib API. The Windows task uses GCC directly; the macOS Makefile uses the system C compiler. Native Windows and macOS builds against installed Raylib still need verification on those operating systems; syntax validation on Linux alone is not proof of native portability.
