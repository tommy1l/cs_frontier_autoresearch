#!/usr/bin/env python3
"""
Implementer for Frontier-CS algorithmic autoresearch.

Handles all mechanical steps (git, eval, log) and calls the Anthropic API
only to write C++ code from the proposer's details.

Usage:
    python agent.py --problem 3   # implement next iteration for problem 3
"""

import argparse
import os
import subprocess
import sys
import time
import anthropic

FRONTIER_CS_DIR = "Frontier-CS"
MAX_ITERATIONS = 15


def run(cmd: str) -> tuple[int, str, str]:
    """Run a shell command, return (returncode, stdout, stderr)."""
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.returncode, result.stdout.strip(), result.stderr.strip()


def count_iterations(problem: int) -> int:
    tsv = f"split_results_{problem}.tsv"
    if not os.path.exists(tsv):
        return 0
    with open(tsv) as f:
        lines = [l for l in f.readlines()[1:] if l.strip()]
    return len(lines)


def read_proposal(problem: int, iteration: int) -> dict | None:
    tsv = f"split_problem_{problem}_proposals.tsv"
    if not os.path.exists(tsv):
        return None
    with open(tsv) as f:
        lines = f.readlines()
    if not lines:
        return None
    headers = lines[0].strip().split("\t")
    for line in lines[1:]:
        parts = line.strip().split("\t")
        if not parts:
            continue
        row = dict(zip(headers, parts))
        if row.get("iteration") == str(iteration):
            return row
    return None


def read_current_solution(problem: int) -> str:
    path = f"split_solutions_{problem}.cpp"
    if not os.path.exists(path):
        return ""
    with open(path) as f:
        return f.read()


def write_solution(problem: int, code: str) -> None:
    with open(f"split_solutions_{problem}.cpp", "w") as f:
        f.write(code)


def revert_to_commit(problem: int, commit: str) -> bool:
    """
    Restore split_solutions_{problem}.cpp from the given commit.
    Returns True on success, False on failure (caller continues without reverting).
    """
    print(f"  Reverting split_solutions_{problem}.cpp to commit {commit}...")
    path = f"split_solutions_{problem}.cpp"
    show = subprocess.run(
        ["git", "show", f"{commit}:{path}"], capture_output=True, text=True
    )
    if show.returncode != 0:
        print(
            f"  WARNING: could not revert to {commit!r} "
            f"(git show stderr: {show.stderr.strip()}). "
            f"Continuing from current solution instead."
        )
        return False
    with open(path, "w") as f:
        f.write(show.stdout)
    return True


def evaluate(problem: int) -> float:
    cmd = (
        f'uv --directory {FRONTIER_CS_DIR} run python3 -c "'
        f'from frontier_cs import SingleEvaluator; '
        f'evaluator = SingleEvaluator(); '
        f'result = evaluator.evaluate(\'algorithmic\', problem_id={problem}, '
        f'code=open(\'../split_solutions_{problem}.cpp\').read()); '
        f'print(f\'Score (unbounded): {{result.score_unbounded}}\')"'
    )
    rc, out, err = run(cmd)
    for line in out.splitlines():
        if line.startswith("Score (unbounded):"):
            return float(line.split(":")[1].strip())
    print(f"  [evaluate] No score line found (rc={rc}).")
    if out:
        print(f"  [evaluate] stdout: {out}")
    if err:
        print(f"  [evaluate] stderr: {err}")
    return 0.0


def commit_and_get_hash(problem: int, score: float, approach: str) -> str:
    subprocess.run(["git", "add", f"split_solutions_{problem}.cpp"], check=True)
    msg = f"p{problem} score={score:.4f} {approach[:50]}"
    subprocess.run(["git", "commit", "-m", msg], check=True)

    push = subprocess.run(["git", "push", "origin", "HEAD"], capture_output=True, text=True)
    if push.returncode != 0:
        print(f"  git push failed: {push.stderr.strip()}. Retrying in 5s...")
        time.sleep(5)
        push = subprocess.run(["git", "push", "origin", "HEAD"], capture_output=True, text=True)
        if push.returncode != 0:
            raise RuntimeError(
                f"git push failed twice; refusing to log an unpushed commit. "
                f"stderr: {push.stderr.strip()}"
            )

    rev = subprocess.run(
        ["git", "rev-parse", "--short", "HEAD"], capture_output=True, text=True, check=True
    )
    return rev.stdout.strip()


