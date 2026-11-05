# Pointer Academy — C + Raylib starter app

A small playable course prototype with:
- Main menu
- Five pointer tutorials
- Reusable quiz interface (20 questions, multiple choice and exact-output)
- Answer explanations and results screen

## Compile on macOS (Homebrew Raylib)

Open VS Code's Terminal in this directory, then run:

```bash
clang main.c content.c -o pointer_academy $(pkg-config --cflags --libs raylib)
./pointer_academy
```

If needed: `brew install raylib pkgconf`

## Controls

- Mouse: navigate and select answers
- A/B/C/D: multiple-choice answers
- Type text: exact-output questions
- Enter: submit / next question
- Backspace: edit answer
- Esc: close the app

## Adding questions

Edit the `QUESTIONS` array in `content.c`. Each record contains a lesson index, question type, prompt, optional code, up to four options, correct answer, and an explanation. The quiz UI in `main.c` renders the question automatically.

## Known scope / next steps

This is an introductory pointers prototype, not a C compiler. Output responses are checked against exact stored text. It doesn't save scores between sessions. Expand with exam 1/2 content, progression per topic, hints, keyboard focus, scrolling, and improved accessibility later.

## Content

The content is adapted from the C Pointers Quiz supplied for this assignment, with two additional output questions and explanations.
