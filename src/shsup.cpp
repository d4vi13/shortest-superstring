#include "shsup.h"

void
find_max_overlap (uint32_t *pbest_i,
    uint32_t *pbest_j, uint32_t *pbest_ov)
{
  uint32_t best_i, best_j, best_ov;
  uint32_t n = ctrl.strs.size ();

  std::cout << "finding max overlap" << std::endl;
  if (ctrl.ws_size == 0) {
  std::cout << "ws is empty" << std::endl;
    *pbest_i = -1;
    *pbest_j = -1;
    *pbest_ov = 0;
    return;
  }

#pragma omp parallel 
  { 
  uint32_t local_i = 0, local_j = 1, local_ov = 0;
#pragma omp for nowait
  for (auto i = 0; i < ctrl.ws_size; i++)
  {
    for (uint32_t j = 0; j < n; j++)
    {
      if ((ctrl.ws_start + i) == j)
        continue;
      if (ctrl.overlaps[i][j] > local_ov)
      {
        local_ov = ctrl.overlaps[i][j];
        local_i = i;
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

  *pbest_i = best_i;
  *pbest_j = best_j;
  *pbest_ov = best_ov;
}

// after this the best_* varibles will hold the reduce versions, note that i in 
// this contexts points to string not a postion in the matrix
void
find_max_overlap_across_nodes (uint32_t *best_i,
                  uint32_t *best_j, uint32_t *best_ov)
{
  uint32_t local_i = *best_i, local_j = *best_j, local_ov = *best_ov;

  if (ctrl.ws_size)
    local_i += ctrl.ws_start;

  MPI_Allreduce(&local_ov, best_ov, 1, MPI_UINT32_T, MPI_MAX, MPI_COMM_WORLD);

  if (*best_ov != local_ov) {
    local_i = -1;
  }

  MPI_Allreduce(&local_i, best_i, 1, MPI_UINT32_T, MPI_MIN, MPI_COMM_WORLD);

  if (*best_i != local_i) {
    local_j = -1;
  }

  MPI_Allreduce(&local_j, best_j, 1, MPI_UINT32_T, MPI_MIN, MPI_COMM_WORLD);
}

/*
std::string
compute_shortest_superstring () {
  uint32_t best_i, best_j, best_ov, start, end; bool won;

  while (strs.size () > 1) // think about how to stop
    {
      won = false;
      best_i = 0, best_j = 1, best_ov = 0;

      start = omp_get_wtime ();

      // find maximum of local matrix using openmp
      find_max_overlap (ws, &best_i, &best_j, &best_ov);
      std:: cout << "best_i " << best_i << " " << ctrl.strs[best_i] ;
      std:: cout << " best_j " << best_j <<  " " << ctrl.strs[best_j] ;
      std:: cout << " best_ov " << best_ov << std::endl;

      break;

      uint32_t backup = best_i;
      find_max_overlap_across_nodes (&best_i, &best_j, &best_ov);

      if (!ctrl.rank) {
        std:: cout << "best_i " << best_i << " " << ctrl.strs[best_i] <<  std::endl;
        std:: cout << "best_j " << best_j <<  " " << ctrl.strs[best_j] << std::endl;
        std:: cout << "best_ov " << best_ov << std::endl;
      }

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
*/

std::string
compute_shortest_superstring() {
  uint32_t best_i, best_j, best_ov, start, end, n = ctrl.strs.size();
  bool won;

  while (ctrl.strs.size() > 1) {
    n = ctrl.strs.size();
    best_i = 0, best_j = 1,best_ov = 0;

  for (auto i = 0; i < ctrl.ws_size; i++)
  {
    for (uint32_t j = 0; j  < ctrl.strs.size(); j++)
      std::cout << ctrl.overlaps[i][j] << " ";
    std::cout << std::endl;
  }
  std::cout << std::endl;
  std::cout << "----------" << std::endl;
 

    // find max overlap 
    find_max_overlap (&best_i, &best_j, &best_ov);

    if (ctrl.ws_size) {
      std:: cout << "best_i " << best_i << " " << ctrl.strs[best_i] ;
      std:: cout << " best_j " << best_j <<  " " << ctrl.strs[best_j] ;
      std:: cout << " best_ov " << best_ov << std::endl;
    }

    uint32_t backup = best_i;
    find_max_overlap_across_nodes (&best_i, &best_j, &best_ov);
    if (true) {
      std:: cout << "best_i " << best_i << " " << ctrl.strs[best_i];
      std:: cout << " best_j " << best_j <<  " " << ctrl.strs[best_j];
      std:: cout << " best_ov " << best_ov << std::endl;
    }

    // check if this thread won and set won to true
    won = (backup + ctrl.ws_start) == best_i;

    ctrl.strs[best_i] = ctrl.strs[best_i] + ctrl.strs[best_j].substr(best_ov);

    // recompute the line if it belongs to the working set
    if (won) {
    #pragma omp parallel for schedule(dynamic)
      for (uint32_t k = 0; k < n; k++) {
        if (k == best_i) continue;
        ctrl.overlaps[backup][k] = calculate_overlap(ctrl.strs[best_i], ctrl.strs[k]);
      }
    }
    
    // recompute the column of best_i
    #pragma omp parallel for schedule(dynamic)
    for (uint32_t k = 0; k < ctrl.ws_size; k++) {
      ctrl.overlaps[k][best_i] = calculate_overlap(ctrl.strs[k], ctrl.strs[best_i]);
    }

    //remove best_j column
    #pragma omp parallel for schedule(static)
    for (uint32_t k = 0; k  < ctrl.overlaps.size(); k++) {
      ctrl.overlaps[k].erase(ctrl.overlaps[k].begin() + best_j);
    }

    // if best_j belongs to the working set remove row
    if ((best_j >= ctrl.ws_start) && (best_j < ctrl.ws_end))
      ctrl.overlaps.erase(ctrl.overlaps.begin() + best_j);

    std::cout << "sz " << ctrl.ws_size << "start "<< ctrl.ws_start << "end " << ctrl.ws_end << std::endl;
    if (best_j  < ctrl.ws_start)
      ctrl.ws_start--;
    if (best_j  < ctrl.ws_end)
      ctrl.ws_end--;

    ctrl.ws_size = ctrl.ws_end - ctrl.ws_start;
    std::cout << "sz " << ctrl.ws_size << "start "<< ctrl.ws_start << "end " << ctrl.ws_end << std::endl;

    ctrl.strs.erase(ctrl.strs.begin() + best_j);
  }

  return ctrl.strs.front();
}
