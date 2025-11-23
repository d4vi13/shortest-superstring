#include "overlap.h"

static inline std::vector<int32_t>
prefix_function (std::string a)
{
  std::vector<int32_t> prefixes (a.size (), 0);

  for (int32_t i = 1; i < a.size (); i++)
    {
      int32_t j = prefixes[i - 1];
      while ((j > 0) && (a[i] != a[j]))
        j = prefixes[j - 1];

      if (a[i] == a[j])
        j++;
      prefixes[i] = j;
    }

  return prefixes;
}

int32_t
calculate_overlap (std::string &a, std::string &b)
{
  // adds space for a case like a=aba b=bab
  std::vector<int32_t> p = prefix_function (b + "#" + a);
  return p.back ();
}

std::vector<std::vector<int32_t>>
compute_overlap_matrix (std::vector<std::string> strs)
{
  double start, end;
  std::vector<std::vector<int32_t>> overlap(ctrl.ws_size, std::vector<int32_t>(strs.size()));

  start = omp_get_wtime();
  #pragma omp parallel for schedule(dynamic)
  for (int32_t i = 0; i < ctrl.ws_size; i++)
    {
      for (int32_t j = 0; j < strs.size(); j++)
        {
          if ((ctrl.ws_start + i) == j)
            overlap[i][j] = -1;
          else {
            overlap[i][j] = calculate_overlap(strs[ctrl.ws_start + i], strs[j]);
          }
        }
    }
  end = omp_get_wtime();
  ptotal += end - start;

  return overlap;
}
