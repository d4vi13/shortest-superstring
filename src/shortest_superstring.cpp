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
compute_shortest_superstring (std::vector<std::string> strs, uint32_t  **overlaps)
{
  std::vector<bool> used_as_prefix(strs.size(), false);
  std::vector<bool> used_as_suffix(strs.size(), false);
  uint32_t total_used, i, j, overlap, *sequence;
  std::tuple<uint32_t, uint32_t, uint32_t> t;
  std::string tmp = "";

  sequence = (uint32_t *) calloc (1, strs.size() * sizeof *sequence); 
  if (!sequence)
    return tmp;

  std::cout << "Starting to compute superstring" << std::endl;
  total_used = 0;
  i = 0;

  std::priority_queue<std::tuple<uint32_t, uint32_t, uint32_t>> pq;
  for (uint32_t i = 0; i < strs.size(); i++)
    for (uint32_t j = 0; j < strs.size(); j++)
      if (i != j)
      {
        std::cout << overlaps[i][j] << " " << strs[i] << " " << i << " " << strs[j] << " " << j << std::endl;
        pq.push({overlaps[i][j], i, j});
      }
  std::cout << "map above" << std::endl;
  
  // assemble string
  while (total_used < (strs.size() - 1))
    {
      t = pq.top(); 
      i = std::get<1>(t); 
      j = std::get<2>(t); 
      overlap = std::get<0>(t);

      if (!used_as_prefix[i] && !used_as_suffix[j])
        {
          used_as_prefix[i] = true;
          used_as_suffix[j] = true;
          
          sequence[i] = j;
          /*
          tmp = strs[i] + strs[j].substr(overlap , strs[j].size() - overlap);
         
          std::cout << strs[i] << std::endl;
          std::cout << strs[j] << std::endl;
          std::cout << tmp << " " << i << " " << j << " " << overlap << " " << total_used << std::endl;

          strs[i] = std::move(tmp); // move ownership of buffer 
          strs[j] = strs[i]; // make a copy
                             // */
          
          total_used += 1;
        }

      pq.pop();
    } 

  for (int i = 0; i < strs.size(); i++)
  {
    //std::cout << used_as_prefix[i] << " " << used_as_suffix[i] << std::endl;
    std::cout << i << " " <<  sequence[i] << " " << overlaps[i][sequence[i]] << std::endl;
    //std::cout << strs[i] << " " << used_as_prefix[i] << " " << used_as_suffix[i] << std::endl;
  }
  std::cout << strs[i] << std::endl;
  return strs[i];
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
