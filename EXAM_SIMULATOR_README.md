# CodeQuest exam simulator

The new **Exam Simulator** card appears at the top of **Practice / Quizzes**.
It runs a 60-minute, five-question written coding exam based on the supplied
"Pointers & linkedList lab" worksheet (Coding Exam 2 preparation).

## Exam behaviour

- Every attempt randomly selects four pointer-function problems and one
  singly/doubly linked-list problem. The problem order changes each time.
- A real-time clock counts down from 60:00 to 00:00, even when moving between
  the five questions. Answers remain in memory while the exam is open.
- Students type C source answers (Tab indents, Enter starts a new line,
  and Cmd/Ctrl+V pastes from the clipboard).
- At zero, editing is locked and a **Time is up!** popup appears. Clicking
  **Review my answers** shows the submitted work and sample solutions.
- Students can also submit early by pressing **Submit entire exam**, then
  **Yes, submit**.
- The review allows students to mark each answer correct (+20) or needing
  practice. This is **manual self-marking**: the exam deliberately does not
  execute arbitrary code or pretend that textual similarity is a valid grade.
- The exam stays on the Practice page until submission so navigating away
  during the timed attempt does not silently discard work.

## Build and launch

On macOS with Homebrew raylib and pkg-config installed:

```sh
make clean
make
./bin/CodeQuest
```

The app remains a **C11 + raylib** project. No Python files or services are
needed. The only new source files are `src/exam_simulator.c` and
`include/exam_simulator.h`; existing screens and content are unchanged except
for a small integration in `src/ui.c` and the Makefile source list.

## Scope note

Answers and marks are held in memory for the current exam session and are
not persisted after closing the app. The timer is real-time while the app is
running; closing the app ends the attempt. Automatic compilation and marking
of submitted code are not part of this feature.
