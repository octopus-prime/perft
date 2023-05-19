#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#  define USE_AVX2   1

#include <immintrin.h>

//-------------------
#include "misc.hpp"
#include "nnue.hpp"
#include "node.hpp"

#define KING(c)    ( (c) ? bking : wking )
#define IS_KING(p) ( ((p) == wking) || ((p) == bking) )
//-------------------

enum {
  PS_W_PAWN   =  1,
  PS_B_PAWN   =  1 * 64 + 1,
  PS_W_KNIGHT =  2 * 64 + 1,
  PS_B_KNIGHT =  3 * 64 + 1,
  PS_W_BISHOP =  4 * 64 + 1,
  PS_B_BISHOP =  5 * 64 + 1,
  PS_W_ROOK   =  6 * 64 + 1,
  PS_B_ROOK   =  7 * 64 + 1,
  PS_W_QUEEN  =  8 * 64 + 1,
  PS_B_QUEEN  =  9 * 64 + 1,
  PS_END      = 10 * 64 + 1
};

uint32_t PieceToIndex[2][14] = {
  { 0, 0, PS_W_QUEEN, PS_W_ROOK, PS_W_BISHOP, PS_W_KNIGHT, PS_W_PAWN,
       0, PS_B_QUEEN, PS_B_ROOK, PS_B_BISHOP, PS_B_KNIGHT, PS_B_PAWN, 0},
  { 0, 0, PS_B_QUEEN, PS_B_ROOK, PS_B_BISHOP, PS_B_KNIGHT, PS_B_PAWN,
       0, PS_W_QUEEN, PS_W_ROOK, PS_W_BISHOP, PS_W_KNIGHT, PS_W_PAWN, 0}
};

// Version of the evaluation file
static const uint32_t NnueVersion = 0x7AF32F16u;

// Constants used in evaluation value calculation
enum {
  FV_SCALE = 16,
  SHIFT = 6
};

enum {
  kHalfDimensions = 256,
  FtInDims = 64 * PS_END, // 64 * 641
  FtOutDims = kHalfDimensions * 2
};

static_assert(kHalfDimensions % 256 == 0, "kHalfDimensions should be a multiple of 256");

#ifdef USE_AVX512
#define SIMD_WIDTH 512
typedef __m512i vec16_t;
typedef __m512i vec8_t;
typedef __mmask64 mask_t;
#define vec_add_16(a,b) _mm512_add_epi16(a,b)
#define vec_sub_16(a,b) _mm512_sub_epi16(a,b)
#define vec_packs(a,b) _mm512_packs_epi16(a,b)
#define vec_mask_pos(a) _mm512_cmpgt_epi8_mask(a,_mm512_setzero_si512())
#define NUM_REGS 8 // only 8 are needed

#elif USE_AVX2
#define SIMD_WIDTH 256
typedef __m256i vec16_t;
typedef __m256i vec8_t;
typedef uint32_t mask_t;
#define vec_add_16(a,b) _mm256_add_epi16(a,b)
#define vec_sub_16(a,b) _mm256_sub_epi16(a,b)
#define vec_packs(a,b) _mm256_packs_epi16(a,b)
#define vec_mask_pos(a) _mm256_movemask_epi8(_mm256_cmpgt_epi8(a,_mm256_setzero_si256()))
#define NUM_REGS 16
#endif

typedef uint64_t mask2_t;
typedef int8_t clipped_t;
typedef int8_t weight_t;

typedef struct {
  size_t size;
  unsigned values[30];
} IndexList;

INLINE int orient(int c, int s)
{
  return s ^ (c == white ? 0x00 : 0x3f);
}

INLINE unsigned make_index(int c, int s, int pc, int ksq)
{
  return orient(c, s) + PieceToIndex[c][pc] + PS_END * ksq;
}

