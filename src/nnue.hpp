#pragma once

#include "side.hpp"

/**
* Internal piece representation
*     wking=1, wqueen=2, wrook=3, wbishop= 4, wknight= 5, wpawn= 6,
*     bking=7, bqueen=8, brook=9, bbishop=10, bknight=11, bpawn=12
*
* Make sure the piecesyou pass to the library from your engine
* use this format.
*/
enum colors {
    white,black
};
enum pieces {
    blank=0,wking,wqueen,wrook,wbishop,wknight,wpawn,
            bking,bqueen,brook,bbishop,bknight,bpawn
};

/**
* nnue data structure
*/

typedef struct DirtyPiece {
  int dirtyNum;
  int pc[3];
  int from[3];
  int to[3];
} DirtyPiece;

typedef struct Accumulator {
  alignas(64) int16_t accumulation[2][256];
  int computedAccumulation;
} Accumulator;

typedef struct NNUEdata {
  Accumulator accumulator;
  DirtyPiece dirtyPiece;
} NNUEdata;

class node;

template <side_t side>
int nnue_evaluate_node(node& node);

/************************************************************************
*         EXTERNAL INTERFACES
*
* Load a NNUE file using
*
*   nnue_init(file_path)
*
* and then probe score using one of three functions, whichever
* is convenient. From easy to hard
*   
*   a) nnue_evaluate_fen         - accepts a fen string for evaluation
*   b) nnue_evaluate             - suitable for use in engines
*   c) nnue_evaluate_incremental - for ultimate performance but will need
*                                  some work on the engines side.
*
**************************************************************************/

/**
* Load NNUE file
*/
void nnue_init(
  const char * evalFile             /** Path to NNUE file */
);
