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
compute_shortest_superstring (std::vector<std::string> strs, std::vector<std::vector<uint32_t>> overlaps)
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
      strs.push_back(tmp);
      overlaps.push_back(overlaps[j]);
      for (uint32_t k = 0; k < strs.size() - 1; k++) 
      {
        overlaps[k].push_back(overlaps[k][i]);
        pq.push({overlaps[strs.size() - 1][k], strs.size() - 1, k});
        pq.push({overlaps[k][i], k, i});
      }

      overlaps[strs.size()].push_back(0);
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
