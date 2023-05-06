# perft

* bulk counting
* no hashing

## Performance

### perft 7

* single core

|       | This           | Stockfish      | QPerft         |
|-------|----------------|----------------|----------------|
| Nodes | 3,195,901,860  | 3,195,901,860  | 3,195,901,860  |
| Time  | 10.5121        | 12.30          | 12.457         |
| Perf  | 304,019,863    | 259,913,944    | 256,554,697    |

### perft 8

* single core

|       | This           | Stockfish      | QPerft         |
|-------|----------------|----------------|----------------|
| Nodes | 84,998,978,956 | 84,998,978,956 | 84,998,978,956 |
| Time  | 279.609        | 323.134        | 333.844        |
| Perf  | 303,992,155    | 263,045,606    | 254,606,879    |

### test suite

* all cores

|       | depth = 5       | depth = 6         |
|-------|-----------------|-------------------|
| Nodes | 131,415,404,805 | 4,160,501,650,185 |
| Time  | 33.2618         | 1063.35           |
| Perf  | 3,950,939,868   | 3,912,629,637     |

### Hardware

AMD Ryzen 9 5900X 12-Core Processor @ 3.70 GHz

## Commands

### This
no cli - compiler and run

### Stockfish
stockfish bench 0 1 8 current perft

### QPerft
perft 8
