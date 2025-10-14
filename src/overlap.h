#ifndef OVERLAP_H
#define OVERLAP_H

#include "common.h"

std::vector<std::vector<uint32_t>>
compute_overlap_matrix (std::vector<std::string> strs);

uint32_t
calculate_overlap (std::string a, std::string b);

#endif