static void half_kp_append_active_indices(node& current, const int c,
    IndexList *active)
{
  int ksq = c == 0 ? current.king<WHITE>().find() : current.king<BLACK>().find();
  ksq = orient(c, ksq);

  for (square sq : current.queen<WHITE>())
    active->values[active->size++] = make_index(c, sq, wqueen, ksq);
  for (square sq : current.queen<BLACK>())
    active->values[active->size++] = make_index(c, sq, bqueen, ksq);

  for (square sq : current.rook<WHITE>())
    active->values[active->size++] = make_index(c, sq, wrook, ksq);
  for (square sq : current.rook<BLACK>())
    active->values[active->size++] = make_index(c, sq, brook, ksq);

  for (square sq : current.bishop<WHITE>())
    active->values[active->size++] = make_index(c, sq, wbishop, ksq);
  for (square sq : current.bishop<BLACK>())
    active->values[active->size++] = make_index(c, sq, bbishop, ksq);

  for (square sq : current.knight<WHITE>())
    active->values[active->size++] = make_index(c, sq, wknight, ksq);
  for (square sq : current.knight<BLACK>())
    active->values[active->size++] = make_index(c, sq, bknight, ksq);

  for (square sq : current.pawn<WHITE>())
    active->values[active->size++] = make_index(c, sq, wpawn, ksq);
  for (square sq : current.pawn<BLACK>())
    active->values[active->size++] = make_index(c, sq, bpawn, ksq);
}

static void half_kp_append_changed_indices(node& current, const int c,
    const DirtyPiece *dp, IndexList *removed, IndexList *added)
{
  int ksq = c == 0 ? current.king<WHITE>().find() : current.king<BLACK>().find();
  ksq = orient(c, ksq);
  for (int i = 0; i < dp->dirtyNum; i++) {
    int pc = dp->pc[i];
    if (IS_KING(pc)) continue;
    if (dp->from[i] != 64)
      removed->values[removed->size++] = make_index(c, dp->from[i], pc, ksq);
    if (dp->to[i] != 64)
      added->values[added->size++] = make_index(c, dp->to[i], pc, ksq);
  }
}

static void append_active_indices(node& current, IndexList active[2])
{
  for (unsigned c = 0; c < 2; c++)
    half_kp_append_active_indices(current, c, &active[c]);
}

static void append_changed_indices(node& current, IndexList removed[2],
    IndexList added[2], bool reset[2])
{
  const DirtyPiece *dp = &(current.nnue.dirtyPiece);
  // assert(dp->dirtyNum != 0);

  if (current.parent()->nnue.accumulator.computedAccumulation) {
    for (unsigned c = 0; c < 2; c++) {
      reset[c] = dp->pc[0] == (int)KING(c);
      if (reset[c])
        half_kp_append_active_indices(current, c, &added[c]);
      else
        half_kp_append_changed_indices(current, c, dp, &removed[c], &added[c]);
    }
  } else {
    const DirtyPiece *dp2 = &(current.parent()->nnue.dirtyPiece);
    for (unsigned c = 0; c < 2; c++) {
      reset[c] =   dp->pc[0] == (int)KING(c)
                || dp2->pc[0] == (int)KING(c);
      if (reset[c])
        half_kp_append_active_indices(current, c, &added[c]);
      else {
        half_kp_append_changed_indices(current, c, dp, &removed[c], &added[c]);
        half_kp_append_changed_indices(current, c, dp2, &removed[c], &added[c]);
      }
    }
  }
}

// InputLayer = InputSlice<256 * 2>
// out: 512 x clipped_t

// Hidden1Layer = ClippedReLu<AffineTransform<InputLayer, 32>>
// 512 x clipped_t -> 32 x int32_t -> 32 x clipped_t

// Hidden2Layer = ClippedReLu<AffineTransform<hidden1, 32>>
// 32 x clipped_t -> 32 x int32_t -> 32 x clipped_t

// OutputLayer = AffineTransform<HiddenLayer2, 1>
// 32 x clipped_t -> 1 x int32_t

#if !defined(USE_AVX512)
static weight_t hidden1_weights alignas(64) [32 * 512];
static weight_t hidden2_weights alignas(64) [32 * 32];
#else
static weight_t hidden1_weights alignas(64) [64 * 512];
static weight_t hidden2_weights alignas(64) [64 * 32];
#endif
static weight_t output_weights alignas(64) [1 * 32];

static int32_t hidden1_biases alignas(64) [32];
static int32_t hidden2_biases alignas(64) [32];
static int32_t output_biases[1];

