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
