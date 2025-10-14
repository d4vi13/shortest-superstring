#include "shsup.h"

std::string
compute_shortest_superstring(std::vector<std::string> &strs, std::vector<std::vector<uint32_t>> &overlaps) {
  uint32_t start, end, n = strs.size();
  while (strs.size() > 1) {
    n = strs.size();

    uint32_t best_i = 0, best_j = 1,best_ov = 0;

    start = omp_get_wtime();
    #pragma omp parallel
    {
      uint32_t local_i = 0, local_j = 1, local_ov = 0;

      #pragma omp for nowait //collapse(2)
      for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
          if (i == j) continue;
          uint32_t ov = overlaps[i][j];
          if (ov > local_ov) {
            local_ov = ov;
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

    // merge best_i and best_j into best_i
    //std::cout << best_ov << " " << strs[best_i] << " + " << strs[best_j] << std::endl;
    std::string merged = strs[best_i] + strs[best_j].substr(best_ov);
    //std::cout << merged  << std::endl; 
    strs[best_i] = std::move(merged);

    /*
     * use dynamic since the time might vary
     * according with the strings sizez
     */
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

    // erase row best_j
    overlaps.erase(overlaps.begin() + best_j);

    /*
     * use static since each iteration should take
     * the same amount of time
     * erase column best_j from every remaining row
    */
    start = omp_get_wtime();
    #pragma omp parallel for schedule(static)
    for (uint32_t k = 0; k  < overlaps.size(); k++) {
      overlaps[k].erase(overlaps[k].begin() + best_j);
    }   
    end = omp_get_wtime();
    ptotal += end - start;

    // erase string best_j
    strs.erase(strs.begin() + best_j);
  }                                                                                                                                                                                                                                                                                                                          

  //std::cout << strs.front() << std::endl;
  //std::cout << strs.front().size() << std::endl;
  return strs.front();
}

std::string
old_compute_shortest_superstring (std::vector<std::string> &strs, std::vector<std::vector<uint32_t>> &overlaps)
{
  std::vector<bool> used(strs.size(), false);
  uint32_t total_used, i, j, overlap;
  std::tuple<uint32_t, uint32_t, uint32_t> t;
  std::string tmp = "";

  std::cout << "Starting to compute superstring" << std::endl;
  total_used = 0;
  i = 0;

  // do this sequentialy to guarantee coherence 
  std::priority_queue<std::tuple<uint32_t, uint32_t, uint32_t>> pq;
  for (uint32_t i = 0; i < strs.size(); i++)
    for (uint32_t j = 0; j < strs.size(); j++)
      if (i != j)
        pq.push({overlaps[i][j], i, j});

  // assemble string
  while (total_used < (strs.size() - 1))
  {
    t = pq.top(); 
    pq.pop();

    i = std::get<1>(t); 
    j = std::get<2>(t); 
    overlap = std::get<0>(t);

    if (used[i] || used[j])
      continue;

    used[i] = true;
    used[j] = true;

    tmp = strs[i] + strs[j].substr(overlap , strs[j].size() - overlap);
    strs.push_back(std::move(tmp));
    overlaps.push_back(std::move(overlaps[j]));
    for (uint32_t k = 0; k < strs.size() - 1; k++) 
    {
      if (!used[k])
      {
        overlaps[k].push_back(overlaps[k][i]);
        pq.push({overlaps[k][i], k, i});
      }
      pq.push({overlaps[strs.size() - 1][k], strs.size() - 1, k});
    }

    overlaps[strs.size() - 1].push_back(0);
    total_used += 2;

    used.push_back(false);

    std::cout << strs[i] << std::endl;
    std::cout << strs[j] << std::endl;
    std::cout << " " << i << " " << j << " " << overlap << " " << total_used << std::endl;
  } 

  std::cout << strs.back() << std::endl;
  std::cout << strs.back().size() << std::endl;
  return strs.back();
}
