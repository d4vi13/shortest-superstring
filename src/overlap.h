#ifndef OVERLAP_H
#define OVERLAP_H

#include "common.h"

uint32_t calculate_overlap (std::string a, std::string b);

std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t> >
compute_overlap_matrix (std::unordered_map<uint32_t, std::string> &strs,
                        std::set<uint32_t> working_set);

#endif
