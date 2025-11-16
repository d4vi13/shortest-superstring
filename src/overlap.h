#ifndef OVERLAP_H
#define OVERLAP_H

#include "common.h"

uint32_t calculate_overlap (std::string a, std::string b);

std::map<uint32_t, std::map<uint32_t, uint32_t> >
compute_overlap_matrix (std::vector<std::string> strs,
                        std::set<uint32_t> working_set);

#endif
