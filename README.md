# AutoProver

### Automated Theorem Proving tool for Propositional Calculus

**Work in progress**


At the moment, the project consists of:
- Formulas parser (including operators precedence) and printer
- Naive Model Checking
- Efficient implementation of the Resolution method with proof generation

Example:
```
((a & ~b) | c) <-> (d -> (e & f)) <-> ((a & ~b) | c) <-> (d -> (e & f))
Model checking: valid
Resolution: valid

Proof by refutation:
0. ~(((((a & ~b) | c) <-> (d -> (e & f))) <-> ((a & ~b) | c)) <-> (d -> (e & f)))
1. (a | c | d)                                                      [cnf transformation 0]
2. (~c | d)                                                         [cnf transformation 0]
3. (a | c | d) & (~c | d) -> (a | d)                                [resolution 1, 2]
4. (a | c | ~d | e)                                                 [cnf transformation 0]
5. (~c | ~d | e)                                                    [cnf transformation 0]
6. (a | c | ~d | e) & (~c | ~d | e) -> (a | ~d | e)                 [resolution 4, 5]
7. (a | c | ~d | f)                                                 [cnf transformation 0]
8. (a | c | ~e | ~f)                                                [cnf transformation 0]
9. (a | c | ~d | f) & (a | c | ~e | ~f) -> (a | c | ~d | ~e)        [resolution 7, 8]
10. (~c | ~d | f)                                                   [cnf transformation 0]
11. (~c | ~e | ~f)                                                  [cnf transformation 0]
12. (~c | ~d | f) & (~c | ~e | ~f) -> (~c | ~d | ~e)                [resolution 10, 11]
13. (a | c | ~d | ~e) & (~c | ~d | ~e) -> (a | ~d | ~e)             [resolution 9, 12]
14. (a | ~d | e) & (a | ~d | ~e) -> (a | ~d)                        [resolution 6, 13]
15. (a | d) & (a | ~d) -> (a)                                       [resolution 3, 14]
16. (~a | b | d)                                                    [cnf transformation 0]
17. (~a | b | ~d | e)                                               [cnf transformation 0]
18. (~a | b | ~d | f)                                               [cnf transformation 0]
19. (~a | b | ~e | ~f)                                              [cnf transformation 0]
20. (~a | b | ~d | f) & (~a | b | ~e | ~f) -> (~a | b | ~d | ~e)    [resolution 18, 19]
21. (~a | b | ~d | e) & (~a | b | ~d | ~e) -> (~a | b | ~d)         [resolution 17, 20]
22. (~a | b | d) & (~a | b | ~d) -> (~a | b)                        [resolution 16, 21]
23. (a) & (~a | b) -> (b)                                           [resolution 15, 22]
24. (~b | c | d)                                                    [cnf transformation 0]
25. (~c | d)                                                        [cnf transformation 0]
26. (~b | c | d) & (~c | d) -> (~b | d)                             [resolution 24, 25]
27. (~b | c | ~d | e)                                               [cnf transformation 0]
28. (~c | ~d | e)                                                   [cnf transformation 0]
29. (~b | c | ~d | e) & (~c | ~d | e) -> (~b | ~d | e)              [resolution 27, 28]
30. (~b | c | ~d | f)                                               [cnf transformation 0]
31. (~b | c | ~e | ~f)                                              [cnf transformation 0]
32. (~b | c | ~d | f) & (~b | c | ~e | ~f) -> (~b | c | ~d | ~e)    [resolution 30, 31]
33. (~c | ~d | f)                                                   [cnf transformation 0]
34. (~c | ~e | ~f)                                                  [cnf transformation 0]
35. (~c | ~d | f) & (~c | ~e | ~f) -> (~c | ~d | ~e)                [resolution 33, 34]
36. (~b | c | ~d | ~e) & (~c | ~d | ~e) -> (~b | ~d | ~e)           [resolution 32, 35]
37. (~b | ~d | e) & (~b | ~d | ~e) -> (~b | ~d)                     [resolution 29, 36]
38. (~b | d) & (~b | ~d) -> (~b)                                    [resolution 26, 37]
39. (b) & (~b) -> (F)                                               [resolution 23, 38]
Refutation found
Time elapsed: 0.001s
```
