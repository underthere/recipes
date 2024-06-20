#pragma once

#if __has_include(<expected>)
#include <expected>
#define Expected std::expected
#define UnExpect std::unexpect
#define UnExpected std::unexpected
#else
#include <experimental/expected>
#define Expected std::experimental::expected
#endif