INLINE int32_t affine_propagate(clipped_t *input, int32_t *biases,
    weight_t *weights)
{
#if defined(USE_AVX2)
  __m256i *iv = (__m256i *)input;
  __m256i *row = (__m256i *)weights;
#if defined(USE_VNNI)
  __m256i prod = _mm256_dpbusd_epi32(_mm256_setzero_si256(), iv[0], row[0]);
#else
  __m256i prod = _mm256_maddubs_epi16(iv[0], row[0]);
  prod = _mm256_madd_epi16(prod, _mm256_set1_epi16(1));
#endif
  __m128i sum = _mm_add_epi32(
      _mm256_castsi256_si128(prod), _mm256_extracti128_si256(prod, 1));
  sum = _mm_add_epi32(sum, _mm_shuffle_epi32(sum, 0x1b));
  return _mm_cvtsi128_si32(sum) + _mm_extract_epi32(sum, 1) + biases[0];
#endif
}

static_assert(FtOutDims % 64 == 0, "FtOutDims not a multiple of 64");

INLINE bool next_idx(unsigned *idx, unsigned *offset, mask2_t *v,
    mask_t *mask, unsigned inDims)
{
  while (*v == 0) {
    *offset += 8 * sizeof(mask2_t);
    if (*offset >= inDims) return false;
    memcpy(v, (char *)mask + (*offset / 8), sizeof(mask2_t));
  }
  *idx = *offset + std::countr_zero(*v);
  *v &= *v - 1;
  return true;
}

#if defined(USE_AVX512)
INLINE void affine_txfm(int8_t *input, void *output, unsigned inDims,
    unsigned outDims, const int32_t *biases, const weight_t *weights,
    mask_t *inMask, mask_t *outMask, const bool pack8_and_calc_mask)
{
  assert(outDims == 32);

  (void)outDims;
  const __m512i kZero = _mm512_setzero_si512();
  __m512i out_0 = ((__m512i *)biases)[0];
  __m512i out_1 = ((__m512i *)biases)[1];
  __m512i first, second;
  mask2_t v;
  unsigned idx;

  memcpy(&v, inMask, sizeof(mask2_t));
  for (unsigned offset = 0; offset < inDims;) {
    if (!next_idx(&idx, &offset, &v, inMask, inDims))
      break;
    first = ((__m512i *)weights)[idx];
    uint16_t factor = input[idx];
    if (next_idx(&idx, &offset, &v, inMask, inDims)) {
      second = ((__m512i *)weights)[idx];
      factor |= input[idx] << 8;
    } else {
      second = kZero;
    }
    __m512i mul = _mm512_set1_epi16(factor), prod, signs;
    prod = _mm512_maddubs_epi16(mul, _mm512_unpacklo_epi8(first, second));
    signs = _mm512_srai_epi16(prod, 15);
    out_0 = _mm512_add_epi32(out_0, _mm512_unpacklo_epi16(prod, signs));
    out_1 = _mm512_add_epi32(out_1, _mm512_unpackhi_epi16(prod, signs));
  }

  __m512i out16 = _mm512_srai_epi16(_mm512_packs_epi32(out_0, out_1), SHIFT);

  __m256i *outVec = (__m256i *)output;
  const __m256i kZero256 = _mm256_setzero_si256();
  outVec[0] = _mm256_packs_epi16(
      _mm512_castsi512_si256(out16),_mm512_extracti64x4_epi64(out16, 1));
  if (pack8_and_calc_mask)
    outMask[0] = (uint32_t)_mm256_movemask_epi8(_mm256_cmpgt_epi8(outVec[0], kZero256));
  else
    outVec[0] = _mm256_max_epi8(outVec[0], kZero256);
}
#elif defined(USE_AVX2)
INLINE void affine_txfm(int8_t *input, void *output, unsigned inDims,
    unsigned outDims, const int32_t *biases, const weight_t *weights,
    mask_t *inMask, mask_t *outMask, const bool pack8_and_calc_mask)
{
  assert(outDims == 32);

  (void)outDims;
  const __m256i kZero = _mm256_setzero_si256();
  __m256i out_0 = ((__m256i *)biases)[0];
  __m256i out_1 = ((__m256i *)biases)[1];
  __m256i out_2 = ((__m256i *)biases)[2];
  __m256i out_3 = ((__m256i *)biases)[3];
  __m256i first, second;
  mask2_t v;
  unsigned idx;

  memcpy(&v, inMask, sizeof(mask2_t));
  for (unsigned offset = 0; offset < inDims;) {
    if (!next_idx(&idx, &offset, &v, inMask, inDims))
      break;
    first = ((__m256i *)weights)[idx];
    uint16_t factor = input[idx];
    if (next_idx(&idx, &offset, &v, inMask, inDims)) {
      second = ((__m256i *)weights)[idx];
      factor |= input[idx] << 8;
    } else {
      second = kZero;
    }
    __m256i mul = _mm256_set1_epi16(factor), prod, signs;
    prod = _mm256_maddubs_epi16(mul, _mm256_unpacklo_epi8(first, second));
    signs = _mm256_cmpgt_epi16(kZero, prod);
    out_0 = _mm256_add_epi32(out_0, _mm256_unpacklo_epi16(prod, signs));
    out_1 = _mm256_add_epi32(out_1, _mm256_unpackhi_epi16(prod, signs));
    prod = _mm256_maddubs_epi16(mul, _mm256_unpackhi_epi8(first, second));
    signs = _mm256_cmpgt_epi16(kZero, prod);
    out_2 = _mm256_add_epi32(out_2, _mm256_unpacklo_epi16(prod, signs));
    out_3 = _mm256_add_epi32(out_3, _mm256_unpackhi_epi16(prod, signs));
  }

  __m256i out16_0 = _mm256_srai_epi16(_mm256_packs_epi32(out_0, out_1), SHIFT);
  __m256i out16_1 = _mm256_srai_epi16(_mm256_packs_epi32(out_2, out_3), SHIFT);

  __m256i *outVec = (__m256i *)output;
  outVec[0] = _mm256_packs_epi16(out16_0, out16_1);
  if (pack8_and_calc_mask)
    outMask[0] = _mm256_movemask_epi8(_mm256_cmpgt_epi8(outVec[0], kZero));
  else
    outVec[0] = _mm256_max_epi8(outVec[0], kZero);
}
#endif

