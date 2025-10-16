#include "shsup.h"

std::string
compute_shortest_superstring(std::vector<std::string> &strs, std::vector<std::vector<uint32_t>> &overlaps) {
  uint32_t best_i, best_j, best_ov, start, end, n = strs.size();
  std::string merged;
  while (strs.size() > 1) {
    n = strs.size();
    best_i = 0, best_j = 1,best_ov = 0;

    start = omp_get_wtime();
    #pragma omp parallel
    {
      uint32_t local_i = 0, local_j = 1, local_ov = 0;

      #pragma omp for nowait 
      for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
          if (i == j) continue;
          if (overlaps[i][j] > local_ov) {
            local_ov = overlaps[i][j];
            local_i = i;
            local_j = j;
          }
        }
      }

      #pragma omp critical
      {
        if (local_ov > best_ov) {
          best_ov = local_ov;
          best_i = local_i;
          best_j = local_j;
        } else if (local_ov == best_ov) {
          if (local_i < best_i) {
            best_i = local_i;
            best_j = local_j;
            best_ov = local_ov;
          } else if (local_i == best_i) {
            if (local_j < best_j) {
              best_j = local_j;
              best_i = local_i;
              best_ov = local_ov;
            }
          }
        }
      }
    }
    end = omp_get_wtime();
    ptotal += end - start;

    merged = strs[best_i] + strs[best_j].substr(best_ov);
    strs[best_i] = std::move(merged);

    start = omp_get_wtime();
    #pragma omp parallel for schedule(dynamic)
    for (uint32_t k = 0; k < n; k++) {
      if (k == best_i) continue;
      overlaps[best_i][k] = calculate_overlap(strs[best_i], strs[k]);
      overlaps[k][best_i] = calculate_overlap(strs[k], strs[best_i]);
    }
    end = omp_get_wtime();
    ptotal += end - start;
    overlaps[best_i][best_i] = 0;

    overlaps.erase(overlaps.begin() + best_j);
    
    start = omp_get_wtime();
    #pragma omp parallel for schedule(static)
    for (uint32_t k = 0; k  < overlaps.size(); k++) {
      overlaps[k].erase(overlaps[k].begin() + best_j);
    }   
    end = omp_get_wtime();
    ptotal += end - start;

    strs.erase(strs.begin() + best_j);
  }                                                                                                                                                                                                                                                                                                                          

  return strs.front();
}

