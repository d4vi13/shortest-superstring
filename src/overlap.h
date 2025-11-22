#ifndef OVERLAP_H
#define OVERLAP_H

#include "common.h"

int32_t calculate_overlap (std::string &a, std::string &b);


std::vector<std::vector<int32_t>>
compute_overlap_matrix (std::vector<std::string> strs);

#endif
