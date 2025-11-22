#include "shsup.h"

void
find_max_overlap (std::vector<uint32_t> &ws, uint32_t &best_i,
    uint32_t &best_j, uint32_t &best_ov)
{

  uint32_t n = ctrl.strs.size ();

#pragma omp parallel 
  { 

  uint32_t local_i = 0, local_j = 1, local_ov = 0;
#pragma omp for nowait
  for (auto i = ws.begin (); i != ws.end (); i++)
  {
    for (uint32_t j = 0; j < n; j++)
    {
      if (*i == j)
        continue;
      if (ctrl.overlaps[*i][j] > local_ov)
      {
        local_ov = ctrl.overlaps[*i][j];
        local_i = *i;
        local_j = j;
      }
    }
  }

#pragma omp critical
  {
    if (local_ov > best_ov)
    {
      best_ov = local_ov;
      best_i = local_i;
      best_j = local_j;
    }
    else if (local_ov == best_ov)
    {
      if (local_i < best_i)
      {
        best_i = local_i;
        best_j = local_j;
        best_ov = local_ov;
      }
      else if (local_i == best_i)
      {
        if (local_j < best_j)
        {
          best_j = local_j;
          best_i = local_i;
          best_ov = local_ov;
        }
      }
    }
  }
}
}

void
find_max_overlap_across_nodes (uint32_t &best_i,
                  uint32_t &best_j, uint32_t &best_ov)
{
  uint32_t local_i = best_i, local_j = best_j, local_ov = best_ov;

  MPI_Allreduce(&local_ov, &best_ov, 1, MPI_UINT32_T, MPI_MAX, MPI_COMM_WORLD);

  if (best_ov != local_ov) {
    local_i = -1;
    local_j = -1;
  }

  MPI_Allreduce(&local_i, &best_i, 1, MPI_UINT32_T, MPI_MIN, MPI_COMM_WORLD);

  if (best_i != local_i) {
    local_i = -1;
    local_j = -1;
  }

  MPI_Allreduce(&local_j, &best_j, 1, MPI_UINT32_T, MPI_MIN, MPI_COMM_WORLD);
}


std::string
compute_shortest_superstring (
    std::unordered_map<uint32_t, std::string> &strs,
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t> > &overlaps)
{
  uint32_t best_i, best_j, best_ov, start, end;
  bool won;
  std::vector<uint32_t> ws (ctrl.working_set.begin (), ctrl.working_set.end ());

  while (strs.size () > 1 && ws.size()) // think about how to stop
    {
      won = false;
      best_i = 0, best_j = 1, best_ov = 0;

      start = omp_get_wtime ();

      // find maximum of local matrix using openmp
      find_max_overlap (ws, best_i, best_j, best_ov);

      uint32_t backup = best_i;
      find_max_overlap_across_nodes (best_i, best_j, best_ov);

      // check if this thread won and set won to true
      if (backup == best_i) 
        won = true;

      end = omp_get_wtime ();
      ptotal += end - start;

      // compute new string
      strs[best_i] = strs[best_i] + strs[best_j].substr (best_ov);

      start = omp_get_wtime ();
#pragma omp parallel for schedule(dynamic)
      for (auto k = ws.begin(); k != ws.end(); k++)
        {
          if (*k == best_i)
            continue;

          if (won)
            overlaps[best_i][*k] = calculate_overlap (strs[best_i], strs[*k]);

          overlaps[*k][best_i] = calculate_overlap (strs[*k], strs[best_i]);
        }
      end = omp_get_wtime ();
      ptotal += end - start;
      overlaps[best_i][best_i] = 0;

      // remove best_i both from working set and overlap rows
      auto it = overlaps.find(best_j);
      if (it != overlaps.end()) {
        ws.erase(std::find(ws.begin(), ws.end(), best_j));
        overlaps.erase(it);
      }

      // remove from columns
      start = omp_get_wtime ();
#pragma omp parallel for schedule(static)
      for (auto k = ws.begin(); k != ws.end (); k++)
        {
          overlaps[*k].erase (best_j);
        }
      end = omp_get_wtime ();
      ptotal += end - start;

      strs.erase (best_j);
    }

  if ((strs.size() == 1) && ws.size())
    return strs[ws.front ()];
  return std::string();  
}