// Input feature converter
static int16_t ft_biases alignas(64) [kHalfDimensions];
static int16_t ft_weights alignas(64) [kHalfDimensions * FtInDims];

#define TILE_HEIGHT (NUM_REGS * SIMD_WIDTH / 16)

// Calculate cumulative value without using difference calculation
INLINE void refresh_accumulator(node& current)
{
  Accumulator *accumulator = &(current.nnue.accumulator);

  IndexList activeIndices[2];
  activeIndices[0].size = activeIndices[1].size = 0;
  append_active_indices(current, activeIndices);

  for (unsigned c = 0; c < 2; c++) {
    for (unsigned i = 0; i < kHalfDimensions / TILE_HEIGHT; i++) {
      vec16_t *ft_biases_tile = (vec16_t *)&ft_biases[i * TILE_HEIGHT];
      vec16_t *accTile = (vec16_t *)&accumulator->accumulation[c][i * TILE_HEIGHT];
      vec16_t acc[NUM_REGS];

      for (unsigned j = 0; j < NUM_REGS; j++)
        acc[j] = ft_biases_tile[j];

      for (size_t k = 0; k < activeIndices[c].size; k++) {
        unsigned index = activeIndices[c].values[k];
        unsigned offset = kHalfDimensions * index + i * TILE_HEIGHT;
        vec16_t *column = (vec16_t *)&ft_weights[offset];

        for (unsigned j = 0; j < NUM_REGS; j++)
          acc[j] = vec_add_16(acc[j], column[j]);
      }

      for (unsigned j = 0; j < NUM_REGS; j++)
        accTile[j] = acc[j];
    }
  }

  accumulator->computedAccumulation = 1;
}

