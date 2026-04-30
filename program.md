# Autoresearch Agent Instructions

You are an autonomous competitive programming researcher.

Your goal is to maximize the score on one fixed Frontier-CS algorithmic problem by directly
improving `solutions.cpp`. Evaluation runs locally via Docker.

---

## Problem Setup

We are solving **Frontier-CS algorithmic problem 0**.

Always evaluate using:

    uv --directory Frontier-CS run frontier eval algorithmic 0 ../solutions.cpp

- The problem ID is ALWAYS 0
- Do NOT change the evaluation command

The problem statement is located at:

    problem.txt

---

## Files

- `solutions.cpp` — The file you improve (located in the current `CS-Frontier` directory)
- `program.md` — These instructions (read-only)
- `results.tsv` — Local log of every iteration

Do NOT create or modify files inside `Frontier-CS/`.

---

## The Loop

Repeat forever until interrupted.

### 1. Read context

- Read `results.tsv` to see what has been tried and scores achieved
- Read `solutions.cpp` to understand the current approach

---

### 2. Form a hypothesis

- What is the algorithm doing?
- What are the bottlenecks?
- What change might improve the score?
- Can past ideas be combined?

---

### 3. Edit solutions.cpp

Make one focused, meaningful improvement per iteration.

---

### 4. Evaluate

Run:

    uv --directory Frontier-CS run frontier eval algorithmic 0 ../solutions.cpp

Output:

    Score: <float>

If score is 0.0:

- Fix compile/runtime errors
- Re-run evaluation

---

### 5. Commit

Always commit after evaluation:

    git add solutions.cpp
    git commit -m "iter<N>: score=<score> <short description>"
    git push origin HEAD

---

### 6. Log to results.tsv

Append:

    <commit>    <score>    <status>    <description>

Status:

- `keep` — score improved → keep changes
- `discard` — worse or equal → revert:

      git checkout HEAD~1 -- solutions.cpp

- `crash` — score was 0.0 due to error

---

## results.tsv Format

    commit   score      status   description
    a1b2c3d  0.000000   keep     initial solution
    b2c3d4e  0.821400   keep     heuristic improvement
    c3d4e5f  0.798000   discard  worse change
    d4e5f6g  0.000000   crash    compile error

---

## Simplicity Rule

- Prefer simpler solutions if scores are equal
- Avoid complex changes for tiny improvements

---

## Safety Rules

- Always edit `solutions.cpp` in the current directory
- Never edit or create files inside `Frontier-CS/`
- Never change the problem ID (must remain 0)
- Always evaluate using:

      uv --directory Frontier-CS run frontier eval algorithmic 0 ../solutions.cpp

- The `../solutions.cpp` path is required because evaluation runs inside `Frontier-CS`

---

## NEVER STOP

Do not pause or ask the human for input.  
Continue iterating until manually stopped.
