# CS-Frontier Autoresearch
Automated research pipeline for solving frontier CS algorithm problems through iterative code improvement.

## Overview
This repo tracks experiments where an LLM iteratively improves solutions to frontier CS problems. Two starting point conditions are tested — a **normal** (AI-generated) starting point and a **human** starting point — with an optional **prompt_fix** variant that uses a modified prompting strategy of program.md to attempt to produce more algorithmic diversity across runs.

## Repo Structure
```
runs/
├── normal/              # Runs starting from an AI-generated starting point
│   ├── cpp/             # Iterated solution files (solutions_0.cpp ... solutions_N.cpp). You can see the actual iterations through the github history of the file
│   ├── plots/           # Graphs of score/performance across iterations
│   ├── logs/            # TSV logs of each run
│   └── prompt_fix/      # Same as above but with modified prompting strategy
│       ├── cpp/
│       ├── plots/
│       └── logs/
└── human/               # Runs starting from a human-written starting point
    ├── cpp/             # Iterated solution files
    │   └── initial/     # Original human starting point files (not iterated)
    ├── plots/
    ├── logs/
    └── prompt_fix/
        ├── cpp/
        ├── plots/
        └── logs/
analysis/                # Markdown analysis of line-by-line changes iteration to iteration
├── problem_0.md
├── problem_0_human.md
├── problem_1.md
└── problem_1_human.md
```

## File Types
| Extension | Description |
|-----------|-------------|
| `.cpp` | Solution files iterated on by the model |
| `.png` | Plots showing performance across iterations |
| `.tsv` | Full logs of each run (scores, diffs, metadata) |
| `.md` | Analysis of changes from iteration to iteration |

## Run Types
**Normal** — the model starts from an AI-generated solution and iterates.

**Human** — the model starts from a human-written solution (`runs/human/cpp/initial/`) and iterates. The initial file is kept separately as a reference and is not modified.

**Prompt Fix** — a variant of either run type using a modified prompting strategy. Found under `prompt_fix/` within each run type folder.

## Problems
Each problem is numbered (0, 1, ...). File names include the problem number (e.g. `solutions_0.cpp` = problem 0, `results_human_1.tsv` = human run on problem 1).

## Scripts
- `plot.py` — generates plots from TSV logs
- `program.md` — file for claude code to read and do agentic loop

## How to Run

1. **Adapt `program.md`** for your target problem and run type by updating:
   - The problem number (e.g. replace `1` with `0`)
   - The solution filename (e.g. `prompt_fix_human_problem_1.cpp` → `normal_problem_0.cpp`)
   - The log filename (e.g. `prompt_fix_human_problem_1.tsv` → `normal_problem_0.tsv`)
   - The commit message prefix (e.g. `p1` → `p0`)
   - The starting file in First-Time Setup (e.g. `initial_reference_problem_1.cpp` → the appropriate AI-generated or human starting point)

2. **Open Claude Code** in the repo root and run:
   ```
   read program.md and proceed
   ```

Claude Code will handle the rest — setting up the log file if needed, resuming from where it left off if interrupted, and stopping automatically after 15 iterations.

## How to Run (Advisor–Worker)

This is the alternate setup where two separate agents take turns: an **advisor** decides what to try next, and a **worker** implements it. Files involved:

- `advisor.md` — prompt the advisor reads. It runs as `claude -p` and is told to propose the next approach for the current iteration, then append a row to `split_problem_{N}_proposals.tsv`.
- `worker.py` — implementer. Reads the latest proposal, optionally reverts `split_solutions_{N}.cpp` to a prior commit, calls the Anthropic API to write the new C++, evaluates it, commits, pushes, and logs to `split_results_{N}.tsv`. Run standalone with `python worker.py --problem N`.
- `advisor_worker.py` — orchestrator. Alternates advisor → worker for each problem in `PROBLEMS` until 15 iterations are logged. Resumes from existing TSVs if interrupted.

Outputs per problem `N`:
- `split_problem_{N}_proposals.tsv` — one row per advisor proposal
- `split_solutions_{N}.cpp` — current solution being iterated
- `split_results_{N}.tsv` — per-iteration log (commit, score, status, approach)

### Steps

1. **Edit the problem list in `advisor_worker.py`.** Change the `PROBLEMS` constant near the top to the problems you want to run:
   ```python
   PROBLEMS = [0]        # or [2, 3], etc.
   ```
2. **Run from the repo root:**
   ```
   python advisor_worker.py
   ```

The orchestrator will skip iterations already logged in `split_results_{N}.tsv`, so it's safe to re-run after an interruption.
