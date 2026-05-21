# problem_1_reference — iteration log

File: `reference_problem1.cpp` (188 lines)

| #  | commit  | description                          |   + |  - | score | kept |
|----|---------|--------------------------------------|----:|---:|------:|:----:|
| 0  | 215a7f5 | initial solution                     | 189 |  0 |   100 | yes  |
| 1  | f2f14ee | add 3-opt triple optimization        |  52 |  0 |   100 | no   |
| 2  | 5061552 | residual fill after 2-opt            |  17 |  0 |   100 | no   |
| 3  | 29d0e72 | systematic deterministic 2-opt       |  23 | 20 |   100 | no   |
| 4  | 828dbcb | final 3-opt pass on best solution    |  38 |  0 |   100 | no   |
| 5  | 61a51f6 | SA kick-and-reoptimize loop          |  50 |  0 |   100 | no   |
| 6  | a96501b | deterministic seeds with density mix |  20 |  0 |   100 | no   |
| 7  | d8c9ab0 | density-biased random shuffle        |  14 |  1 |   100 | no   |
| 8  | 25ed743 | deduplicate pool candidates          |   8 |  0 |   100 | no   |
| 9  | e56958d | adaptive generation termination      |  12 |  4 |   100 | no   |
| 10 | 7c5de25 | inline 3-opt per pool iteration      |  28 |  0 |   100 | no   |
| 11 | 008665a | fixed capacity weights for density   |   2 |  2 |   100 | no   |
| 12 | 4ca48e7 | drop-one-refill polish               |  43 |  0 |   100 | no   |
| 13 | 54a5e3b | weighted density seeded make_cand    |  10 | 15 |   100 | no   |
| 14 | 9e0e6ef | single-item enumeration polish       |  56 |  0 |   100 | no   |

## Analysis

Score trajectory: 100 (initial) → 100 (every attempt). 14 attempts, 0 improvements.

This is a ceiling case — the initial solution already hits the maximum score of 100, so no follow-up could improve it. Iterations are small-to-medium (8–56 lines, never above ~30% of file). Every attempt is followed by an explicit revert (classic problem_0-style discipline), in contrast to the non-reference problem_1 run which had no reverts.
