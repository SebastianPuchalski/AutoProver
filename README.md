# AutoProver

### Automated Theorem Proving tool for Propositional Calculus

**Work in progress**


At the moment, the project consists of:
- Formulas parser (including operators precedence) and printer
- Naive Model Checking
- DPLL (CNF SAT) algorithm based Model Checking
- Very efficient implementation of the Resolution method with proof generation
- Forward Chaining based on Horn clauses in the form of an updatable knowledge base
- Implementation of the Natural Deduction alike method with proof generation

Natural deduction example:
```
1. p & s                  Premise
2. p                      OK 1
3. s                      OK 1
4. (s & r) -> t           Premise
5. (p -> q) & (q -> r)    Premise
6. p -> q                 OK 5
7. q                      RO 6, 2
8. q -> r                 OK 5
9. r                      RO 8, 7
10. s & r                 DK 3, 9
11. t                     RO 4, 10
```

Resolution example 1:
```
((p & (q | r)) <-> ((p & q) | (p & r))) & ((q & (p | r)) <-> ((q & p) | (q & r))) & ((r & (p | q)) <-> ((r & p) | (r & q)))
Model checking: valid
Resolution: valid

Proof by refutation:
0. ~((((p & (q | r)) <-> ((p & q) | (p & r))) & ((q & (p | r)) <-> ((q & p) | (q & r)))) & ((r & (p | q)) <-> ((r & p) | (r & q))))
1. (~p | ~q | ~r)                                 [cnf transformation 0]
2. (p | ~q | ~r)                                  [cnf transformation 0]
3. (~p | ~q | ~r) & (p | ~q | ~r) -> (~q | ~r)    [resolution 1, 2]
4. (~p | q | ~r)                                  [cnf transformation 0]
5. (~q | ~r) & (~p | q | ~r) -> (~p | ~r)         [resolution 3, 4]
6. (p | q)                                        [cnf transformation 0]
7. (~q | ~r) & (p | q) -> (p | ~r)                [resolution 3, 6]
8. (~p | ~r) & (p | ~r) -> (~r)                   [resolution 5, 7]
9. (~p | ~q | r)                                  [cnf transformation 0]
10. (~r) & (~p | ~q | r) -> (~p | ~q)             [resolution 8, 9]
11. (p | r)                                       [cnf transformation 0]
12. (~r) & (p | r) -> (p)                         [resolution 8, 11]
13. (~p | ~q) & (p) -> (~q)                       [resolution 10, 12]
14. (q | r)                                       [cnf transformation 0]
15. (~r) & (q | r) -> (q)                         [resolution 8, 14]
16. (~q) & (q) -> (F)                             [resolution 13, 15]
Refutation found
Elapsed time: 0.000021s
```

