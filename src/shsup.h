#ifndef SHSUP_H
#define SHSUP_H

#include "common.h"
#include "overlap.h"

std::string
compute_shortest_superstring (std::unordered_map<uint32_t, std::string> &strs,
                              std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t> > &overlaps);

#endif