// Calculate cumulative value using difference calculation if possible
INLINE bool update_accumulator(node& current)
{
  Accumulator *accumulator = &(current.nnue.accumulator);
  if (accumulator->computedAccumulation)
    return true;

  Accumulator *prevAcc;
  if (   (!current.parent() || !(prevAcc = &current.parent()->nnue.accumulator)->computedAccumulation)
      && (!current.parent()->parent() || !(prevAcc = &current.parent()->parent()->nnue.accumulator)->computedAccumulation) )
    return false;

  IndexList removed_indices[2], added_indices[2];
  removed_indices[0].size = removed_indices[1].size = 0;
  added_indices[0].size = added_indices[1].size = 0;
  bool reset[2];
  append_changed_indices(current, removed_indices, added_indices, reset);

  for (unsigned i = 0; i< kHalfDimensions / TILE_HEIGHT; i++) {
    for (unsigned c = 0; c < 2; c++) {
      vec16_t *accTile = (vec16_t *)&accumulator->accumulation[c][i * TILE_HEIGHT];
      vec16_t acc[NUM_REGS];

      if (reset[c]) {
        vec16_t *ft_b_tile = (vec16_t *)&ft_biases[i * TILE_HEIGHT];
        for (unsigned j = 0; j < NUM_REGS; j++)
          acc[j] = ft_b_tile[j];
      } else {
        vec16_t *prevAccTile = (vec16_t *)&prevAcc->accumulation[c][i * TILE_HEIGHT];
        for (unsigned j = 0; j < NUM_REGS; j++)
          acc[j] = prevAccTile[j];

        // Difference calculation for the deactivated features
        for (unsigned k = 0; k < removed_indices[c].size; k++) {
          unsigned index = removed_indices[c].values[k];
          const unsigned offset = kHalfDimensions * index + i * TILE_HEIGHT;

          vec16_t *column = (vec16_t *)&ft_weights[offset];
          for (unsigned j = 0; j < NUM_REGS; j++)
            acc[j] = vec_sub_16(acc[j], column[j]);
        }
      }

      // Difference calculation for the activated features
      for (unsigned k = 0; k < added_indices[c].size; k++) {
        unsigned index = added_indices[c].values[k];
        const unsigned offset = kHalfDimensions * index + i * TILE_HEIGHT;

        vec16_t *column = (vec16_t *)&ft_weights[offset];
        for (unsigned j = 0; j < NUM_REGS; j++)
          acc[j] = vec_add_16(acc[j], column[j]);
      }

      for (unsigned j = 0; j < NUM_REGS; j++)
        accTile[j] = acc[j];
    }
  }

  accumulator->computedAccumulation = 1;
  return true;
}

// Convert input features
INLINE void transform(node& current, side_t side, clipped_t *output, mask_t *outMask)
{
  if (!update_accumulator(current))
    refresh_accumulator(current);

  int16_t (*accumulation)[2][256] = &current.nnue.accumulator.accumulation;
  (void)outMask; // avoid compiler warning

  const int perspectives[2] = { side, ~side };
  for (unsigned p = 0; p < 2; p++) {
    const unsigned offset = kHalfDimensions * p;

    const unsigned numChunks = (16 * kHalfDimensions) / SIMD_WIDTH;
    vec8_t *out = (vec8_t *)&output[offset];
    for (unsigned i = 0; i < numChunks / 2; i++) {
      vec16_t s0 = ((vec16_t *)(*accumulation)[perspectives[p]])[i * 2];
      vec16_t s1 = ((vec16_t *)(*accumulation)[perspectives[p]])[i * 2 + 1];
      out[i] = vec_packs(s0, s1);
      *outMask++ = vec_mask_pos(out[i]);
    }
  }
}

struct NetData {
  alignas(64) clipped_t input[FtOutDims];
  clipped_t hidden1_out[32];
  int8_t hidden2_out[32];
};

// Evaluation function
template <side_t side>
int nnue_evaluate_node(node& current)
{
  int32_t out_value;
  alignas(8) mask_t input_mask[FtOutDims / (8 * sizeof(mask_t))];
  alignas(8) mask_t hidden1_mask[8 / sizeof(mask_t)] = { 0 };
  struct NetData buf;
#define B(x) (buf.x)

  transform(current, side, B(input), input_mask);

  affine_txfm(B(input), B(hidden1_out), FtOutDims, 32,
      hidden1_biases, hidden1_weights, input_mask, hidden1_mask, true);

  affine_txfm(B(hidden1_out), B(hidden2_out), 32, 32,
      hidden2_biases, hidden2_weights, hidden1_mask, NULL, false);

  out_value = affine_propagate((int8_t *)B(hidden2_out), output_biases,
      output_weights);

  return out_value / FV_SCALE;
}

template int nnue_evaluate_node<WHITE>(node& current);
template int nnue_evaluate_node<BLACK>(node& current);

static void read_output_weights(weight_t *w, const char *d)
{
  for (unsigned i = 0; i < 32; i++) {
    unsigned c = i;
#if defined(USE_AVX512)
    unsigned b = c & 0x18;
    b = (b << 1) | (b >> 1);
    c = (c & ~0x18) | (b & 0x18);
#endif
    w[c] = *d++;
  }
}