Resolution example2:
```
((a & ~b) | c) <-> (d -> (e & f)) <-> ((a & ~b) | c) <-> (d -> (e & f))
Model checking: valid
Resolution: valid

Proof by refutation:
0. ~(((((a & ~b) | c) <-> (d -> (e & f))) <-> ((a & ~b) | c)) <-> (d -> (e & f)))
1. (~a | b | d)                                              [cnf transformation 0]
2. (~c | d)                                                  [cnf transformation 0]
3. (~b | c | d)                                              [cnf transformation 0]
4. (~c | d) & (~b | c | d) -> (~b | d)                       [resolution 2, 3]
5. (~a | b | d) & (~b | d) -> (~a | d)                       [resolution 1, 4]
6. (a | c | d)                                               [cnf transformation 0]
7. (~c | d) & (a | c | d) -> (a | d)                         [resolution 2, 6]
8. (~a | d) & (a | d) -> (d)                                 [resolution 5, 7]
9. (~a | b | ~e | ~f)                                        [cnf transformation 0]
10. (~c | ~e | ~f)                                           [cnf transformation 0]
11. (~b | c | ~e | ~f)                                       [cnf transformation 0]
12. (~c | ~e | ~f) & (~b | c | ~e | ~f) -> (~b | ~e | ~f)    [resolution 10, 11]
13. (~a | b | ~e | ~f) & (~b | ~e | ~f) -> (~a | ~e | ~f)    [resolution 9, 12]
14. (a | c | ~e | ~f)                                        [cnf transformation 0]
15. (~c | ~e | ~f) & (a | c | ~e | ~f) -> (a | ~e | ~f)      [resolution 10, 14]
16. (~a | ~e | ~f) & (a | ~e | ~f) -> (~e | ~f)              [resolution 13, 15]
17. (~a | b | ~d | e)                                        [cnf transformation 0]
18. (~e | ~f) & (~a | b | ~d | e) -> (~a | b | ~d | ~f)      [resolution 16, 17]
19. (d) & (~a | b | ~d | ~f) -> (~a | b | ~f)                [resolution 8, 18]
20. (~c | ~d | e)                                            [cnf transformation 0]
21. (~e | ~f) & (~c | ~d | e) -> (~c | ~d | ~f)              [resolution 16, 20]
22. (d) & (~c | ~d | ~f) -> (~c | ~f)                        [resolution 8, 21]
23. (~b | c | ~d | e)                                        [cnf transformation 0]
24. (~e | ~f) & (~b | c | ~d | e) -> (~b | c | ~d | ~f)      [resolution 16, 23]
25. (~c | ~f) & (~b | c | ~d | ~f) -> (~b | ~d | ~f)         [resolution 22, 24]
26. (d) & (~b | ~d | ~f) -> (~b | ~f)                        [resolution 8, 25]
27. (~a | b | ~f) & (~b | ~f) -> (~a | ~f)                   [resolution 19, 26]
28. (a | c | ~d | e)                                         [cnf transformation 0]
29. (~e | ~f) & (a | c | ~d | e) -> (a | c | ~d | ~f)        [resolution 16, 28]
30. (~c | ~f) & (a | c | ~d | ~f) -> (a | ~d | ~f)           [resolution 22, 29]
31. (~a | ~f) & (a | ~d | ~f) -> (~d | ~f)                   [resolution 27, 30]
32. (d) & (~d | ~f) -> (~f)                                  [resolution 8, 31]
33. (~a | b | ~d | f)                                        [cnf transformation 0]
34. (~f) & (~a | b | ~d | f) -> (~a | b | ~d)                [resolution 32, 33]
35. (d) & (~a | b | ~d) -> (~a | b)                          [resolution 8, 34]
36. (~c | ~d | f)                                            [cnf transformation 0]
37. (~f) & (~c | ~d | f) -> (~c | ~d)                        [resolution 32, 36]
38. (d) & (~c | ~d) -> (~c)                                  [resolution 8, 37]
39. (~b | c | ~d | f)                                        [cnf transformation 0]
40. (~f) & (~b | c | ~d | f) -> (~b | c | ~d)                [resolution 32, 39]
41. (~c) & (~b | c | ~d) -> (~b | ~d)                        [resolution 38, 40]
42. (d) & (~b | ~d) -> (~b)                                  [resolution 8, 41]
43. (~a | b) & (~b) -> (~a)                                  [resolution 35, 42]
44. (a | c | ~d | f)                                         [cnf transformation 0]
45. (~f) & (a | c | ~d | f) -> (a | c | ~d)                  [resolution 32, 44]
46. (~c) & (a | c | ~d) -> (a | ~d)                          [resolution 38, 45]
47. (~a) & (a | ~d) -> (~d)                                  [resolution 43, 46]
48. (d) & (~d) -> (F)                                        [resolution 8, 47]
Refutation found
Elapsed time: 0.000121s
```

