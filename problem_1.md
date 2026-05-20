# problem_1 — iteration log

File: `solutions_1.cpp` (currently 206 lines)

| #  | commit  | description                              |   + |   - |     score | kept |
|----|---------|------------------------------------------|----:|----:|----------:|:----:|
| 0  | 54bc624 | initial solution                         | 163 |   0 | 99.233333 | yes  |
| 1  | adee129 | multi-start local search                 |  19 |   8 | 99.233333 | no   |
| 2  | 760aa2c | fill-remaining after swap                |  28 |   1 | 99.233333 | no   |
| 3  | cbd47b9 | 3-way swap moves                         | 117 |   1 | 99.233333 | no   |
| 4  | 1d16bd4 | LP-vertex structure enumeration          |  96 |   0 | 99.613333 | yes  |
| 5  | 40bcd88 | add 3-way swap to local search           |  63 |   1 | 99.613333 | no   |
| 6  | e2c5105 | 3-fractional LP enumeration              |  52 |   0 | 99.613333 | no   |
| 7  | bb3c110 | gauss-seidel pair optimization           |  18 |  29 |       100 | yes  |
| 8  | 8b057ee | remove redundant multi-weight greedy init |   0 |  11 |       100 | yes  |
| 9  | f36effb | remove local_search after lp_enumerate   |   0 |   4 | 98.746667 | no   |
| 10 | 7e1b5f2 | remove dead greedy code                  |   0 |  25 |       100 | yes  |
| 11 | 6748502 | remove time-limit safety net             |   3 |   9 |       100 | yes  |
| 12 | 8cff99d | simplify best_ab LP intersection check   |   3 |   9 | 98.986667 | no   |
| 13 | 8cdbeb7 | single-pass swap_improve                 |   1 |   1 |       100 | yes  |
| 14 | b66f9b4 | remove redundant Sm/Sl guards in best_ab |   2 |   2 |       100 | yes  |

## Analysis

Score trajectory of kept commits: 99.233 → 99.613 → 100 → 100 → 100 → 100 → 100 → 100.

Iteration pattern now matches problem_0 discipline: every discard is followed by an explicit revert. (This is a re-run — git note "redoing run for solutions_1.cpp, before was incorrectly reverting" — and the new pattern lines up with the reference: tweak → measure → revert on failure.)

Where the gains came from:

- The +0.38 step (99.233 → 99.613) came from a LARGE structural rewrite (1d16bd4, +96 lines, LP-vertex structure enumeration).
- The +0.39 step (99.613 → 100) came from a MEDIUM, focused change (bb3c110, +18/-29, gauss-seidel pair optimization).
- After hitting 100, every accepted commit is a REMOVAL (-11, -25, +3/-9, +1/-1, +2/-2) — score holds because the deleted code was redundant. Two attempts at simplification overshot (f36effb, 8cff99d) and got reverted.

Size vs. outcome:

- The huge speculative rewrite (cbd47b9, +117/-1, 72% of file) gave nothing and was reverted.
- The two large/medium structural changes that DID help (1d16bd4, bb3c110) were both motivated by a specific algorithmic idea (LP geometry, pairwise re-optimization), not blind expansion.
- All post-ceiling improvements are tiny/small cleanups — once at 100, gains stop and only code-size shrinks.

Comparison to the previous problem_1 run (now superseded):

- Old run: no reverts, layered huge rewrites, ceiling at 99.743.
- New run: clean revert discipline, fewer total commits, ceiling at 100.
- Confirms the reference-style iterate/revert workflow beats keep-everything-and-rewrite for this problem.
