# problem_0 — iteration log

File: `solutions_0.cpp` (currently 224 lines)

| #  | commit  | description           |   + |  - | score | kept |
|----|---------|-----------------------|----:|---:|------:|:----:|
| 0  | 9e08a68 | initial BL fill       | 121 |  0 | 41.08 | yes  |
| 1  | 61df541 | skyline-based BL fill |  35 | 21 | 70.37 | yes  |
| 2  | 8683d88 | monomino backfill     |  25 |  1 | 70.37 | no   |
| 3  | a041f9c | sort by max(w h)      |   3 |  0 | 70.41 | yes  |
| 4  | 2849791 | downward Y scan       |  13 |  8 | 69.32 | no   |
| 5  | ee9ed33 | multi-ordering search |  40 | 13 | 69.71 | no   |
| 6  | d7980b9 | sort by bbox area     |   3 |  0 | 69.74 | no   |
| 7  | f4a0741 | prefer flatter orient |   4 |  0 | 70.69 | yes  |
| 8  | 4d0cd15 | waste tiebreak        |  11 |  3 | 76.29 | yes  |
| 9  | 39a3ec7 | sort k desc           |   2 |  2 | 75.94 | no   |
| 10 | e27d7be | post-skyline max      |  13 |  5 | 77.47 | yes  |
| 11 | aa40d09 | monomino backfill     |  25 |  1 | 77.47 | no   |
| 12 | db945fe | bbox square criterion |  10 |  2 | 42.40 | no   |
| 13 | 694cc17 | shrink output         |   7 |  1 | 77.47 | no   |
| 14 | 99aca77 | single-step tuck      |  13 |  0 | 77.47 | no   |

## Analysis

Score trajectory of kept commits: 41.08 → 70.37 → 70.41 → 70.69 → 76.29 → 77.47. After hitting 77.47, five attempts in a row failed to improve it.

Observation on size vs. gain:

- Two biggest jumps came from a LARGE rewrite (skyline, +29 pts) and a MEDIUM scoring-criterion change (waste tiebreak, +5.6 pts; post-skyline max, +1.2 pts).
- Small tweaks (+3/+4 lines) gave only fractional gains (~0.04 / ~0.28).
- Once the score plateaued at 77.47, further small AND medium attempts (including 25-line aa40d09 and 10-line db945fe) all failed.
- Suggests the current local optimum needs a structural change rather than another parameter tweak.
