#include "shsup.h"

void
find_max_overlap (int32_t *pbest_i,
    int32_t *pbest_j, int32_t *pbest_ov)
{
  int32_t best_i = -1, best_j = -1, best_ov = -1;
  int32_t n = ctrl.strs.size ();

  if (ctrl.ws_size == 0) {
    return;
  }

#pragma omp parallel 
  { 
  int32_t local_i = 0, local_j = 1, local_ov = 0;
#pragma omp for nowait
  for (auto i = 0; i < ctrl.ws_size; i++)
  {
    for (int32_t j = 0; j < n; j++)
    {
      if ((ctrl.ws_start + i) == j)
        continue;
      if (ctrl.overlaps[i][j] >= local_ov)
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
      if (local_i > best_i)
      {
        best_i = local_i;
        best_j = local_j;
        best_ov = local_ov;
      }
      else if (local_i == best_i)
      {
        if (local_j > best_j)
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
find_max_overlap_across_nodes (int32_t *best_i,
                  int32_t *best_j, int32_t *best_ov)
{
  int32_t local_i = *best_i, local_j = *best_j, local_ov = *best_ov;

  if (ctrl.ws_size)
    local_i += ctrl.ws_start;

  MPI_Allreduce(&local_ov, best_ov, 1, MPI_INT32_T, MPI_MAX, MPI_COMM_WORLD);

  if (*best_ov != local_ov) {
    local_i = -1;
  }

  MPI_Allreduce(&local_i, best_i, 1, MPI_INT32_T, MPI_MAX, MPI_COMM_WORLD);

  if (*best_i != local_i) {
    local_j = -1;
  }

  MPI_Allreduce(&local_j, best_j, 1, MPI_INT32_T, MPI_MAX, MPI_COMM_WORLD);
}

std::string
compute_shortest_superstring() {
  int32_t best_i, best_j, best_ov, start, end, n = ctrl.strs.size();
  bool won;

  while (ctrl.strs.size() > 1) {
    n = ctrl.strs.size();
    best_i = -1, best_j = -1,best_ov = -1;
    won = false;

    // PRINT MATRIX
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


    std::cout << "finding consensus\n";
    int32_t backup = best_i;
    find_max_overlap_across_nodes (&best_i, &best_j, &best_ov);
      std:: cout << "\nbest_i " << best_i << " best_j " << best_j << " overlap" << best_ov << std::endl;

    //std::cout << "computing new string\n";
    // check if this thread won and set won to true
    won = (backup + ctrl.ws_start) == best_i;

    std::cout << "best i string " <<  ctrl.strs[best_i] << std::endl;
    std::cout << "best j string " <<  ctrl.strs[best_j] << std::endl;
    std::cout<< "overlap " << best_ov << std::endl;
    ctrl.strs[best_i] = ctrl.strs[best_i] + ctrl.strs[best_j].substr(best_ov);
    std::cout << "new string " <<  ctrl.strs[best_i] << std::endl;

    //std::cout << "recomputing " << ctrl.ws_size << std::endl;
    // recompute the line if it belongs to the working set
    if (won && ctrl.ws_size) {
      std::cout << "ganhou\n";
    #pragma omp parallel for schedule(dynamic)
      for (uint32_t k = 0; k < n; k++) {
        if (k == best_i) continue;
        ctrl.overlaps[backup][k] = calculate_overlap(ctrl.strs[best_i], ctrl.strs[k]);
      }
    }
    
    //std::cout << "recomputing2 " << ctrl.ws_size << std::endl;
    // recompute the column of best_i
    #pragma omp parallel for schedule(dynamic)
    for (uint32_t k = 0; k < ctrl.ws_size; k++) {
      ctrl.overlaps[k][best_i] = calculate_overlap(ctrl.strs[ctrl.ws_start + k], ctrl.strs[best_i]);
    }

    //std::cout << "deleting column\n";
    //remove best_j column
    #pragma omp parallel for schedule(static)
    for (uint32_t k = 0; k  < ctrl.overlaps.size(); k++) {
      ctrl.overlaps[k].erase(ctrl.overlaps[k].begin() + best_j);
    }

    //std::cout << "deleting row\n";
    // if best_j belongs to the working set remove row
    if ((best_j >= ctrl.ws_start) && (best_j < ctrl.ws_end))
      ctrl.overlaps.erase(ctrl.overlaps.begin() + (best_j - ctrl.ws_start));

    //std::cout << "fixing ws\n";
    // if best_j belongs to the working set remove row
    //std::cout << "sz " << ctrl.ws_size << "start "<< ctrl.ws_start << "end " << ctrl.ws_end << std::endl;
    if (best_j  < ctrl.ws_start)
      ctrl.ws_start--;
    if (best_j  < ctrl.ws_end)
      ctrl.ws_end--;

    ctrl.ws_size = ctrl.ws_end - ctrl.ws_start;
    //std::cout << "sz " << ctrl.ws_size << "start "<< ctrl.ws_start << "end " << ctrl.ws_end << std::endl;

    std::cout << "string remaining "<< ctrl.strs.size() << std::endl;
    if (ctrl.rank == 0)
    {
      int h = 0;
      for (auto s = ctrl.strs.begin(); s != ctrl.strs.end(); s++) {
        std::cout <<h << " " << *s << std::endl;
        h++;
      }
      std::cout << "removing " << ctrl.strs[best_j] << std::endl;
    }
    
    ctrl.strs.erase(ctrl.strs.begin() + best_j);
  }

  return ctrl.strs.front();
}
