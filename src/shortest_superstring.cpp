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

void
compute_shortest_superstring (std::vector<std::string> strs, uint32_t  **overlaps)
{
  std::vector<bool> used_as_prefix(strs.size(), false);
  std::vector<bool> used_as_suffix(strs.size(), false);
  uint32_t i_max, j_max, max, j_tmp;
  i_max = 0;
  j_max = 0;
  max = 0;

  auto cmp = [](const triple& a, const triple& b) {
    return std::get<0>(a) < std::get<0>(b);
  };

  std::priority_queue<triple, std::vector<triple>, decltype(cmp)> pq(cmp);
  for (uint32_t i = 0; i < strs.size(); i++)
    for (uint32_t j = 0; j < strs.size(); j++)
      pq.push({matrix[i][j], i, j});

  

  
  std::cout << strs[i_max];
  used[i_max] = true;
  for (uint32_t i = 0; i < strs.size(); i++)
    {
      used[j_max] = true;
      std::cout << strs[j_max].substr(overlaps[i_max][j_max], strs[j_max].size() - overlaps[i_max][j_max]);

      max = 0;
      j_tmp = 0;
      for (uint32_t j = 0; j < strs.size(); j++)
        {
          if (used[j])
            continue;
          if (overlaps[j_max][j] > max)
            {
              max = overlaps[j_max][j];
              j_tmp = j; 
            }
        }

      i_max = j_max;
      j_max = j_tmp;
    }

  std::cout << std::endl;

  return;
}

int 
main(int argc, char **argv) 
{
  std::vector<std::string> strs;
  std::ifstream file;
  std::istream* in = &std::cin;

  uint32_t **overlaps;

  if (argc >= 2)
    {
      file.open(argv[1]);
      if (!file)
        return 1;

      in = &file;
    }

  load_strings (*in, strs);
  overlaps = compute_overlap_matrix(strs);
  compute_shortest_superstring (strs, overlaps);

  for (uint32_t i = 0; i < strs.size(); i++) 
    {
      for (uint32_t j = 0; j < strs.size(); j++)
        std::cout << overlaps[i][j] << " ";
      std::cout << std::endl;
    }

  return 0;
}
