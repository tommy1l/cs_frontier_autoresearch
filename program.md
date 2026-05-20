# Autoresearch Agent Instructions

You are an autonomous competitive programming researcher.

Your goal is to maximize the score on **Frontier-CS algorithmic problem 0** by iteratively improving `prompt_fix_human_problem_0.cpp`.

---

## Pre-Run Check: Count Logged Iterations

Before starting, check how many iterations are already logged:

    f="prompt_fix_human_problem_0.tsv"
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

2.  Copy the human solution as the starting point:

        cp initial_reference_problem_0.cpp prompt_fix_human_problem_0.cpp

3.  Create the log file with a header row:

        printf "commit\tbest_unbounded\tstatus\tdescription\n" > prompt_fix_human_problem_0.tsv

4.  Evaluate the human solution to establish a baseline:

        uv --directory Frontier-CS run python3 -c "
        from frontier_cs import SingleEvaluator
        evaluator = SingleEvaluator()
        result = evaluator.evaluate('algorithmic', problem_id=0, code=open('../prompt_fix_human_problem_0.cpp').read())
        print(f'Score (unbounded): {result.score_unbounded}')
        "

5.  Commit and log as iteration 0:

        git add prompt_fix_human_problem_0.cpp
        git commit -m "p0 score=<score> human baseline"
        git push origin HEAD
        git rev-parse --short HEAD

    Then append using the hash printed above — run this as a separate standalone command:

        printf "<hash>\t<score>\tkeep\thuman baseline\n" >> prompt_fix_human_problem_0.tsv

This counts as iteration 0. Then continue the loop for 15 more iterations.

---

## Files

| Solution file                    | Log file                         |
| -------------------------------- | -------------------------------- |
| `prompt_fix_human_problem_0.cpp` | `prompt_fix_human_problem_0.tsv` |

The evaluation command is:

    uv --directory Frontier-CS run python3 -c "
    from frontier_cs import SingleEvaluator
    evaluator = SingleEvaluator()
    result = evaluator.evaluate('algorithmic', problem_id=0, code=open('../prompt_fix_human_problem_0.cpp').read())
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
- `prompt_fix_human_problem_0.tsv`
- `prompt_fix_human_problem_0.cpp`

Optionally review past git history:

    git log --oneline
    git show <commit>:prompt_fix_human_problem_0.cpp | cat

---

### 2. Edit `prompt_fix_human_problem_0.cpp`

Make one meaningful algorithmic change per iteration.

Each iteration should explore a fundamentally different algorithmic approach, not refine the current one.

Do not edit:

- anything inside `Frontier-CS/`
- any other file

**Never make trivial non-algorithmic changes** such as adjusting time limits, buffer sizes, or constants that don't change the algorithm's logic.

---

### 3. Evaluate

    uv --directory Frontier-CS run python3 -c "
    from frontier_cs import SingleEvaluator
    evaluator = SingleEvaluator()
    result = evaluator.evaluate('algorithmic', problem_id=0, code=open('../prompt_fix_human_problem_0.cpp').read())
    print(f'Score (unbounded): {result.score_unbounded}')
    "

Find the output line:

    Score (unbounded): <float>

If score is `0.0`, rerun once, inspect output, fix if needed, evaluate again.

---

### 4. Commit

Run each of these as a separate standalone command:

    git add prompt_fix_human_problem_0.cpp

    git commit -m "p0 score=<score> <short description>"

    git push origin HEAD

    git rev-parse --short HEAD

---

### 5. Log result

Run each of these as a separate standalone command.

First get the commit hash:

    git rev-parse --short HEAD

Then append to `prompt_fix_human_problem_0.tsv` using the hash printed above:

    printf "<hash>\t<best_unbounded>\t<status>\t<description>\n" >> prompt_fix_human_problem_0.tsv

Status values:

- `keep` — score improved; keep the change
- `discard` — score equal or worse; log first, then revert by running each command separately:

      git show HEAD~1:prompt_fix_human_problem_0.cpp > prompt_fix_human_problem_0.cpp

      git add prompt_fix_human_problem_0.cpp

      git commit -m "p0 revert to previous"

      git push origin HEAD

- `crash` — evaluation failed or score was `0.0`

---

## prompt_fix_human_problem_0.tsv Format

    commit   best_unbounded  status   description
    a1b2c3d  67.109247       keep     human baseline
    b2c3d4e  68.421000       keep     improved approach
    c3d4e5f  66.900000       discard  worse result

---

## Safety Rules

- Always read `Frontier-CS/algorithmic/problems/0/statement.txt` before writing a solution
- Only edit `prompt_fix_human_problem_0.cpp`
- Never read files inside `Frontier-CS/` except for `Frontier-CS/algorithmic/problems/0/statement.txt`
- Never edit or create any file inside `Frontier-CS/`
- Never change the evaluation command structure
- Never run `g++`, `clang++`, `gcc`, `make`, or any manual compile command
- Always evaluate before committing
- Always log the exact printed score
- **Never use `$()`, `$var`, or `&&` chaining in any bash command. Every command must be run as a completely separate, standalone line with no variable substitution.**
- Never read or copy from `initial_reference_problem_0.cpp` after iteration 0, or any other existing solution file

---

## Simplicity Rule

Prefer simpler code when scores are equal.

---

## STOP WHEN DONE

Complete exactly 15 iterations, then stop. Do not ask the human for input during the run.
