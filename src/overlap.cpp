#include "overlap.h"

static inline std::vector<uint32_t>
prefix_function (std::string a)
{
  std::vector<uint32_t> prefixes (a.size (), 0);

  for (uint32_t i = 1; i < a.size (); i++)
    {
      uint32_t j = prefixes[i - 1];
      while ((j > 0) && (a[i] != a[j]))
        j = prefixes[j - 1];

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
  std::vector<uint32_t> p = prefix_function (b + "#" + a);
  return p.back ();
}

std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t> >
compute_overlap_matrix (std::unordered_map<uint32_t, std::string> &strs,
                        std::set<uint32_t> working_set)
{
  double start, end;
  std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t> > overlap;

  /* Make array out of set to allow parallelization since
   * its not possible to use parralel for primitve with
   * iterators that are not random-acess like the set
   */
  std::vector<uint32_t> working_set_vector (working_set.begin (),
                                            working_set.end ());

  start = omp_get_wtime ();
#pragma omp parallel for schedule(dynamic)
  for (auto i = working_set_vector.begin (); i < working_set_vector.end ();
       i++)
    {
      overlap[*i] = std::unordered_map<uint32_t, uint32_t> ();
      for (uint32_t j = 0; j < strs.size (); j++)
        {
          if (*i == j)
            overlap[*i][j] = strs[*i].size ();
          else
            overlap[*i][j] = calculate_overlap (strs[*i], strs[j]);
        }
    }
  end = omp_get_wtime ();
  ptotal += end - start;

  return overlap;
}
