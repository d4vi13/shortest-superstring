#include "overlap.h"

static inline uint32_t
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
  std::vector<uint32_t> prefixes;
  prefixes.push_back(0);

  for (uint32_t i = 1; i < a.size(); i++)
  {
    int32_t j = prefixes[i-1];
    for (; j >= 0 ; j = prefixes[j-1])
      {
        if (a[j] == a[i])
          {
            prefixes.push_back(j + 1);
            break;
          }
      }
    if (j < 0)
      prefixes.push_back(0);
  }

  return prefixes;
}

static inline uint32_t
calculate_overlap (std::string a, std::string b)
{
  // adds space for a case like a=aba b=bab
  std::vector<uint32_t> p = prefix_function (b + " " +a);
  return p.back(); 
}

uint32_t**
compute_overlap_matrix (std::vector<std::string> strs)
{
  uint32_t **overlap;

  overlap = (uint32_t **) malloc (strs.size() * sizeof *overlap);
  if (!overlap)
    return overlap;

  for (uint32_t i = 0; i < strs.size(); i++)
    {
      overlap[i] = (uint32_t *) malloc (strs.size() * sizeof **overlap);
      if (!overlap[i])
        return (uint32_t **) NULL;
    }
  
  for (uint32_t i = 0; i < strs.size(); i++)
    {
      for (uint32_t j = 0; j < strs.size(); j++)
        {
          if (i == j)
            continue;
          overlap[i][j] = calculate_overlap(strs[i], strs[j]);
        }
    }

  return overlap;
}

