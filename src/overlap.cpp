#include "overlap.h"

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
  double start, end;
  std::vector<std::vector<uint32_t>> overlap(strs.size(), std::vector<uint32_t>(strs.size()));

  start = omp_get_wtime();
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
  end = omp_get_wtime();
  ptotal += end - start;

  return overlap;
}

