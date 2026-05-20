import sys
import os
import csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

FIGSIZE_PER_PLOT = (10, 5.5)

COLORS = {
    "keep":    "#2ecc71",
    "discard": "#e74c3c",
    "crash":   "#e67e22",
    "best":    "#2980b9",
}

def load_tsv(path):
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f, delimiter="\t")
        for i, row in enumerate(reader):
            try:
                score = float(row["best_unbounded"])
            except (ValueError, KeyError):
                score = None
            rows.append({
                "iteration": i + 1,
                "commit":      row.get("commit", ""),
                "score":       score,
                "status":      row.get("status", "keep").strip().lower(),
                "description": row.get("description", ""),
            })
    return rows

def running_best(rows):
    best = None
    bests = []
    for r in rows:
        s = r["score"]
        if s is not None and (best is None or s > best):
            best = s
        bests.append(best)
    return bests

def problem_number_from_name(name):
    # e.g. "results_0" -> "0"
    parts = name.rsplit("_", 1)
    return parts[-1] if len(parts) == 2 else name

def plot_one(ax, rows, name):
    problem_num = problem_number_from_name(name)

    iterations = [r["iteration"] for r in rows]
    scores     = [r["score"]     for r in rows]
    statuses   = [r["status"]    for r in rows]
    bests      = running_best(rows)
    valid      = [s for s in scores if s is not None]

    if valid:
        best_val = max(valid)
        title = f"Frontier-CS Algorithmic Problem {problem_num} — Autoresearch Opus 4.7 Starting Point"
    else:
        title = f"Frontier-CS Algorithmic Problem {problem_num} — Autoresearch Opus 4.7 Starting Point"

    # Best-so-far line
    ax.plot(iterations, bests, color=COLORS["best"], linewidth=2,
            label="best score", zorder=2)

    # Scatter dots
    for it, sc, st in zip(iterations, scores, statuses):
        if sc is None:
            continue
        color = COLORS.get(st, COLORS["discard"])
        ax.scatter(it, sc, color=color, s=70, zorder=3, linewidths=0)


    ax.set_title(title, fontsize=13, fontweight="bold", pad=10)
    ax.set_xlabel("Iteration", fontsize=11)
    ax.set_ylabel("Score (bounded)", fontsize=11)
    ax.grid(True, linestyle="--", linewidth=0.5, color="#cccccc", alpha=0.8)
    ax.set_facecolor("white")

    # Always show 0-15 on x-axis
    ax.set_xlim(0.5, 15.5)
    ax.set_xticks(range(1, 16))
    ax.set_ylim(0, 105)

    # Legend outside plot area (top-right, outside)
    legend_handles = [
        mpatches.Patch(color=COLORS["keep"],    label="keep"),
        mpatches.Patch(color=COLORS["discard"], label="discard"),
        plt.Line2D([0], [0], color=COLORS["best"], linewidth=2, label="best score"),
    ]
    ax.legend(handles=legend_handles, fontsize=9,
              loc="upper left", bbox_to_anchor=(1.01, 1), borderaxespad=0,
              framealpha=0.95, edgecolor="#aaaaaa")

    if valid:
        best_val = max(valid)
        # If most data is in the top 30% of the chart, put label bottom-left
        high_points = sum(1 for s in valid if s > 70)
        if high_points > len(valid) * 0.6:
            y_pos, va = 0.03, "bottom"
        else:
            y_pos, va = 0.97, "top"
        ax.text(0.01, y_pos, f"Best: {best_val:.4f}",
                transform=ax.transAxes, fontsize=11, fontweight="bold",
                va=va, ha="left",
                bbox=dict(boxstyle="round,pad=0.35", facecolor="#d6eaf8",
                          edgecolor="#2980b9", linewidth=1.5, alpha=0.6))

def main():
    paths = sys.argv[1:]
    if not paths:
        paths = ["results_0.tsv", "results_1.tsv"]

    datasets = []
    for p in paths:
        if not os.path.exists(p):
            print(f"Skipping missing file: {p}")
            continue
        rows = load_tsv(p)
        name = os.path.splitext(os.path.basename(p))[0]
        datasets.append((name, rows))

    for name, rows in datasets:
        fig, ax = plt.subplots(figsize=FIGSIZE_PER_PLOT)
        fig.patch.set_facecolor("white")
        plot_one(ax, rows, name)
        plt.tight_layout(pad=2.5)
        out = f"{name}.png"
        plt.savefig(out, dpi=150, bbox_inches="tight", facecolor="white")
        print(f"Saved -> {out}")
        plt.close(fig)

if __name__ == "__main__":
    main()