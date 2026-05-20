# problem_0_reference — iteration log

File: `reference_problem0.cpp` (297 lines from initial commit)

| #  | commit  | description                   |   + |  - | score | kept |
|----|---------|-------------------------------|----:|---:|------:|:----:|
| 0  | 85bd192 | initial solution              | 297 |  0 | 89.09 | yes  |
| 1  | 31e35bc | add large-first ordering      |  12 |  0 | 87.82 | no   |
| 2  | 1994768 | ord_large only for S<=1500    |  12 |  0 | 89.08 | no   |
| 3  | c401734 | dsum=gap-only                 |   1 |  0 | 89.66 | yes  |
| 4  | 8edc4f2 | sqrt(S) W candidates          |  10 |  1 | 88.33 | no   |
| 5  | baf92bd | prefer larger k in tie        |   7 |  4 | 89.76 | yes  |
| 6  | c07cb4c | move k pref after dsum        |   6 |  6 | 89.66 | no   |
| 7  | 0766fb6 | increase W span               |   5 |  5 | 89.66 | no   |
| 8  | 5dc8c89 | large-first ord4              |   5 |  5 | 88.66 | no   |
| 9  | d8b6148 | relax time margin 1.05        |   1 |  1 | 89.72 | no   |
| 10 | 70b2e5d | prefer smaller bb after k tie |   9 |  4 | 88.27 | no   |
| 11 | 813fa80 | widen lookahead n/2           |   1 |  1 | 87.67 | no   |
| 12 | 51c0564 | prefer wider after k tie      |   8 |  4 | 89.73 | no   |
| 13 | 5efaeba | ord4 id ASC tiebreaker        |   1 |  1 | 88.34 | no   |
| 14 | 1ad6070 | factor 0.30 for S<10000       |   1 |  1 | 89.75 | no   |

## Analysis

Score trajectory of kept commits: 89.09 → 89.66 → 89.76, then 10 discards in a row.

Comparison to problem_0 (solutions_0.cpp):

- Iteration size is much more conservative here — every change after the initial commit is ≤12 lines (~4% of file), versus 35–40-line rewrites in solutions_0.
- Starting point is already strong (89.09), so the runway for improvement is narrow — total gain across all kept commits is only +0.67 (89.09 → 89.76).
- Several discarded attempts came VERY close (89.72, 89.73, 89.75) — the optimizer is hovering at a local plateau where 1–2 line parameter tweaks aren't enough to break through (similar to the 77.47 plateau on solutions_0).
- Iteration style is "knob-twiddling" — no attempt at a structural change like the skyline rewrite or multi-ordering search that produced the big jumps on solutions_0.
