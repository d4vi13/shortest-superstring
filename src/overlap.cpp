#include "overlap.h"

uint32_t
old_calculate_overlap (std::string a, std::string b)
{
  uint32_t count = 0;
  for (uint32_t i = 1; i < a.size(); i++) 
    {
      uint32_t old = i;
      for (uint32_t j = 0; j < b.size(); j++)
        {
          if (a[i] != b[j])
            {
              i = old;
              count = 0;
              break;
            }

          count++;
          i++;

          if (i >= a.size())
            goto found;
        }
    }

found:
  return count;
}

static inline std::vector<uint32_t> 
prefix_function (std::string a)
{
  std::vector<uint32_t> prefixes(a.size(), 0);

  for (uint32_t i = 1; i < a.size(); i++)
  {
    uint32_t j = prefixes[i-1];
    while ((j > 0) && (a[i] != a[j]))
        j = prefixes[j-1];

    if (a[i] == a[j])
        j++;
    prefixes[i] = j;
  }

  return prefixes;
}

uint32_t
calculate_overlap (std::string a, std::string b)
{
  // adds space for a case like a=aba b=bab
  std::vector<uint32_t> p = prefix_function (b + "#" +a);
  return p.back(); 
}

std::vector<std::vector<uint32_t>>
compute_overlap_matrix (std::vector<std::string> strs)
{
  std::vector<std::vector<uint32_t>> overlap(strs.size(), std::vector<uint32_t>(strs.size()));

  #pragma omp parallel for schedule(dynamic)
  for (uint32_t i = 0; i < strs.size(); i++)
    {
      for (uint32_t j = 0; j < strs.size(); j++)
        {
          if (i == j)
            overlap[i][j] = strs[i].size();
          else
            overlap[i][j] = calculate_overlap(strs[i], strs[j]);
        }
    }

  return overlap;
}

