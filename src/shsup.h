#ifndef SHSUP_H
#define SHSUP_H

#include "common.h"
#include "overlap.h"

std::string
compute_shortest_superstring (std::vector<std::string> &strs, std::vector<std::vector<uint32_t>> &overlaps);

#endif
