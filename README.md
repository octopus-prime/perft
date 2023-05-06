# perft

* bulk counting
* no hashing

## Performance

### perft 7

* single core

|       | This           | Stockfish      | QPerft         |
|-------|----------------|----------------|----------------|
| Nodes | 3,195,901,860  | 3,195,901,860  | 3,195,901,860  |
| Time  | 9.36147        | 12.30          | 12.457         |
| Perf  | 341,388,830    | 259,913,944    | 256,554,697    |

### perft 8

* single core

|       | This           | Stockfish      | QPerft         |
|-------|----------------|----------------|----------------|
| Nodes | 84,998,978,956 | 84,998,978,956 | 84,998,978,956 |
| Time  | 247.552        | 323.134        | 333.844        |
| Perf  | 343,358,117    | 263,045,606    | 254,606,879    |

### test suite

* all cores

|       | depth = 5       | depth = 6         |
|-------|-----------------|-------------------|
| Nodes | 131,404,864,960 | 4,160,378,507,849 |
| Time  | 29.785          | 955.195           |
| Perf  | 4,411,786,207   | 4,355,529,660     |

### Hardware

AMD Ryzen 9 5900X 12-Core Processor @ 3.70 GHz

## Commands

### This
no cli - compile and run

### Stockfish
stockfish bench 0 1 8 current perft

### QPerft
perft 8
