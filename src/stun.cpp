#include <clipp.h>
#include <fmt/printf.h>
#include <fmt/ranges.h>
#include <memory>
#include <string>

#include "stun_dag/common.h"
#include "stun_dag/random_utils.h"
#include "stun_dag/schedule_info.h"
#include "stun_dag/stochastic_tunneling.h"
#include "stun_dag/temperature.h"
#include "stun_dag/utils.h"
#include <gsl/gsl-lite.hpp>
#include <range/v3/view/span.hpp>
#include <utility>

namespace angonoka::stun_dag {

void run()
{
    std::vector<int16> available_agents_data{
        0, 1, 3, 4, 5, 6, 7, 8, 9, 0, 1, 3, 7, 0, 2, 3, 6, 8, 9, 0, 1,
        2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 4, 5, 7, 9, 2, 3, 0, 1, 2, 5,
        7, 8, 9, 0, 1, 2, 3, 5, 6, 7, 8, 9, 3, 5, 7, 9, 6, 2, 3, 8, 9,
        1, 2, 5, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 3,
        1, 2, 3, 5, 6, 7, 8, 9, 0, 1, 2, 3, 6, 8, 0, 8, 2, 3, 4, 6, 7,
        8, 9, 1, 7, 8, 0, 3, 4, 7, 9, 3, 4, 5, 6, 9, 2, 4, 6, 0, 1, 3,
        5, 6, 7, 0, 1, 2, 3, 6, 7, 8, 9, 7, 2, 3, 7, 3, 6, 8, 9, 1, 6,
        7, 2, 4, 6, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4,
        5, 6, 3, 5, 6, 4, 0, 1, 2, 4, 5, 8, 9, 3, 7, 0, 3, 4, 6, 7, 8,
        9, 0, 2, 4, 5, 6, 7, 9, 6, 2, 5, 7, 8, 9, 1, 2, 3, 4, 5, 7, 8,
        9, 0, 1, 2, 3, 6, 7, 9, 1, 2, 5, 7, 8, 9, 0, 1, 3, 5, 6, 8, 1,
        3, 5, 6, 7, 0, 1, 2, 4, 6, 7, 8, 9, 8, 0, 1, 2, 3, 5, 6, 7, 8,
        9, 0, 1, 4, 6, 7, 8, 9, 0, 2, 9, 0, 1, 3, 9, 6, 9, 0, 1, 2, 3,
        4, 6, 7, 8, 9, 3, 6, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 4,
        8, 9, 0, 1, 2, 4, 5, 7, 0, 2, 3, 4, 5, 7, 8, 9, 0, 2, 3, 0, 1,
        2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 6, 7, 8, 9, 1, 2, 5, 6,
        3, 8, 0, 2, 3, 4, 5, 6, 7, 8, 9, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3,
        4, 5, 6, 7, 8, 9, 0, 3, 4, 6, 8, 2, 4, 5, 0, 1, 2, 3, 4, 5, 6,
        7, 8, 9, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 3, 7, 2,
        3, 4, 7, 1, 2, 7, 0, 1, 3, 4, 5, 6, 8, 9, 0, 1, 4, 5, 6, 7, 4,
        0, 1, 2, 3, 5, 6, 7, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 3,
        4, 5, 8, 9, 0, 1, 2, 3, 4, 5, 6, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8,
        9, 0, 1, 3, 4, 5, 6, 7, 3, 4, 7, 0, 2, 3, 5, 8, 1, 3, 5, 6, 7,
        2, 2, 4, 5, 1, 4, 5, 6, 8, 9, 0, 2, 6, 0, 1, 4, 8, 9, 0, 1, 5,
        6, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 9, 4, 5,
        8, 9, 0, 1, 2, 3, 4, 5, 7, 8, 9, 0, 1, 4, 7, 8, 0, 1, 2, 4, 5,
        6, 7, 9, 1, 2, 4, 6, 8, 0, 1, 2, 3, 5, 6, 8, 0, 1, 3, 4, 5, 7,
        0, 1, 2, 3, 4, 7, 8, 9, 0, 1, 3, 4, 5, 8, 9, 2, 3, 4, 8, 9, 0,
        1, 2, 3, 4, 5, 7, 8, 1, 3, 4, 6, 8, 9, 3, 4, 6, 8, 5, 6, 8, 1,
        3, 4, 6, 3, 5, 6, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 3, 4, 5,
        6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 3, 6, 7, 8, 0, 2, 3, 5, 7, 9, 0, 1, 2, 3, 5, 7, 8, 9, 0,
        1, 9};
    std::vector<int16> dependencies_data{
        1,   2,   1,   0,   3,   4,   0,   2,   3,   0,   2,   3,
        4,   5,   6,   7,   0,   1,   2,   3,   5,   7,   8,   6,
        0,   1,   3,   4,   5,   7,   8,   9,   10,  5,   6,   9,
        10,  0,   1,   2,   3,   4,   5,   7,   9,   10,  11,  12,
        2,   8,   9,   0,   2,   4,   7,   13,  14,  1,   7,   10,
        14,  0,   1,   2,   3,   4,   5,   6,   7,   9,   10,  12,
        13,  14,  16,  1,   3,   5,   7,   9,   10,  11,  14,  15,
        16,  0,   1,   2,   3,   4,   6,   7,   9,   10,  11,  15,
        17,  18,  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
        10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,
        2,   4,   6,   7,   8,   10,  11,  17,  18,  19,  20,  22,
        4,   21,  22,  0,   1,   2,   3,   4,   5,   6,   7,   8,
        9,   10,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,
        22,  23,  24,  1,   3,   4,   13,  15,  22,  0,   1,   2,
        3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  14,  15,
        16,  17,  18,  19,  20,  21,  22,  23,  24,  26,  21,  1,
        3,   4,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,
        16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
        28,  2,   3,   5,   6,   7,   9,   12,  13,  19,  21,  22,
        23,  25,  26,  28,  29,  1,   2,   3,   4,   5,   8,   9,
        10,  11,  13,  17,  18,  19,  20,  21,  23,  26,  27,  30,
        4,   5,   6,   8,   11,  13,  18,  21,  24,  26,  29,  30,
        31,  0,   1,   2,   4,   5,   6,   7,   8,   9,   10,  11,
        16,  17,  19,  23,  25,  26,  31,  1,   2,   3,   5,   6,
        7,   9,   10,  13,  14,  16,  19,  20,  21,  22,  23,  24,
        25,  26,  27,  28,  29,  32,  33,  19,  26,  1,   2,   3,
        5,   8,   9,   11,  12,  14,  15,  18,  21,  22,  23,  24,
        26,  28,  29,  30,  31,  33,  34,  3,   25,  27,  30,  35,
        0,   1,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,
        13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
        26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,
        1,   2,   3,   4,   5,   6,   8,   9,   10,  13,  14,  19,
        20,  21,  22,  23,  24,  25,  31,  32,  34,  35,  36,  37,
        0,   2,   3,   4,   6,   7,   8,   9,   10,  11,  12,  14,
        15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,
        27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
        39,  0,   3,   4,   6,   7,   8,   9,   10,  12,  13,  14,
        15,  16,  17,  19,  20,  22,  23,  25,  26,  27,  28,  29,
        30,  31,  32,  34,  35,  36,  38,  40,  2,   3,   16,  26,
        29,  30,  35,  37,  41,  0,   1,   2,   3,   4,   5,   7,
        8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  20,
        21,  22,  23,  25,  26,  27,  28,  29,  30,  31,  32,  33,
        34,  35,  36,  37,  38,  40,  41,  42,  4,   8,   9,   29,
        31,  39,  42,  1,   4,   5,   11,  13,  15,  16,  17,  19,
        21,  22,  30,  33,  35,  36,  37,  41,  42,  43,  44,  0,
        2,   3,   4,   5,   7,   8,   9,   10,  11,  12,  13,  14,
        15,  16,  17,  22,  24,  25,  27,  28,  30,  32,  39,  40,
        42,  43,  44,  45,  2,   3,   4,   5,   7,   18,  20,  26,
        30,  33,  36,  39,  43,  44,  46,  0,   2,   3,   4,   5,
        6,   7,   8,   9,   10,  12,  13,  15,  16,  17,  18,  19,
        20,  21,  22,  23,  25,  26,  27,  28,  29,  30,  31,  32,
        33,  34,  35,  37,  39,  40,  41,  42,  43,  44,  45,  46,
        47,  3,   4,   5,   6,   7,   8,   9,   11,  12,  14,  17,
        18,  20,  24,  25,  26,  27,  28,  29,  30,  33,  34,  36,
        39,  40,  41,  44,  46,  0,   4,   5,   6,   12,  16,  19,
        21,  23,  24,  30,  33,  34,  38,  41,  44,  47,  49,  1,
        2,   4,   5,   6,   7,   8,   11,  13,  16,  17,  20,  23,
        25,  26,  28,  29,  30,  31,  32,  33,  38,  39,  40,  42,
        43,  45,  46,  47,  48,  49,  50,  2,   4,   6,   7,   9,
        11,  14,  15,  16,  17,  18,  21,  22,  25,  26,  27,  28,
        29,  30,  33,  36,  37,  40,  41,  42,  43,  44,  50,  51,
        0,   3,   8,   10,  12,  14,  16,  18,  22,  23,  24,  25,
        26,  28,  29,  30,  31,  32,  34,  36,  38,  39,  40,  41,
        44,  45,  47,  48,  50,  51,  0,   1,   2,   3,   4,   5,
        6,   8,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
        20,  21,  22,  24,  25,  26,  27,  31,  33,  34,  35,  36,
        37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
        49,  50,  51,  52,  53,  0,   2,   3,   4,   5,   6,   8,
        9,   10,  11,  13,  14,  16,  17,  18,  21,  23,  24,  25,
        26,  28,  29,  32,  36,  37,  39,  40,  41,  48,  51,  53,
        54,  17,  23,  35,  54,  1,   2,   5,   7,   8,   13,  14,
        15,  16,  21,  23,  24,  25,  33,  35,  36,  39,  40,  42,
        46,  48,  49,  51,  52,  54,  5,   7,   12,  15,  23,  33,
        34,  36,  37,  44,  50,  56,  1,   3,   4,   7,   8,   9,
        10,  12,  13,  15,  16,  17,  18,  19,  22,  23,  24,  25,
        26,  27,  29,  31,  32,  34,  35,  36,  37,  39,  40,  41,
        42,  43,  44,  46,  47,  50,  51,  52,  53,  54,  55,  56,
        57,  3,   5,   6,   9,   11,  12,  18,  20,  21,  22,  25,
        29,  30,  32,  34,  35,  38,  45,  48,  50,  55,  59,  1,
        3,   6,   8,   10,  11,  12,  13,  19,  21,  22,  26,  28,
        31,  34,  35,  37,  40,  41,  45,  47,  49,  50,  57,  58,
        0,   1,   5,   6,   7,   8,   13,  15,  16,  17,  18,  22,
        23,  24,  26,  29,  32,  36,  38,  39,  40,  41,  42,  47,
        48,  52,  55,  61,  0,   5,   7,   8,   10,  12,  15,  16,
        23,  31,  35,  36,  40,  42,  43,  45,  47,  49,  53,  55,
        62,  0,   2,   3,   4,   5,   7,   8,   9,   11,  12,  14,
        16,  17,  18,  19,  21,  22,  23,  25,  26,  27,  28,  29,
        31,  32,  34,  35,  36,  37,  38,  39,  41,  43,  44,  48,
        49,  50,  51,  52,  54,  55,  56,  58,  59,  61,  62,  63,
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   11,  12,
        13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
        25,  26,  28,  29,  30,  31,  32,  33,  34,  35,  38,  39,
        40,  42,  43,  46,  47,  48,  49,  50,  51,  52,  53,  54,
        55,  56,  57,  58,  59,  60,  62,  64,  0,   3,   5,   6,
        15,  28,  30,  50,  54,  56,  64,  65,  0,   1,   3,   4,
        5,   7,   8,   10,  12,  13,  14,  17,  19,  23,  24,  26,
        27,  28,  29,  30,  31,  32,  33,  35,  36,  37,  38,  40,
        41,  43,  44,  45,  46,  47,  48,  49,  50,  52,  53,  60,
        61,  64,  65,  66,  8,   12,  41,  0,   2,   3,   4,   5,
        6,   7,   9,   10,  11,  12,  13,  14,  16,  17,  18,  19,
        20,  21,  23,  26,  27,  28,  29,  30,  31,  32,  34,  35,
        36,  37,  38,  39,  40,  42,  43,  45,  46,  47,  48,  49,
        50,  52,  53,  54,  55,  56,  57,  58,  59,  60,  62,  63,
        64,  65,  67,  68,  0,   1,   2,   5,   6,   8,   9,   10,
        11,  12,  15,  17,  18,  20,  21,  23,  24,  25,  27,  28,
        31,  32,  33,  35,  37,  39,  41,  43,  45,  46,  48,  49,
        52,  54,  55,  56,  58,  60,  62,  63,  67,  0,   1,   2,
        3,   5,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,
        17,  18,  20,  21,  22,  25,  26,  29,  31,  32,  33,  34,
        35,  36,  37,  39,  40,  41,  42,  46,  47,  50,  51,  52,
        53,  54,  56,  57,  58,  60,  61,  64,  68,  70,  8,   10,
        11,  18,  20,  21,  22,  28,  29,  35,  41,  42,  49,  50,
        51,  52,  56,  60,  1,   2,   3,   4,   5,   6,   7,   8,
        9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,
        21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
        33,  34,  35,  37,  38,  39,  40,  41,  42,  43,  44,  45,
        46,  47,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
        59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,
        71,  72,  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
        10,  11,  12,  14,  15,  16,  17,  18,  21,  22,  23,  24,
        25,  26,  27,  28,  29,  30,  32,  33,  34,  35,  37,  38,
        39,  40,  41,  43,  44,  46,  47,  48,  50,  51,  52,  53,
        54,  55,  56,  57,  58,  60,  61,  62,  63,  64,  65,  66,
        67,  68,  69,  70,  71,  72,  0,   1,   2,   3,   4,   6,
        7,   9,   10,  11,  12,  13,  14,  15,  18,  19,  22,  23,
        24,  25,  26,  27,  29,  30,  31,  33,  34,  35,  36,  37,
        38,  40,  42,  43,  44,  46,  47,  48,  49,  50,  53,  54,
        55,  56,  57,  59,  60,  61,  62,  64,  65,  66,  67,  69,
        70,  72,  73,  74,  1,   5,   6,   9,   10,  11,  15,  21,
        22,  23,  24,  26,  27,  29,  32,  33,  34,  35,  37,  38,
        44,  46,  47,  51,  53,  54,  55,  56,  59,  60,  61,  62,
        63,  67,  71,  72,  74,  0,   1,   2,   3,   4,   5,   6,
        7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,
        19,  20,  22,  23,  24,  25,  26,  28,  29,  30,  32,  33,
        34,  35,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,
        47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
        60,  61,  62,  63,  64,  65,  66,  67,  69,  70,  71,  72,
        73,  74,  75,  76,  0,   3,   4,   6,   8,   9,   11,  13,
        14,  15,  16,  23,  25,  26,  29,  31,  32,  33,  34,  37,
        40,  43,  47,  49,  50,  51,  52,  54,  59,  62,  63,  64,
        67,  70,  75,  77,  6,   7,   12,  13,  16,  23,  25,  26,
        27,  29,  32,  37,  38,  42,  56,  57,  59,  61,  62,  63,
        65,  67,  70,  73,  76,  77,  0,   1,   2,   3,   4,   7,
        8,   9,   10,  11,  12,  13,  14,  15,  17,  18,  19,  20,
        21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
        33,  34,  35,  36,  38,  39,  41,  42,  43,  44,  46,  47,
        48,  49,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
        61,  62,  63,  64,  65,  67,  69,  70,  71,  72,  73,  74,
        75,  77,  78,  79,  7,   12,  22,  28,  38,  44,  49,  59,
        63,  68,  74,  1,   2,   5,   10,  12,  14,  15,  17,  18,
        19,  21,  22,  24,  25,  27,  29,  31,  32,  39,  40,  43,
        45,  51,  52,  54,  55,  57,  58,  60,  62,  65,  67,  72,
        74,  75,  77,  78,  80,  0,   1,   2,   3,   5,   6,   7,
        8,   10,  11,  12,  13,  15,  16,  18,  19,  23,  24,  25,
        26,  27,  28,  30,  31,  32,  34,  35,  36,  37,  38,  39,
        40,  42,  43,  44,  46,  47,  48,  49,  50,  52,  53,  54,
        56,  57,  60,  61,  62,  63,  64,  65,  67,  68,  69,  70,
        71,  73,  76,  77,  78,  79,  80,  81,  82,  0,   1,   2,
        3,   5,   6,   8,   9,   10,  11,  12,  13,  14,  16,  19,
        20,  21,  23,  25,  26,  27,  28,  30,  31,  33,  34,  37,
        38,  40,  42,  43,  44,  45,  46,  48,  50,  51,  52,  53,
        54,  55,  57,  58,  59,  61,  62,  63,  64,  65,  66,  68,
        69,  70,  71,  75,  76,  77,  79,  80,  81,  82,  83,  1,
        2,   4,   5,   8,   9,   11,  16,  19,  20,  24,  33,  34,
        36,  37,  39,  45,  48,  54,  56,  61,  63,  69,  70,  73,
        80,  81,  18,  20,  22,  26,  39,  47,  50,  61,  64,  67,
        70,  72,  75,  79,  1,   16,  24,  26,  36,  39,  46,  50,
        54,  56,  66,  69,  75,  82,  84,  85,  86,  2,   3,   4,
        5,   6,   7,   8,   10,  11,  13,  14,  16,  17,  18,  21,
        22,  23,  24,  25,  26,  27,  28,  30,  31,  33,  35,  36,
        37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
        49,  50,  52,  54,  55,  58,  59,  60,  63,  64,  65,  66,
        67,  68,  69,  70,  73,  74,  77,  78,  79,  80,  81,  85,
        86,  0,   1,   6,   7,   10,  11,  12,  13,  16,  20,  21,
        23,  32,  36,  37,  38,  42,  45,  47,  50,  55,  58,  59,
        62,  63,  68,  69,  73,  79,  84,  87,  0,   1,   2,   4,
        9,   10,  11,  12,  13,  14,  16,  17,  20,  22,  23,  24,
        26,  27,  28,  29,  30,  31,  32,  33,  36,  37,  39,  40,
        42,  43,  44,  46,  47,  48,  49,  50,  51,  54,  55,  56,
        57,  58,  60,  61,  62,  64,  65,  66,  68,  69,  71,  72,
        73,  74,  75,  76,  78,  79,  80,  81,  83,  85,  86,  87,
        89,  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   11,
        12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,
        24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,
        36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
        48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  60,
        61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,
        73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,
        85,  86,  87,  88,  89,  90,  1,   5,   9,   11,  12,  13,
        14,  22,  23,  24,  25,  26,  29,  31,  36,  37,  46,  47,
        49,  50,  51,  52,  57,  58,  60,  66,  70,  79,  81,  82,
        83,  84,  88,  89,  0,   1,   2,   3,   5,   6,   11,  12,
        13,  14,  18,  20,  22,  23,  25,  26,  27,  28,  31,  32,
        35,  37,  38,  39,  40,  41,  42,  43,  44,  45,  47,  49,
        52,  54,  56,  58,  59,  60,  62,  63,  64,  66,  67,  69,
        71,  73,  75,  76,  77,  78,  80,  81,  83,  84,  85,  89,
        90,  92,  0,   3,   4,   5,   6,   7,   8,   9,   11,  12,
        14,  15,  17,  18,  19,  20,  21,  24,  25,  26,  30,  31,
        34,  35,  37,  38,  39,  40,  42,  44,  46,  49,  50,  52,
        53,  54,  55,  59,  61,  62,  63,  65,  68,  69,  71,  72,
        73,  74,  75,  76,  78,  79,  80,  81,  82,  84,  85,  86,
        87,  91,  92,  2,   3,   4,   7,   11,  20,  27,  34,  55,
        65,  69,  70,  75,  76,  79,  80,  84,  91,  1,   4,   11,
        12,  14,  19,  24,  28,  31,  33,  35,  40,  45,  46,  53,
        55,  57,  63,  66,  73,  74,  92,  93,  95,  1,   2,   4,
        12,  13,  14,  18,  19,  20,  26,  27,  29,  31,  36,  42,
        43,  48,  50,  53,  57,  62,  64,  70,  72,  73,  75,  76,
        80,  83,  86,  89,  92,  93,  94,  96,  0,   1,   2,   3,
        4,   5,   6,   8,   9,   10,  11,  13,  14,  15,  16,  17,
        18,  19,  20,  22,  23,  24,  25,  26,  27,  28,  29,  30,
        31,  33,  34,  35,  36,  37,  38,  39,  40,  42,  43,  44,
        45,  46,  47,  48,  49,  52,  53,  54,  55,  56,  57,  58,
        59,  60,  62,  63,  64,  66,  67,  68,  70,  71,  72,  73,
        74,  75,  76,  77,  78,  79,  80,  82,  83,  84,  85,  86,
        88,  89,  91,  92,  94,  95,  96,  97,  0,   8,   10,  14,
        15,  17,  26,  27,  50,  52,  53,  54,  61,  65,  67,  70,
        75,  79,  81,  82,  86,  90,  92,  96,  98,  2,   3,   7,
        37,  43,  87,  90,  97,  98,  99,  0,   1,   2,   4,   5,
        7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,
        19,  20,  22,  23,  24,  25,  26,  27,  28,  30,  33,  34,
        35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,
        47,  48,  49,  51,  53,  54,  55,  56,  57,  59,  60,  61,
        62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,
        74,  75,  77,  79,  80,  81,  82,  83,  84,  85,  86,  87,
        88,  89,  90,  91,  93,  94,  95,  97,  98,  99,  100, 2,
        3,   4,   5,   6,   7,   9,   10,  11,  12,  14,  15,  17,
        19,  20,  21,  23,  24,  25,  26,  27,  28,  29,  30,  31,
        33,  35,  36,  37,  38,  39,  40,  42,  43,  44,  45,  46,
        47,  48,  49,  50,  51,  52,  53,  55,  56,  57,  59,  61,
        62,  63,  64,  65,  68,  69,  70,  71,  72,  73,  75,  77,
        78,  79,  80,  81,  82,  84,  86,  87,  90,  91,  92,  94,
        95,  96,  97,  98,  99,  100, 101, 1,   3,   5,   6,   7,
        8,   9,   10,  14,  15,  16,  17,  18,  19,  21,  23,  24,
        26,  27,  28,  31,  32,  33,  34,  35,  36,  37,  38,  39,
        40,  43,  45,  46,  47,  48,  49,  50,  51,  53,  54,  55,
        56,  57,  58,  59,  60,  62,  63,  64,  65,  67,  68,  69,
        70,  72,  73,  74,  75,  77,  78,  79,  82,  84,  85,  87,
        93,  94,  95,  96,  99,  100, 101, 102, 0,   1,   3,   4,
        6,   7,   8,   11,  12,  13,  14,  15,  16,  17,  20,  21,
        22,  24,  25,  26,  27,  28,  29,  30,  31,  33,  34,  35,
        36,  38,  40,  41,  42,  44,  45,  46,  47,  48,  53,  55,
        56,  57,  60,  61,  62,  64,  65,  66,  67,  68,  73,  75,
        77,  80,  81,  84,  85,  86,  87,  88,  89,  91,  92,  93,
        95,  96,  98,  99,  100, 102, 103, 1,   2,   3,   6,   8,
        9,   10,  11,  13,  14,  15,  16,  17,  18,  19,  20,  21,
        22,  24,  25,  27,  28,  29,  30,  31,  32,  33,  35,  36,
        37,  38,  39,  40,  41,  42,  43,  44,  45,  47,  48,  51,
        52,  54,  57,  58,  59,  60,  61,  62,  64,  65,  66,  68,
        69,  70,  72,  73,  75,  77,  79,  81,  82,  83,  85,  87,
        89,  90,  91,  92,  93,  94,  95,  96,  98,  99,  100, 101,
        102, 103, 104, 0,   2,   3,   6,   7,   8,   9,   11,  12,
        13,  14,  15,  16,  17,  18,  19,  21,  22,  23,  24,  26,
        27,  28,  29,  31,  33,  34,  35,  36,  37,  40,  41,  44,
        45,  46,  47,  48,  49,  50,  52,  53,  54,  55,  56,  57,
        58,  59,  60,  61,  62,  63,  64,  65,  67,  68,  69,  70,
        71,  72,  73,  74,  76,  77,  79,  80,  81,  82,  84,  85,
        86,  87,  89,  90,  91,  92,  93,  94,  98,  100, 101, 102,
        104, 105, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
        11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
        23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,
        35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,
        47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
        59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,
        71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,
        83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,
        95,  96,  97,  98,  99,  101, 102, 103, 104, 105, 106, 0,
        1,   2,   4,   5,   6,   7,   8,   9,   10,  12,  13,  14,
        16,  19,  20,  22,  23,  24,  27,  28,  29,  30,  31,  33,
        34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,
        46,  47,  48,  49,  51,  52,  53,  54,  55,  56,  57,  59,
        60,  61,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,
        73,  75,  76,  77,  78,  79,  80,  81,  82,  83,  85,  87,
        88,  90,  91,  93,  94,  95,  96,  97,  98,  99,  100, 101,
        102, 103, 105, 106, 107, 3,   12,  13,  38,  43,  45,  46,
        48,  49,  57,  64,  67,  74,  79,  83,  88,  91,  100, 108,
        0,   2,   3,   6,   8,   11,  12,  13,  17,  18,  20,  21,
        23,  25,  27,  28,  29,  31,  32,  33,  34,  35,  37,  38,
        40,  42,  44,  45,  47,  48,  49,  50,  51,  52,  54,  55,
        56,  58,  59,  60,  61,  62,  63,  66,  68,  70,  72,  73,
        75,  76,  77,  78,  79,  80,  81,  83,  84,  85,  86,  87,
        90,  94,  95,  96,  97,  98,  100, 101, 105, 106, 107, 109,
        3,   5,   9,   22,  24,  45,  57,  59,  61,  76,  79,  80,
        84,  91,  94,  99,  100, 102, 104, 0,   3,   4,   5,   6,
        7,   8,   11,  12,  13,  14,  15,  17,  18,  19,  20,  22,
        23,  24,  25,  27,  28,  29,  30,  31,  32,  36,  37,  41,
        43,  44,  45,  48,  49,  51,  52,  54,  55,  56,  57,  58,
        60,  61,  62,  63,  64,  65,  66,  67,  70,  71,  72,  73,
        74,  75,  77,  79,  80,  81,  82,  87,  88,  89,  90,  92,
        93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 105, 108,
        109, 110, 111, 0,   3,   5,   7,   10,  12,  14,  20,  21,
        23,  27,  28,  30,  31,  36,  40,  45,  48,  52,  53,  55,
        56,  58,  59,  61,  62,  64,  65,  72,  74,  77,  78,  79,
        80,  81,  82,  83,  85,  87,  88,  89,  95,  96,  97,  100,
        104, 105, 106, 109, 110, 0,   1,   2,   3,   4,   5,   6,
        7,   8,   9,   10,  11,  12,  14,  16,  17,  18,  19,  21,
        22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,
        34,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,
        47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
        59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,
        71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,
        83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,
        95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106,
        107, 108, 109, 110, 111, 112, 113, 4,   18,  40,  82,  87,
        91,  92,  106, 0,   1,   3,   5,   7,   10,  11,  12,  13,
        15,  16,  17,  18,  20,  21,  22,  23,  24,  25,  27,  28,
        30,  32,  33,  35,  40,  41,  42,  48,  50,  51,  52,  53,
        56,  58,  59,  65,  67,  68,  69,  70,  72,  73,  76,  77,
        78,  79,  81,  82,  85,  87,  88,  89,  90,  91,  92,  94,
        96,  98,  99,  100, 101, 102, 103, 105, 106, 107, 111, 113,
        114, 1,   5,   7,   8,   18,  20,  25,  33,  36,  37,  42,
        55,  59,  61,  63,  70,  75,  79,  80,  83,  85,  89,  90,
        99,  102, 114, 115, 116, 0,   6,   14,  24,  32,  33,  43,
        51,  54,  69,  71,  88,  89,  90,  93,  102, 111, 113, 114,
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,
        12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,
        24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,
        36,  37,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
        50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,
        62,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
        76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
        88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  100,
        101, 102, 103, 104, 105, 106, 107, 108, 109, 111, 112, 113,
        114, 115, 116, 117};

    std::vector<span<int16>> available_agents
        = [&]() -> std::vector<span<int16>> {
        auto* h = available_agents_data.data();
        const auto n = [&](auto s) {
            return span<int16>{std::exchange(h, std::next(h, s)), s};
        };
        return {n(9),  n(4), n(6),  n(10), n(7), n(2),  n(7),  n(9),
                n(4),  n(1), n(4),  n(3),  n(7), n(10), n(1),  n(8),
                n(6),  n(2), n(7),  n(3),  n(5), n(5),  n(3),  n(6),
                n(8),  n(1), n(3),  n(4),  n(3), n(5),  n(10), n(7),
                n(3),  n(1), n(7),  n(2),  n(7), n(7),  n(1),  n(5),
                n(8),  n(7), n(6),  n(6),  n(4), n(1),  n(8),  n(1),
                n(9),  n(7), n(2),  n(1),  n(4), n(2),  n(9),  n(3),
                n(10), n(5), n(6),  n(8),  n(3), n(10), n(9),  n(4),
                n(2),  n(9), n(6),  n(10), n(5), n(3),  n(10), n(4),
                n(10), n(3), n(4),  n(3),  n(8), n(6),  n(1),  n(8),
                n(10), n(7), n(8),  n(10), n(7), n(3),  n(5),  n(5),
                n(1),  n(3), n(6),  n(3),  n(5), n(6),  n(10), n(6),
                n(4),  n(9), n(5),  n(8),  n(5), n(7),  n(6),  n(8),
                n(7),  n(5), n(8),  n(6),  n(4), n(3),  n(4),  n(4),
                n(10), n(8), n(10), n(9),  n(4), n(6),  n(8),  n(3)};
    }();

    std::vector<span<int16>> dependencies
        = [&]() -> std::vector<span<int16>> {
        auto* h = dependencies_data.data();
        const auto n = [&](auto s) {
            return span<int16>{std::exchange(h, std::next(h, s)), s};
        };
        return {n(0),  n(0),  n(0),   n(2),  n(1),  n(3),  n(3),
                n(6),  n(1),  n(7),   n(1),  n(9),  n(4),  n(11),
                n(3),  n(6),  n(4),   n(14), n(0),  n(10), n(13),
                n(0),  n(22), n(12),  n(3),  n(24), n(6),  n(25),
                n(1),  n(26), n(16),  n(19), n(13), n(18), n(24),
                n(2),  n(22), n(5),   n(36), n(24), n(37), n(31),
                n(9),  n(39), n(7),   n(20), n(29), n(15), n(42),
                n(28), n(18), n(32),  n(29), n(30), n(47), n(32),
                n(4),  n(25), n(12),  n(43), n(22), n(25), n(28),
                n(21), n(47), n(56),  n(12), n(44), n(3),  n(57),
                n(41), n(49), n(18),  n(70), n(64), n(58), n(37),
                n(71), n(36), n(26),  n(70), n(11), n(38), n(64),
                n(62), n(27), n(14),  n(17), n(64), n(31), n(65),
                n(89), n(34), n(58),  n(61), n(18), n(24), n(35),
                n(84), n(25), n(10),  n(88), n(80), n(73), n(71),
                n(80), n(83), n(105), n(90), n(19), n(72), n(19),
                n(80), n(50), n(110), n(8),  n(70), n(28), n(19),
                n(112)};
    }();

    ScheduleInfo info{
        .agent_performance{
            1.1638119543227283F,
            0.6737987296001547F,
            0.8055083549776275F,
            0.6950162407210337F,
            1.4894348466812746F,
            0.7519534213945868F,
            1.054570662242845F,
            0.8152891315114477F,
            1.4253600804568045F,
            0.7666303117557152F},
        .task_duration{0.05285533523509549F,  0.09637112257561846F,
                       0.0576002998340268F,   0.1421620312802443F,
                       0.32502146062831F,     0.17721302625293892F,
                       0.34598058017611333F,  0.1584906929293631F,
                       0.06850123152692925F,  0.307324713744863F,
                       0.36181292335930043F,  0.02919870348233573F,
                       0.127881853524176F,    0.4373119335503231F,
                       0.09982117241333775F,  0.054395927978122705F,
                       0.10522473678384753F,  0.3981876278519215F,
                       0.2614195765739074F,   0.43271706720681546F,
                       0.4920262876722579F,   0.16638913580990297F,
                       0.23557711332840356F,  0.3396515151719267F,
                       0.06434445368583319F,  0.35291894789606687F,
                       0.2796516480838109F,   0.12415235479743286F,
                       0.31011705730934F,     0.19232161425047029F,
                       0.2974350519088449F,   0.25438100908324196F,
                       0.15975946743512232F,  0.2918932310151993F,
                       0.11681615900792645F,  0.34543196576562774F,
                       0.4133291692933404F,   0.495844576866851F,
                       0.1146405640481416F,   0.28438110022412216F,
                       0.15609831489722978F,  0.4260190487827474F,
                       0.02350871650435618F,  0.08638353576244003F,
                       0.13508705975342383F,  0.10446365136256042F,
                       0.2762661790866425F,   0.46653263623422736F,
                       0.2559707914868141F,   0.3584589819212315F,
                       0.34373963006962693F,  0.09529452632742443F,
                       0.35615946272120425F,  0.15957725423259092F,
                       0.259906887129866F,    0.26209606266134966F,
                       0.4522138105515216F,   0.24276666665039628F,
                       0.20818933139195195F,  0.09838130918205597F,
                       0.12467274223482985F,  0.4249651883802459F,
                       0.3700248260679456F,   0.4469594012317944F,
                       0.052317526034965894F, 0.41312759080517386F,
                       0.4884228792392867F,   0.09018089829882932F,
                       0.24243175505940728F,  0.07055546468640857F,
                       0.1560390875413459F,   0.49538693759579994F,
                       0.10327123746931244F,  0.048073728754723F,
                       0.1842339028242139F,   0.43893517899503864F,
                       0.24124775394326808F,  0.41021281505385504F,
                       0.23938631377189792F,  0.1174702332487398F,
                       0.30430333987781943F,  0.33983221390721435F,
                       0.49730497152085124F,  0.19211437190090022F,
                       0.19127378446974866F,  0.11187458298583579F,
                       0.41610753107755155F,  0.15875087663600368F,
                       0.10021792182485367F,  0.16534057016138717F,
                       0.397910102877048F,    0.08768443331460161F,
                       0.0201421015373505F,   0.380319493984765F,
                       0.40831802222374486F,  0.3843816166653113F,
                       0.2208301571620856F,   0.03452116096140765F,
                       0.3294563815972428F,   0.20339967906482853F,
                       0.22043846986895502F,  0.34753712476912907F,
                       0.06505132769696688F,  0.1709833871346954F,
                       0.14027606527112876F,  0.04814418988333945F,
                       0.24661652313225432F,  0.07997082813314224F,
                       0.22741998402727043F,  0.37496599262932295F,
                       0.377961050610707F,    0.24342373990818886F,
                       0.3772716509256642F,   0.31243394344644443F,
                       0.1858450496202924F,   0.022156562782374677F,
                       0.06760149379072283F,  0.2593205021168373F,
                       0.3555895974420166F,   0.1448510732855101F},
        .available_agents{
            std::move(available_agents_data),
            std::move(available_agents)},
        .dependencies{
            std::move(dependencies_data),
            std::move(dependencies)}};

    std::vector<StateItem> state{
        {0, 8},   {1, 7},   {2, 0},   {3, 2},   {4, 0},   {5, 3},
        {6, 1},   {7, 5},   {8, 3},   {9, 6},   {10, 2},  {11, 2},
        {12, 7},  {13, 1},  {14, 3},  {15, 8},  {16, 0},  {17, 0},
        {18, 7},  {19, 7},  {20, 3},  {21, 3},  {22, 4},  {23, 0},
        {24, 2},  {25, 7},  {26, 3},  {27, 6},  {28, 1},  {29, 4},
        {30, 8},  {31, 0},  {32, 3},  {33, 4},  {34, 4},  {35, 7},
        {36, 8},  {37, 7},  {38, 6},  {39, 2},  {40, 8},  {41, 1},
        {42, 5},  {43, 0},  {44, 5},  {45, 7},  {46, 1},  {47, 8},
        {48, 5},  {49, 7},  {50, 2},  {51, 9},  {52, 1},  {53, 6},
        {54, 7},  {55, 8},  {56, 2},  {57, 8},  {58, 0},  {59, 8},
        {60, 3},  {61, 5},  {62, 1},  {63, 5},  {64, 8},  {65, 6},
        {66, 9},  {67, 2},  {68, 8},  {69, 4},  {70, 5},  {71, 4},
        {72, 1},  {73, 7},  {74, 7},  {75, 2},  {76, 9},  {77, 4},
        {78, 4},  {79, 5},  {80, 1},  {81, 8},  {82, 9},  {83, 4},
        {84, 1},  {85, 4},  {86, 2},  {87, 7},  {88, 2},  {89, 5},
        {90, 4},  {91, 6},  {92, 4},  {93, 6},  {94, 7},  {95, 6},
        {96, 8},  {97, 8},  {98, 4},  {99, 6},  {100, 2}, {101, 8},
        {102, 4}, {103, 2}, {104, 1}, {105, 2}, {106, 1}, {107, 4},
        {108, 8}, {109, 5}, {110, 6}, {111, 6}, {112, 3}, {113, 3},
        {114, 3}, {115, 5}, {116, 6}, {117, 5}, {118, 2}, {119, 1}};

    float beta = 1.0F;
    for (int i{0}; i < 10; ++i) {

        RandomUtils random_utils;
        Mutator mutator{info, random_utils};
        Makespan makespan{info};
        Temperature temperature{
            Beta{beta},
            BetaScale{1e-4f},
            StunWindow{10000}};
        auto r = stochastic_tunneling(
            state,
            STUNOptions{
                .mutator{&mutator},
                .random{&random_utils},
                .makespan{&makespan},
                .temp{&temperature},
                .gamma{.5F}});
        state = std::move(r.state);
        beta = r.temperature;
    }
    fmt::print("{}\n", state);
}
} // namespace angonoka::stun_dag

namespace {
struct Options {
};
} // namespace

int main(int /*unused*/, char** /*unused*/)
{
    using namespace clipp;
    // TODO: Add a test STUN invocation.

    // Options options;

    // group cli{value("input file", options.filename)};
    //
    // if (!parse(argc, argv, cli)) {
    //     fmt::print("{}", make_man_page(cli, *argv));
    //     return 1;
    // }

    angonoka::stun_dag::run();
    return 0;
}
