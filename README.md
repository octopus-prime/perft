# perft

* bulk counting
* no hashing
* single core

## Performance

### perft 7

|       | This           | Stockfish      | QPerft         |
|-------|----------------|----------------|----------------|
| Nodes | 3,195,901,860  | 3,195,901,860  | 3,195,901,860  |
| Time  | 10.087         | 12.30          | 12.457         |
| Perf  | 316,827,980    | 259,913,944    | 256,554,697    |

### perft 8

|       | This           | Stockfish      | QPerft         |
|-------|----------------|----------------|----------------|
| Nodes | 84,998,978,956 | 84,998,978,956 | 84,998,978,956 |
| Time  | 275.796        | 323.134        | 333.844        |
| Perf  | 308,194,990    | 263,045,606    | 254,606,879    |

### Hardware

AMD Ryzen 9 5900X 12-Core Processor @ 3.70 GHz

## Commands

### Stockfish
stockfish bench 0 1 8 current perft

### QPerft
perft 8