def log_result(problem: int, commit: str, score: float, status: str, approach: str) -> None:
    tsv = f"split_results_{problem}.tsv"
    if not os.path.exists(tsv):
        with open(tsv, "w") as f:
            f.write("commit\tbest_unbounded\tstatus\tdescription\n")
    with open(tsv, "a") as f:
        f.write(f"{commit}\t{score}\t{status}\t{approach}\n")


def generate_cpp(problem: int, approach: str, details: str, current_solution: str) -> str:
    """Call Anthropic API to write C++ implementing the proposal."""
    client = anthropic.Anthropic()

    system = (
        "You are an expert competitive programmer. "
        "You write clean, correct C++ solutions. "
        "Output ONLY the raw C++ code with no markdown, no explanation, no backticks."
    )

    user_parts = [
        f"## Approach to implement\n\n{approach}",
        f"## Implementation details\n\n{details}",
    ]
    if current_solution:
        user_parts.append(f"## Current solution (for reference)\n\n{current_solution}")
    user_parts.append(
        "Write a complete C++ solution implementing the approach and details above. "
        "Output only the raw C++ code."
    )

    message = client.messages.create(
        model="claude-opus-4-7",
        max_tokens=4096,
        system=system,
        messages=[{"role": "user", "content": "\n\n".join(user_parts)}],
    )

    code = message.content[0].text.strip()
    # Strip any accidental markdown fences
    if code.startswith("```"):
        lines = code.splitlines()
        code = "\n".join(lines[1:-1] if lines[-1].strip() == "```" else lines[1:])
    return code


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--problem", type=int, required=True)
    args = parser.parse_args()

    if not os.environ.get("ANTHROPIC_API_KEY"):
        print("Error: ANTHROPIC_API_KEY not set")
        sys.exit(1)

    problem = args.problem

    iteration = count_iterations(problem)
    print(f"Problem {problem} | Iteration {iteration}")

    if iteration >= MAX_ITERATIONS:
        print(f"Problem {problem} already has {iteration} iterations. Done.")
        sys.exit(0)

    # Read proposal
    proposal = read_proposal(problem, iteration)
    if proposal is None:
        print(f"No proposal found for problem {problem} iteration {iteration}. Run proposer first.")
        sys.exit(1)

    approach = proposal.get("approach", "")
    details = proposal.get("details", "")
    revert_to = proposal.get("revert_to", "none").strip()

    print(f"  Approach:   {approach}")
    print(f"  Revert to:  {revert_to}")

    # Handle revert_to (soft-fail: warn and continue if hash is bad)
    if revert_to and revert_to != "none":
        revert_to_commit(problem, revert_to)

    # Read context for code generation
    current_solution = read_current_solution(problem)

    # Generate C++
    print("  Generating C++ solution...")
    code = generate_cpp(problem, approach, details, current_solution)
    write_solution(problem, code)
    print(f"  Written split_solutions_{problem}.cpp ({len(code)} chars)")

    # Evaluate
    print("  Evaluating...")
    score = evaluate(problem)
    print(f"  Score: {score}")

    if score == 0.0:
        print("  Score is 0.0, retrying evaluation...")
        score = evaluate(problem)
        print(f"  Score (retry): {score}")

    status = "crash" if score == 0.0 else "logged"

    # Commit
    print("  Committing...")
    commit_hash = commit_and_get_hash(problem, score, approach)
    print(f"  Commit: {commit_hash}")

    # Log
    log_result(problem, commit_hash, score, status, approach)
    print(f"  Logged to split_results_{problem}.tsv")
    print(f"Done. Problem {problem} iteration {iteration} complete.")


if __name__ == "__main__":
    main()