INLINE unsigned wt_idx(unsigned r, unsigned c, unsigned dims)
{
  (void)dims;

#if defined(USE_AVX512)
  if (dims > 32) {
    unsigned b = c & 0x38;
    b = (b << 1) | (b >> 2);
    c = (c & ~0x38) | (b & 0x38);
  }
  else if (dims == 32) {
    unsigned b = c & 0x18;
    b = (b << 1) | (b >> 1);
    c = (c & ~0x18) | (b & 0x18);
  }

#elif defined(USE_AVX2)
  if (dims > 32) {
    unsigned b = c & 0x18;
    b = (b << 1) | (b >> 1);
    c = (c & ~0x18) | (b & 0x18);
  }

#endif

#if defined(USE_AVX512)
  return c * 64 + r + (r & ~7);

#else
  return c * 32 + r;

#endif
}

static const char *read_hidden_weights(weight_t *w, unsigned dims,
    const char *d)
{
  for (unsigned r = 0; r < 32; r++)
    for (unsigned c = 0; c < dims; c++)
      w[wt_idx(r, c, dims)] = *d++;

  return d;
}

#ifdef USE_AVX2
static void permute_biases(int32_t *biases)
{
  __m128i *b = (__m128i *)biases;
  __m128i tmp[8];
#ifdef USE_AVX512
  tmp[0] = b[0];
  tmp[1] = b[2];
  tmp[2] = b[4];
  tmp[3] = b[6];
  tmp[4] = b[1];
  tmp[5] = b[3];
  tmp[6] = b[5];
  tmp[7] = b[7];
#elif USE_AVX2
  tmp[0] = b[0];
  tmp[1] = b[4];
  tmp[2] = b[1];
  tmp[3] = b[5];
  tmp[4] = b[2];
  tmp[5] = b[6];
  tmp[6] = b[3];
  tmp[7] = b[7];
#else
#error
#endif
  memcpy(b, tmp, 8 * sizeof(__m128i));
}
#endif

enum {
  TransformerStart = 3 * 4 + 177,
  NetworkStart = TransformerStart + 4 + 2 * 256 + 2 * 256 * 64 * 641
};

static bool verify_net(const void *evalData, size_t size)
{
  if (size != 21022697) return false;

  const char *d = (const char*)evalData;
  if (readu_le_u32(d) != NnueVersion) return false;
  if (readu_le_u32(d + 4) != 0x3e5aa6eeU) return false;
  if (readu_le_u32(d + 8) != 177) return false;
  if (readu_le_u32(d + TransformerStart) != 0x5d69d7b8) return false;
  if (readu_le_u32(d + NetworkStart) != 0x63337156) return false;

  return true;
}

static void init_weights(const void *evalData)
{
  const char *d = (const char *)evalData + TransformerStart + 4;

  // Read transformer
  for (unsigned i = 0; i < kHalfDimensions; i++, d += 2)
    ft_biases[i] = readu_le_u16(d);
  for (unsigned i = 0; i < kHalfDimensions * FtInDims; i++, d += 2)
    ft_weights[i] = readu_le_u16(d);

  // Read network
  d += 4;
  for (unsigned i = 0; i < 32; i++, d += 4)
    hidden1_biases[i] = readu_le_u32(d);
  d = read_hidden_weights(hidden1_weights, 512, d);
  for (unsigned i = 0; i < 32; i++, d += 4)
    hidden2_biases[i] = readu_le_u32(d);
  d = read_hidden_weights(hidden2_weights, 32, d);
  for (unsigned i = 0; i < 1; i++, d += 4)
    output_biases[i] = readu_le_u32(d);
  read_output_weights(output_weights, d);

#ifdef USE_AVX2
  permute_biases(hidden1_biases);
  permute_biases(hidden2_biases);
#endif
}

static bool load_eval_file(const char *evalFile)
{
  const void *evalData;
  map_t mapping;
  size_t size;

  {
    FD fd = open_file(evalFile);
    if (fd == FD_ERR) return false;
    evalData = map_file(fd, &mapping);
    size = file_size(fd);
    close_file(fd);
  }

  bool success = verify_net(evalData, size);
  if (success)
    init_weights(evalData);
  if (mapping) unmap_file(evalData, mapping);
  return success;
}

/*
Interfaces
*/
void nnue_init(const char* evalFile)
{
  printf("Loading NNUE : %s\n", evalFile);
  fflush(stdout);

  if (load_eval_file(evalFile)) {
    printf("NNUE loaded !\n");
    fflush(stdout);
    return;
  }

  printf("NNUE file not found!\n");
  fflush(stdout);
}