Resolution example 3:
```
((a | b | c) & (d | e | f) & (g | h | i) & (j | k | l)) -> ((a & d & g & j) | (a & d & g & k) | (a & d & g & l) | (a & d & h & j) | (a & d & h & k) | (a & d & h & l) | (a & d & i & j) | (a & d & i & k) | (a & d & i & l) | (a & e & g & j) | (a & e & g & k) | (a & e & g & l) | (a & e & h & j) | (a & e & h & k) | (a & e & h & l) | (a & e & i & j) | (a & e & i & k) | (a & e & i & l) | (a & f & g & j) | (a & f & g & k) | (a & f & g & l) | (a & f & h & j) | (a & f & h & k) | (a & f & h & l) | (a & f & i & j) | (a & f & i & k) | (a & f & i & l) | (b & d & g & j) | (b & d & g & k) | (b & d & g & l) | (b & d & h & j) | (b & d & h & k) | (b & d & h & l) | (b & d & i & j) | (b & d & i & k) | (b & d & i & l) | (b & e & g & j) | (b & e & g & k) | (b & e & g & l) | (b & e & h & j) | (b & e & h & k) | (b & e & h & l) | (b & e & i & j) | (b & e & i & k) | (b & e & i & l) | (b & f & g & j) | (b & f & g & k) | (b & f & g & l) | (b & f & h & j) | (b & f & h & k) | (b & f & h & l) | (b & f & i & j) | (b & f & i & k) | (b & f & i & l) | (c & d & g & j) | (c & d & g & k) | (c & d & g & l) | (c & d & h & j) | (c & d & h & k) | (c & d & h & l) | (c & d & i & j) | (c & d & i & k) | (c & d & i & l) | (c & e & g & j) | (c & e & g & k) | (c & e & g & l) | (c & e & h & j) | (c & e & h & k) | (c & e & h & l) | (c & e & i & j) | (c & e & i & k) | (c & e & i & l) | (c & f & g & j) | (c & f & g & k) | (c & f & g & l) | (c & f & h & j) | (c & f & h & k) | (c & f & h & l) | (c & f & i & j) | (c & f & i & k) | (c & f & i & l))
Model checking: valid
Resolution: valid

Proof by refutation:
0. ~((((((a | b) | c) & ((d | e) | f)) & ((g | h) | i)) & ((j | k) | l)) -> (((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((a & d) & g) & j) | (((a & d) & g) & k)) | (((a & d) & g) & l)) | (((a & d) & h) & j)) | (((a & d) & h) & k)) | (((a & d) & h) & l)) | (((a & d) & i) & j)) | (((a & d) & i) & k)) | (((a & d) & i) & l)) | (((a & e) & g) & j)) | (((a & e) & g) & k)) | (((a & e) & g) & l)) | (((a & e) & h) & j)) | (((a & e) & h) & k)) | (((a & e) & h) & l)) | (((a & e) & i) & j)) | (((a & e) & i) & k)) | (((a & e) & i) & l)) | (((a & f) & g) & j)) | (((a & f) & g) & k)) | (((a & f) & g) & l)) | (((a & f) & h) & j)) | (((a & f) & h) & k)) | (((a & f) & h) & l)) | (((a & f) & i) & j)) | (((a & f) & i) & k)) | (((a & f) & i) & l)) | (((b & d) & g) & j)) | (((b & d) & g) & k)) | (((b & d) & g) & l)) | (((b & d) & h) & j)) | (((b & d) & h) & k)) | (((b & d) & h) & l)) | (((b & d) & i) & j)) | (((b & d) & i) & k)) | (((b & d) & i) & l)) | (((b & e) & g) & j)) | (((b & e) & g) & k)) | (((b & e) & g) & l)) | (((b & e) & h) & j)) | (((b & e) & h) & k)) | (((b & e) & h) & l)) | (((b & e) & i) & j)) | (((b & e) & i) & k)) | (((b & e) & i) & l)) | (((b & f) & g) & j)) | (((b & f) & g) & k)) | (((b & f) & g) & l)) | (((b & f) & h) & j)) | (((b & f) & h) & k)) | (((b & f) & h) & l)) | (((b & f) & i) & j)) | (((b & f) & i) & k)) | (((b & f) & i) & l)) | (((c & d) & g) & j)) | (((c & d) & g) & k)) | (((c & d) & g) & l)) | (((c & d) & h) & j)) | (((c & d) & h) & k)) | (((c & d) & h) & l)) | (((c & d) & i) & j)) | (((c & d) & i) & k)) | (((c & d) & i) & l)) | (((c & e) & g) & j)) | (((c & e) & g) & k)) | (((c & e) & g) & l)) | (((c & e) & h) & j)) | (((c & e) & h) & k)) | (((c & e) & h) & l)) | (((c & e) & i) & j)) | (((c & e) & i) & k)) | (((c & e) & i) & l)) | (((c & f) & g) & j)) | (((c & f) & g) & k)) | (((c & f) & g) & l)) | (((c & f) & h) & j)) | (((c & f) & h) & k)) | (((c & f) & h) & l)) | (((c & f) & i) & j)) | (((c & f) & i) & k)) | (((c & f) & i) & l)))
1. (~a | ~d | ~g | ~j)                                                     [cnf transformation 0]
2. (~b | ~d | ~g | ~j)                                                     [cnf transformation 0]
3. (~c | ~d | ~g | ~j)                                                     [cnf transformation 0]
4. (a | b | c)                                                             [cnf transformation 0]
5. (~c | ~d | ~g | ~j) & (a | b | c) -> (a | b | ~d | ~g | ~j)             [resolution 3, 4]
6. (~b | ~d | ~g | ~j) & (a | b | ~d | ~g | ~j) -> (a | ~d | ~g | ~j)      [resolution 2, 5]
7. (~a | ~d | ~g | ~j) & (a | ~d | ~g | ~j) -> (~d | ~g | ~j)              [resolution 1, 6]
8. (~a | ~e | ~g | ~j)                                                     [cnf transformation 0]
9. (~b | ~e | ~g | ~j)                                                     [cnf transformation 0]
10. (~c | ~e | ~g | ~j)                                                    [cnf transformation 0]
11. (~c | ~e | ~g | ~j) & (a | b | c) -> (a | b | ~e | ~g | ~j)            [resolution 10, 4]
...
198. (~d | ~i | ~l) & (d | ~i | ~l) -> (~i | ~l)                           [resolution 183, 197]
199. (~i | ~l) & (g | h | i) -> (g | h | ~l)                               [resolution 198, 66]
200. (~h | ~l) & (g | h | ~l) -> (g | ~l)                                  [resolution 177, 199]
201. (~g | ~l) & (g | ~l) -> (~l)                                          [resolution 156, 200]
202. (j | k | l)                                                           [cnf transformation 0]
203. (~l) & (j | k | l) -> (j | k)                                         [resolution 201, 202]
204. (~k) & (j | k) -> (j)                                                 [resolution 135, 203]
205. (~j) & (j) -> (F)                                                     [resolution 69, 204]
Refutation found
Elapsed time: 0.000402s
```
