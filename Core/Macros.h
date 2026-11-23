#pragma once

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

// TODO_MATHS_LIB: replace with a function
#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))