#include "common.h"
#include "overlap.h"

void 
load_strings (std::istream& in, std::vector<std::string>& strs)
{
  std::string aux;
  while (getline (in, aux)) 
    {
      if (aux.empty())
        break;
      strs.push_back (aux);
    }

  return;
}

std::string
old_compute_shortest_superstring (std::vector<std::string> strs, std::vector<std::vector<uint32_t>> overlaps)
{
  std::vector<bool> used(strs.size(), false);
  uint32_t total_used, i, j, overlap, *sequence;
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

std::string
compute_shortest_superstring(std::vector<std::string> &strs, std::vector<std::vector<uint32_t>> &overlaps) {
  size_t n = strs.size();
  while (strs.size() > 1) {
    n = strs.size();

    // find best pair (i,j) with max overlaps[i][j], i != j
    size_t best_i = 0, best_j = 1;
    uint32_t best_ov = 0;
    for (size_t i = 0; i < n; ++i) {
      for (size_t j = 0; j < n; ++j) {
        if (i == j) continue;
        uint32_t ov = overlaps[i][j];
        if (ov > best_ov) {
          best_ov = ov; 
          best_i = i;
          best_j = j;
        }
      }   
    }   

    // merge best_i and best_j into best_i
    std::string merged = strs[best_i] + strs[best_j].substr(best_ov);
    strs[best_i] = std::move(merged);

    // recompute overlaps for row best_i and column best_i against all k
    // keep using current indices; we'll erase row/col best_j afterwards
    for (size_t k = 0; k < n; ++k) {
      if (k == best_i) {
        overlaps[best_i][best_i] = 0;
        continue;
      }   
      overlaps[best_i][k] = calculate_overlap(strs[best_i], strs[k]);
      overlaps[k][best_i] = calculate_overlap(strs[k], strs[best_i]);
    }   

    // erase row best_j
    overlaps.erase(overlaps.begin() + best_j);
    // erase column best_j from every remaining row
    for (size_t r = 0; r + 0 < overlaps.size(); ++r) {
      overlaps[r].erase(overlaps[r].begin() + best_j);
    }   

    // erase string best_j
    strs.erase(strs.begin() + best_j);
  }                                                                                                                                                                                                                                                                                                                          

  std::cout << strs.front() << std::endl;
  std::cout << strs.front().size() << std::endl;
  return strs.front();
}


  int 
main(int argc, char **argv) 
{
  std::vector<std::string> strs;
  std::ifstream file;
  std::istream* in = &std::cin;

  if (argc >= 2)
    {
      file.open(argv[1]);
      if (!file)
        return 1;

      in = &file;
    }

  load_strings (*in, strs);
  auto overlaps = compute_overlap_matrix(strs);
  compute_shortest_superstring (strs, overlaps);

  /*
  for (uint32_t i = 0; i < strs.size(); i++) 
    {
      for (uint32_t j = 0; j < strs.size(); j++)
        std::cout << overlaps[i][j] << " ";
      std::cout << std::endl;
    }

    */
  return 0;
}
