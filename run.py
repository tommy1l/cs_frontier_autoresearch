#!/usr/bin/env python3
"""
Orchestrator for Frontier-CS algorithmic autoresearch.

Alternates between advisor (claude -p) and implementer (worker.py)
for each problem, 15 iterations each.

Usage:
    python run.py
"""

import os
import subprocess
import sys
import time

PROBLEMS = [0]
MAX_ITERATIONS = 15
PROPOSER_PROMPT = "advisor.md"
IMPLEMENTER_SCRIPT = "worker.py"


def count_iterations(problem: int) -> int:
    tsv = f"split_results_{problem}.tsv"
    if not os.path.exists(tsv):
        return 0
    with open(tsv) as f:
        lines = [l for l in f.readlines()[1:] if l.strip()]
    return len(lines)


def proposal_exists_for(problem: int, iteration: int) -> bool:
    tsv = f"split_problem_{problem}_proposals.tsv"
    if not os.path.exists(tsv):
        return False
    with open(tsv) as f:
        lines = f.readlines()
    for line in lines[1:]:
        parts = line.strip().split("\t")
        if parts and parts[0] == str(iteration):
            return True
    return False


def ensure_proposals_file(problem: int) -> None:
    tsv = f"split_problem_{problem}_proposals.tsv"
    if not os.path.exists(tsv):
        with open(tsv, "w") as f:
            f.write("iteration\tapproach\tdetails\trationale\trevert_to\n")


def last_proposal_iteration(problem: int) -> str | None:
    tsv = f"split_problem_{problem}_proposals.tsv"
    if not os.path.exists(tsv):
        return None
    with open(tsv) as f:
        lines = [l for l in f.readlines() if l.strip()]
    if len(lines) < 2:
        return None
    parts = lines[-1].strip().split("\t")
    return parts[0] if parts else None


def run_proposer(problem: int, iteration: int) -> bool:
    print(f"\n[proposer] Problem {problem} iteration {iteration}...")
    with open(PROPOSER_PROMPT) as f:
        prompt = f.read()
    prompt = prompt.replace("{N}", str(problem)).replace("{ITER}", str(iteration))
    result = subprocess.run(
        ["claude", "-p", prompt, "--dangerously-skip-permissions"],
        capture_output=False,
        text=True,
    )
    if result.returncode != 0:
        print(f"[proposer] ERROR: exited with code {result.returncode}")
        return False
    return True


def run_implementer(problem: int) -> bool:
    print(f"\n[implementer] Problem {problem}...")
    result = subprocess.run(
        [sys.executable, IMPLEMENTER_SCRIPT, "--problem", str(problem)],
        capture_output=False,
        text=True,
    )
    if result.returncode != 0:
        print(f"[implementer] ERROR: exited with code {result.returncode}")
        return False
    return True


def main():
    if not os.path.exists(PROPOSER_PROMPT):
        print(f"Error: {PROPOSER_PROMPT} not found")
        sys.exit(1)
    if not os.path.exists(IMPLEMENTER_SCRIPT):
        print(f"Error: {IMPLEMENTER_SCRIPT} not found")
        sys.exit(1)

    print(f"Starting autoresearch on problems {PROBLEMS}")
    print(f"Max iterations per problem: {MAX_ITERATIONS}\n")

    for problem in PROBLEMS:
        iterations_done = count_iterations(problem)
        print(f"Problem {problem}: {iterations_done}/{MAX_ITERATIONS} iterations already done")

        while count_iterations(problem) < MAX_ITERATIONS:
            iteration = count_iterations(problem)
            print(f"\n{'='*50}")
            print(f"Problem {problem} | Iteration {iteration}/{MAX_ITERATIONS - 1}")
            print(f"{'='*50}")

            # Run proposer if proposal doesn't exist yet for this iteration
            if not proposal_exists_for(problem, iteration):
                ensure_proposals_file(problem)
                ok = run_proposer(problem, iteration)
                if not ok:
                    print(f"[orchestrator] Proposer failed on problem {problem} iteration {iteration}. Retrying in 10s...")
                    time.sleep(10)
                    ok = run_proposer(problem, iteration)
                    if not ok:
                        print(f"[orchestrator] Proposer failed twice. Skipping to next problem.")
                        break

                # Verify proposal was written for the expected iteration
                last_iter = last_proposal_iteration(problem)
                if last_iter != str(iteration):
                    print(
                        f"[orchestrator] Proposer wrote iteration={last_iter!r}, expected {iteration!r}. "
                        f"Skipping problem {problem} to avoid infinite loop."
                    )
                    break
            else:
                print(f"[proposer] Proposal for iteration {iteration} already exists, skipping.")

            # Run implementer
            ok = run_implementer(problem)
            if not ok:
                print(f"[orchestrator] Implementer failed on problem {problem} iteration {iteration}. Retrying in 10s...")
                time.sleep(10)
                ok = run_implementer(problem)
                if not ok:
                    print(f"[orchestrator] Implementer failed twice. Skipping to next problem.")
                    break

        final_count = count_iterations(problem)
        print(f"\nProblem {problem} complete: {final_count}/{MAX_ITERATIONS} iterations logged.")

    print(f"\n{'='*50}")
    print("All problems complete.")
    for p in PROBLEMS:
        print(f"  Problem {p}: {count_iterations(p)}/{MAX_ITERATIONS} iterations")
    print(f"{'='*50}")


if __name__ == "__main__":
    main()