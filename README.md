# perft

* bulk counting
* no hashing

## Usage

```
Usage: ./perft <command> <depth> [<fen>]
Commands: p[erft] | d[ivide] | t[est]
```

### Example divide 8 initial position

```
$ ./perft d 8
Nb1a3 3193522577
Nb1c3 3926684340
Ng1f3 3937354096
Ng1h3 3221278282
Pa2a3 2863411653
Pb2b3 3579299617
Pc2c3 3806229124
Pd2d3 6093248619
Pe2e3 8039390919
Pf2f3 2728615868
Pg2g3 3641432923
Ph2h3 2860408680
Pa2a4 3676309619
Pb2b4 3569067629
Pc2c4 4199667616
Pd2d4 7184581950
Pe2e4 8102108221
Pf2f4 3199039406
Pg2g4 3466204702
Ph2h4 3711123115
n = 84,998,978,956
t = 183.013
p = 464,443,497
```

### Example perft 6 kiwipete position

```
$ ./perft p 6 'r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -'
n = 8,031,647,685
t = 14.4509
p = 555,787,292
```

## Performance

### perft 7

* single core

|             | This          | Stockfish     | QPerft        |
|-------------|---------------|---------------|---------------|
| Nodes       | 3,195,901,860 | 3,195,901,860 | 3,195,901,860 |
| Time (s)    | 6.89233       | 10.485        | 8.367         |
| Perf (N/s)  | 463,689,470   | 304,807,044   | 381,965,084   |

### perft 8

* single core

|             | This           | Stockfish      | QPerft         |
|-------------|----------------|----------------|----------------|
| Nodes       | 84,998,978,956 | 84,998,978,956 | 84,998,978,956 |
| Time (s)    | 183.013        | 284.789        | 227.948        |
| Perf (N/s)  | 464,443,497    | 298,462,998    | 372,887,583    |

### test suite

* all cores

|            | depth = 5       | depth = 6         |
|------------|-----------------|-------------------|
| Nodes      | 131,411,896,751 | 4,156,802,421,433 |
| Time (s)   | 20.0968         | 639.391           |
| Perf (N/s) | 6,538,944,193   | 6,501,193,715     |

### Hardware

AMD Ryzen 9 5900X 12-Core Processor @ 3.70 GHz

### Compiler

GCC 13 with `-O3 -march=native -flto`

## Commands

### This

* Build `cmake --build`
* Run `perft d 8`

### Stockfish

* Build `make -j profile-build ARCH=x86-64-avx2`
* Run `stockfish bench 0 1 8 current perft`

### QPerft

* Build `gcc -O3 -march=native -flto perft.c`
* Run `perft 8`
