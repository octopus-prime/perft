# perft

* bulk counting
* no hashing

## Usage

```
Usage: ./perft <command> <depth> [<fen>]
Commands: p[erft] | d[ivide] | t[est]
```

## Performance

### perft 7

* single core

|       | This          | Stockfish     | QPerft        |
|-------|---------------|---------------|---------------|
| Nodes | 3,195,901,860 | 3,195,901,860 | 3,195,901,860 |
| Time  | 6.89233       | 12.30         | 12.457        |
| Perf  | 463,689,470   | 259,913,944   | 256,554,697   |

### perft 8

* single core

|       | This           | Stockfish      | QPerft         |
|-------|----------------|----------------|----------------|
| Nodes | 84,998,978,956 | 84,998,978,956 | 84,998,978,956 |
| Time  | 183.013        | 323.134        | 333.844        |
| Perf  | 464,443,497    | 263,045,606    | 254,606,879    |

### test suite

* all cores

|       | depth = 5       | depth = 6         |
|-------|-----------------|-------------------|
| Nodes | 131,411,896,751 | 4,156,802,421,433 |
| Time  | 20.0968         | 639.391           |
| Perf  | 6,538,944,193   | 6,501,193,715     |

### Hardware

AMD Ryzen 9 5900X 12-Core Processor @ 3.70 GHz

## Commands

### This

`perft d 8`

### Stockfish

`stockfish bench 0 1 8 current perft`

### QPerft

`perft 8`
