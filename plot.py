import csv
import os
import sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

COLORS = {
    "normal_dot":  "#2980b9",
    "advisor_dot": "#27ae60",
    "crash":       "#e67e22",
    "normal_best": "#2980b9",
    "advisor_best":"#27ae60",
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
            status = row.get("status", "").strip().lower()
            rows.append({
                "iteration": i + 1,
                "score": score,
                "status": status,
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

def plot_comparison(normal_path, advisor_path, problem_num, out_path):
    normal_rows  = load_tsv(normal_path)
    advisor_rows = load_tsv(advisor_path)

    fig, ax = plt.subplots(figsize=(11, 5.5))
    fig.patch.set_facecolor("white")
    ax.set_facecolor("white")

    def plot_series(rows, dot_color, best_color, label_prefix):
        iterations = [r["iteration"] for r in rows]
        scores     = [r["score"]     for r in rows]
        statuses   = [r["status"]    for r in rows]
        bests      = running_best(rows)

        ax.plot(iterations, bests, color=best_color, linewidth=2,
                label=f"{label_prefix} best", zorder=2)

        for it, sc, st in zip(iterations, scores, statuses):
            if sc is None:
                continue
            if st == "crash":
                ax.scatter(it, sc, color=COLORS["crash"], marker="x",
                           s=90, zorder=4, linewidths=1.8)
            else:
                ax.scatter(it, sc, color=dot_color, s=60, zorder=3,
                           linewidths=0, alpha=0.85)

    plot_series(normal_rows,  COLORS["normal_dot"],  COLORS["normal_best"],  "normal")
    plot_series(advisor_rows, COLORS["advisor_dot"], COLORS["advisor_best"], "advisor")

    # Best score annotations
    normal_scores  = [r["score"] for r in normal_rows  if r["score"] is not None]
    advisor_scores = [r["score"] for r in advisor_rows if r["score"] is not None]
    normal_best  = max(normal_scores)  if normal_scores  else 0
    advisor_best = max(advisor_scores) if advisor_scores else 0

    ax.text(0.01, 0.97,
            f"Normal best:  {normal_best:.4f}\nAdvisor best: {advisor_best:.4f}",
            transform=ax.transAxes, fontsize=10, fontweight="bold",
            va="top", ha="left",
            bbox=dict(boxstyle="round,pad=0.4", facecolor="white",
                      edgecolor="#555555", linewidth=1.2, alpha=0.85))

    # Legend
    legend_handles = [
        mpatches.Patch(color=COLORS["normal_dot"],  label="normal"),
        mpatches.Patch(color=COLORS["advisor_dot"], label="advisor"),
        plt.Line2D([0],[0], color=COLORS["normal_best"],  linewidth=2, label="normal best"),
        plt.Line2D([0],[0], color=COLORS["advisor_best"], linewidth=2, label="advisor best"),
        plt.Line2D([0],[0], marker="x", color=COLORS["crash"], linewidth=0,
                   markersize=8, markeredgewidth=1.8, label="crash"),
    ]
    ax.legend(handles=legend_handles, fontsize=9,
              loc="upper left", bbox_to_anchor=(1.01, 1), borderaxespad=0,
              framealpha=0.95, edgecolor="#aaaaaa")

    max_iter = max(
        max(r["iteration"] for r in normal_rows),
        max(r["iteration"] for r in advisor_rows)
    )
    ax.set_xlim(0.5, max_iter + 0.5)
    ax.set_xticks(range(1, max_iter + 1))

    ax.set_title(f"Frontier-CS Problem {problem_num} — Normal vs Advisor",
                 fontsize=13, fontweight="bold", pad=10)
    ax.set_xlabel("Iteration", fontsize=11)
    ax.set_ylabel("Score (unbounded)", fontsize=11)
    ax.grid(True, linestyle="--", linewidth=0.5, color="#cccccc", alpha=0.8)

    plt.tight_layout(pad=2.5)
    plt.savefig(out_path, dpi=150, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    print(f"Saved -> {out_path}")

if __name__ == "__main__":
    import sys

    problems = [int(x) for x in sys.argv[1:]] if len(sys.argv) > 1 else [0, 2, 3, 4, 7]

    for i in problems:
        normal_path  = f"runs/normal/logs/results_{i}.tsv"
        advisor_path = f"split_results_{i}.tsv"
        out_path     = f"problem{i}_normal_vs_advisor.png"

        if not os.path.exists(normal_path):
            print(f"Skipping problem {i}: missing {normal_path}")
            continue
        if not os.path.exists(advisor_path):
            print(f"Skipping problem {i}: missing {advisor_path}")
            continue

        plot_comparison(normal_path, advisor_path, problem_num=i, out_path=out_path)