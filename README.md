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
│   ├── lesson_details.c   # Detailed explanations, pitfalls, practice
│   ├── tutorials.c        # Existing worked tutorial sections
│   ├── tutorials_extra.c  # Additional course-specific explanations and examples
│   ├── quizzes.c          # Quiz questions and explanations
│   ├── exercises.c        # Coursework labs
│   ├── progress.c         # Saved XP, streak and completion data
│   └── lab_runner.c       # Local compiler and reference-output checks
├── lab_templates/         # 24 starter programs and 24 expected outputs
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

Progress is written to `codequest_progress.dat` in the current working directory, so launch from the project root for consistent save location. Labs can now be compiled and checked against provided reference output using the local GCC/Clang toolchain. The output comparison is educational feedback, not secure grading or a sandbox. The app does not require optional font or image assets: it tries a locally installed system font and falls back to the default Raylib font.

## Requirements and testing

The application compiles as C11 and uses the Raylib API. The Windows task uses GCC directly; the macOS Makefile uses the system C compiler. Native Windows and macOS builds against installed Raylib still need verification on those operating systems; syntax validation on Linux alone is not proof of native portability.

## Learning tracks, detailed notes and themes

- The left sidebar track list scrolls **independently** of the main page. Hover over the track list and use the mouse wheel to reveal lower tracks, then click a track to open its lessons. The XP summary remains fixed at the bottom.
- Every lesson now includes **Step-by-step**, **Code example**, **Common mistakes**, **Try it yourself**, and **Good to know** sections. Code snippets tagged C++ require a C++ compiler when you practise them separately; CodeQuest itself remains C11.
- Click **MOON/SUN** near the logo in the top-left sidebar to switch between dark and light mode. The selected mode is saved to `codequest_theme.cfg` in the working directory, separately from `codequest_progress.dat`.
- F11 fullscreen and Ctrl/Command + +/- zoom are preserved.

## Expanded tutorial notes

All 24 chapters now include two additional in-depth sections with explained examples and expected results, following a learn-example-practise format. These are integrated into the existing lessons, not separate files to open manually. The content is original and adapted to the uploaded C and C++ course topics.

The additional `src/tutorials.c` source is included in both the Makefile and Windows VS Code GCC task.

## Practice quizzes

Open **Practice quizzes** from the sidebar to see five topic quizzes: C Fundamentals, Pointers & Memory, Linked Lists, C++ Features, and STL. Each quiz contains all questions associated with its topic (30 questions in total across five quizzes). The menu displays a question count and number previously mastered, **not individual questions**.

Select **Start quiz**, choose an answer, and click **Check answer** to see its explanation. Click **Next question** to continue. After the final question, click **Finish quiz and view results**. The results page shows your score, percentage, each missed question with its correct answer and explanation, and a shortcut to study the topic. You can retry or return to the quiz menu.

Previously mastered questions and XP continue to be recorded using the existing progress file (`codequest_progress.dat`), with 20 XP awarded only the first time a question is answered correctly. The per-attempt score and review page are kept in memory for the current quiz session, so leave the results page open until you've reviewed it. Returning to the menu mid-quiz abandons that session (previously saved XP and mastered-question progress remain).

## Expanded course-aligned tutorial notes (November 2026)

Each of the 24 learning chapters now includes **four detailed tutorial subsections** (two existing and two newly added), along with the lesson introduction, step-by-step guide, original example, common mistakes, and practice prompt. The extra notes combine the supplied lecture and lab topics with original, beginner-friendly W3Schools-like explanations, annotated code snippets, expected output and exercises. This is original teaching text, not copied W3Schools material.

Course references: Intro to C; Lab1 (2).cpp; Pointers; Lecture on Pointers in C++ and linkedlist; memory management and pointers; Pointers & linkedList lab; templates and stl; STL Lists and Maps; Operator Overloading. Practical examples link directly to the supplied leap-year, prime, array, pointer-only string, singly and doubly linked-list, template, vector, list and map tasks.

**Build change:** `src/tutorials_extra.c` is compiled by both the Windows GCC VS Code task and the macOS Makefile. If updating an older project manually, copy that file, the modified `src/ui.c` and `include/curriculum.h`, and update the source list for both platforms.

## Interactive lab code editor (24 exercises)

Open **Coding labs** and select any exercise. Every lab now contains an editable
starter program and a small set of reference output tests in `lab_templates/`.

- **Run Code:** compile the current C/C++ source with an installed local compiler,
  execute it, and show compiler errors or console output.
- **Check Solution:** compile and run the same code, compare console output with
  the corresponding `.expected` reference file, and award **60 XP once** for a
  match. This is an educational output check, **not** exhaustive semantic
  verification or an anti-cheating system; follow all written task constraints.
- **Reset Starter:** restore the original unfinished template for that lab.
- **Drafts:** code is saved automatically when leaving/changing a lab or exiting
  the application, and whenever you press **Ctrl+S** (or Cmd+S). Saved in
  `codequest_lab_drafts/` beside the executable's working directory.
- **Editing:** click inside the editor to focus. Use arrow keys, Home/End,
  Backspace/Delete, Return, Tab, and Ctrl/Cmd+V. Ctrl/Cmd+C copies the complete
  editor buffer, not a selection. Large files are limited to ~16 KB.

### Compiler requirement

The **application** itself is written in C11 and rendered solely with Raylib;
the code runner uses OS process functions and an **existing compiler**:

- **Windows:** GCC and G++ on PATH, or in `C:\msys64\ucrt64\bin`.
- **macOS:** Clang and Clang++ from Xcode Command Line Tools (`xcode-select --install`).
- **Linux:** GCC and G++ on PATH (if using a compatible Raylib build).

C++ templates use C++17. C templates use C11. No compiler is bundled.
Always launch CodeQuest with the project root as the current working directory,
so it can find `lab_templates/`.

**Safety:** The compiled student's code runs directly on their computer with
normal user permissions. It is **not sandboxed**. Only run code you trust;
untrusted source can access or modify local files. Compilation is limited to
12 seconds and the program itself to 3 seconds, to help handle infinite loops.
These commands run synchronously, so CodeQuest may briefly stop redrawing while
compiling or running a lab. For stronger isolation, run in a dedicated sandbox
or virtual machine before using arbitrary student submissions.

## Lab editor keyboard and mouse editing

The Labs code editor supports **holding Backspace or Delete** to remove text
continuously (including across lines). Click and **drag the mouse** across
characters or lines to highlight a selection; keep dragging above or below
the visible editor to scroll through longer files. You can also extend a
selection with **Shift + arrow keys** (or Shift + Home/End).

- **Type, Backspace, or Delete** to replace or remove highlighted text.
- **Ctrl+A / Command+A:** select all code.
- **Ctrl+C / Command+C:** copy the selection.
- **Ctrl+X / Command+X:** cut the selection.
- **Ctrl+V / Command+V:** paste over the selection.
- **Ctrl+S / Command+S:** save the current draft.

Navigation and selected text are independent from compiled output; any edit
invalidates the prior test result. Saved drafts and all existing labs are
preserved.

## Lab editor navigation

The built-in editor displays a **vertical scrollbar** when source code extends beyond
its visible lines. Hover over the editor and use the mouse wheel to move through
code without scrolling the surrounding Labs page. You can also click the scrollbar
track or drag its purple thumb. Drag-selection and Shift+arrow selection are still
supported, and the viewport follows the caret when typing or navigating with keys.
Holding **Enter** (or Numpad Enter) now repeatedly inserts new lines, using the
same repeat timing as holding Backspace/Delete.
