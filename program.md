# Autoresearch Agent Instructions

You are an autonomous competitive programming researcher.

Your goal is to maximize the score across **Frontier-CS algorithmic problems 1 through 10** by iteratively improving a separate solution file for each problem.

---

## Problem Setup

You will work through problems **1, 2, 3, 4, 5, 6, 7, 8, 9, 10** in order — completing exactly 15 iterations on problem N before moving on to problem N+1.

---

## Pre-Run Check: Count Logged Iterations

**Before starting any problem**, check how many iterations are already logged for each problem by running:

    for p in 1 2 3 4 5 6 7 8 9 10; do
      f="results_${p}.tsv"
      if [ -f "$f" ]; then
        count=$(tail -n +2 "$f" | grep -c .)
        echo "Problem $p: $count iterations logged"
      else
        echo "Problem $p: 0 iterations logged (file not found)"
      fi
    done

Use this output to determine where to resume:

- If a problem already has **15 iterations logged** → **skip it** and move to the next.
- If a problem has **fewer than 15** → **resume from where it left off**, running only the remaining iterations needed to reach 15.
- If a problem has **0 or no file** → begin from scratch (follow First-Time Setup).

**Never redo iterations that are already logged.**

---

Each problem has its own dedicated files:

| Problem | Solution file      | Log file         |
| ------- | ------------------ | ---------------- |
| 1       | `solutions_1.cpp`  | `results_1.tsv`  |
| 2       | `solutions_2.cpp`  | `results_2.tsv`  |
| …       | …                  | …                |
| 10      | `solutions_10.cpp` | `results_10.tsv` |

The evaluation command for problem **P** is:

    uv --directory Frontier-CS run frontier eval algorithmic <P> ../solutions_<P>.cpp

For example, for problem 3:

    uv --directory Frontier-CS run frontier eval algorithmic 3 ../solutions_3.cpp

Rules:

- Never change the evaluation command structure
- Never use the wrong problem ID or wrong solution file for a given problem

---

## First-Time Setup (only if `solutions_<P>.cpp` does not yet exist)

Before beginning the iteration loop for problem P, check whether `solutions_<P>.cpp` exists.

**If it does not exist**, do the following setup steps once:

1.  Read the problem statement carefully:

    cat Frontier-CS/algorithmic/problems/<P>/statement.txt

2.  Write a complete, working `solutions_<P>.cpp` from scratch based on the problem statement. This should be a full solution attempt — not an empty file. Do not copy from any other problem's solution file.

3.  Create the log file with a header row:

        printf "commit\tbest_score\tbest_unbounded\tstatus\tdescription\n" > results_<P>.tsv

    Verify it was created:

        cat results_<P>.tsv

4.  Run the first evaluation to establish a baseline score. This counts as iteration 1.

**If `solutions_<P>.cpp` already exists**, skip setup and go directly to the iteration loop.

---

## Files

- `solutions_<P>.cpp` — the only file to edit when working on problem P; written from scratch for each problem
- `program.md` — instructions; read-only
- `results_<P>.tsv` — experiment log for problem P
- `Frontier-CS/algorithmic/problems/<P>/statement.txt` — the problem statement for problem P

Do **not** create or modify files inside `Frontier-CS/`.

---

## Critical Evaluation Rule

All evaluation must be done **only** through:

    uv --directory Frontier-CS run frontier eval algorithmic <P> ../solutions_<P>.cpp

Never manually compile or run solution files.

Do **not** run:

- `g++`
- `clang++`
- `gcc`
- `make`
- custom compile commands
- manual test binaries

These produce misleading results on this machine.

If evaluation fails or score is `0.0`, rerun the official evaluation command and inspect its output. Do not switch to manual compilation.

---

## Loop (per problem P)

Complete **exactly 15 iterations** for problem P, then move on to problem P+1. Do not stop early. Do not do more than 15.

---

### 1. Read context

Read:

- `Frontier-CS/algorithmic/problems/<P>/statement.txt` — problem statement
- `results_<P>.tsv` — your history for this problem
- `solutions_<P>.cpp` — current solution

Optionally review past git history for ideas on what has or hasn't worked:

    git log --oneline
    git show <commit>:solutions_<P>.cpp

---

### 3. Edit `solutions_<P>.cpp`

Edit only:

    solutions_<P>.cpp

Make one meaningful change per iteration.

Do not edit:

- anything inside `Frontier-CS/`
- `program.md`
- any other problem's solution file

**Never make trivial non-algorithmic changes** such as adjusting time limits, buffer sizes, or constants that don't change the algorithm's logic. Every iteration must implement a meaningfully different algorithmic strategy.

---

### 4. Evaluate

Run exactly:

    uv --directory Frontier-CS run frontier eval algorithmic <P> ../solutions_<P>.cpp

Find the output line:

    Score: <float>

Use that exact score value for the commit message and `results_<P>.tsv`.

If score is `0.0`:

- rerun the official evaluation command once
- inspect only the official evaluation output
- fix `solutions_<P>.cpp` if needed
- evaluate again using the same official command

---

### 5. Commit

After every completed evaluation, commit:

    git add solutions_<P>.cpp
    git commit -m "p<P> score=<score> <short description>"
    git push origin HEAD

Then get the short commit hash:

    git rev-parse --short HEAD

---

### 6. Log result

Append one tab-separated row to `results_<P>.tsv`:

    <commit>	<score>	<score>	<status>	<description>

Status values:

- `keep` — score improved over the previous best; keep the change
- `discard` — score was equal or worse; log first, then revert:

      git checkout HEAD~1 -- solutions_<P>.cpp

- `crash` — official evaluation failed or score was `0.0`

---

## results\_<P>.tsv Format

    commit   best_score  best_unbounded  status   description
    a1b2c3d  67.109247   67.109247       keep     initial solution
    b2c3d4e  68.421000   68.421000       keep     randomized piece ordering
    c3d4e5f  66.900000   66.900000       discard  worse placement tie-breaker
    d4e5f6g  0.000000    0.000000        crash    compile/runtime error from official evaluator

---

## Safety Rules

- Always read `Frontier-CS/algorithmic/problems/<P>/statement.txt` before writing a solution for problem P
- Always edit `solutions_<P>.cpp` when working on problem P — never the wrong file
- Never edit or create any file inside `Frontier-CS/`
- Never change the evaluation command structure
- Never use the wrong problem ID
- Never manually compile
- Always evaluate before committing
- Always log the exact printed score

---

## Simplicity Rule

Prefer simpler code when scores are equal.
Avoid large complexity increases for tiny gains.

---

## STOP WHEN DONE

Complete exactly 15 iterations for each of problems 1–10 in order, then stop.
Do not loop back. Do not ask the human for input during the run.
