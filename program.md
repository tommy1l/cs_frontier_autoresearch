# Autoresearch Agent Instructions

You are an autonomous competitive programming researcher.

Your goal is to maximize the score on **Frontier-CS algorithmic problem 0** by iteratively improving `solutions_0.cpp`.

---

## Pre-Run Check: Count Logged Iterations

Before starting, check how many iterations are already logged:

    f="results_0.tsv"
    if [ -f "$f" ]; then
      count=$(tail -n +2 "$f" | grep -c .)
      echo "Problem 0: $count iterations logged"
    else
      echo "Problem 0: 0 iterations logged (file not found)"
    fi

- If already **15 iterations logged** → stop, you are done.
- If **fewer than 15** → resume from where it left off, running only the remaining iterations needed to reach 15.
- If **0 or no file** → do First-Time Setup before starting the loop.

**Never redo iterations that are already logged.**

---

## First-Time Setup (only if 0 iterations logged)

1.  Read the problem statement carefully:

        cat Frontier-CS/algorithmic/problems/0/statement.txt

2.  Write a complete, working `solutions_0.cpp` from scratch based on the problem statement. This should be a full solution attempt — not an empty file.

3.  Create the log file with a header row:

        printf "commit\tbest_unbounded\tstatus\tdescription\n" > results_0.tsv

4.  Evaluate the new `solutions_0.cpp` to establish a baseline:

uv --directory Frontier-CS run python3 -c "
from frontier_cs import SingleEvaluator
evaluator = SingleEvaluator()
result = evaluator.evaluate('algorithmic', problem_id=0, code=open('../solutions_0.cpp').read())
print(f'Score (unbounded): {result.score_unbounded}')
"

5.  Commit and log as iteration 0:

        git add solutions_0.cpp
        git commit -m "p0 score=<score> initial solution"
        git push origin HEAD
        git rev-parse --short HEAD

    Then append using the hash printed above — run this as a separate standalone command:

        printf "<hash>\t<score>\tkeep\tinitial solution\n" >> results_0.tsv

This counts as iteration 0. Then continue the loop for 15 more iterations.

---

## Files

| Solution file     | Log file        |
| ----------------- | --------------- |
| `solutions_0.cpp` | `results_0.tsv` |

The evaluation command is:

    uv --directory Frontier-CS run python3 -c "
    from frontier_cs import SingleEvaluator
    evaluator = SingleEvaluator()
    result = evaluator.evaluate('algorithmic', problem_id=0, code=open('../solutions_0.cpp').read())
    print(f'Score (unbounded): {result.score_unbounded}')
    "

Rules:

- Never change the evaluation command structure
- Never use a different problem ID or solution file

---

## Loop

Complete **exactly 15 iterations**, then stop.

---

### 1. Read context

Read:

- `Frontier-CS/algorithmic/problems/0/statement.txt`
- `results_0.tsv`
- `solutions_0.cpp`

Optionally review past git history:

    git log --oneline
    git show <commit>:solutions_0.cpp | cat

---

### 2. Edit `solutions_0.cpp`

Make one meaningful algorithmic change per iteration.

Do not edit:

- anything inside `Frontier-CS/`
- any other file

**Never make trivial non-algorithmic changes** such as adjusting time limits, buffer sizes, or constants that don't change the algorithm's logic.

---

### 3. Evaluate

uv --directory Frontier-CS run python3 -c "
from frontier_cs import SingleEvaluator
evaluator = SingleEvaluator()
result = evaluator.evaluate('algorithmic', problem_id=0, code=open('../solutions_0.cpp').read())
print(f'Score (unbounded): {result.score_unbounded}')
"

Find the output line:

    Score (unbounded): <float>

If score is `0.0`, rerun once, inspect output, fix if needed, evaluate again.

---

### 4. Commit

Run each of these as a separate standalone command:

    git add solutions_0.cpp

    git commit -m "p0 score=<score> <short description>"

    git push origin HEAD

    git rev-parse --short HEAD

---

### 5. Log result

Run each of these as a separate standalone command.

First get the commit hash:

    git rev-parse --short HEAD

Then append to `results_0.tsv` using the hash printed above:

    printf "<hash>\t<best_unbounded>\t<status>\t<description>\n" >> results_0.tsv

Status values:

- `keep` — score improved; keep the change
- `discard` — score equal or worse; log first, then revert by running each command separately:

      git show HEAD~1:solutions_0.cpp > solutions_0.cpp

      git add solutions_0.cpp

      git commit -m "p0 revert to previous"

      git push origin HEAD

- `crash` — evaluation failed or score was `0.0`

---

## results_0.tsv Format

    commit   best_unbounded  status   description
    a1b2c3d  67.109247       keep     initial solution
    b2c3d4e  68.421000       keep     improved approach
    c3d4e5f  66.900000       discard  worse result

---

## Safety Rules

- Always read `Frontier-CS/algorithmic/problems/0/statement.txt` before writing a solution
- Only edit `solutions_0.cpp`
- Never read files inside `Frontier-CS/` except for `Frontier-CS/algorithmic/problems/0/statement.txt`
- Never edit or create any file inside `Frontier-CS/`
- Never change the evaluation command structure
- Never run `g++`, `clang++`, `gcc`, `make`, or any manual compile command
- Always evaluate before committing
- Always log the exact printed score
- **Never use `$()`, `$var`, or `&&` chaining in any bash command. Every command must be run as a completely separate, standalone line with no variable substitution.**
- Never read or copy from `reference_problem0.cpp` or any other existing solution file

---

## Simplicity Rule

Prefer simpler code when scores are equal.

---

## STOP WHEN DONE

Complete exactly 15 iterations, then stop. Do not ask the human for input during the run.
