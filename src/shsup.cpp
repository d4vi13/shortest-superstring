#include "shsup.h"
#include <limits>
#include <cstdint>

void
find_max_overlap (int32_t *pbest_i,
                  int32_t *pbest_j,
                  int32_t *pbest_ov)
{
  int32_t best_i = -1, best_j = -1, best_ov = -1;
  int32_t n = ctrl.strs.size();

  if (ctrl.ws_size == 0) {
    return;
  }

  #pragma omp parallel
  {
    int32_t local_i = -1;
    int32_t local_j = -1;
    int32_t local_ov = -1;

    // find local overlap, but uses global indexes
    #pragma omp for nowait
    for (int32_t row = 0; row < ctrl.ws_size; ++row) {

      int32_t row_abs = ctrl.ws_start + row;

      for (int32_t col = 0; col < n; ++col) {
        if (row_abs == col) continue;

        int32_t ov = ctrl.overlaps[row][col];
        if (ov > local_ov)
        {
          local_ov = ov;
          local_i = row_abs;
          local_j = col;
        }
      }
    } 

    #pragma omp critical
    {
      if (local_ov > best_ov 
          || (local_ov == best_ov && local_i > best_i) 
          || (local_ov == best_ov && local_i == best_i && local_j > best_j))
      {
        best_ov = local_ov;
        best_i = local_i;
        best_j = local_j;
      }
    }
  } 

  *pbest_i = best_i;
  *pbest_j = best_j;
  *pbest_ov = best_ov;
}

void
find_max_overlap_across_nodes (int32_t *best_i,
                               int32_t *best_j,
                               int32_t *best_ov)
{
  int32_t local_i, local_j, local_ov = *best_ov;

  MPI_Allreduce(&local_ov, best_ov, 1, MPI_INT32_T, MPI_MAX, MPI_COMM_WORLD);

  local_i = (local_ov == *best_ov ? *best_i : -1);
  MPI_Allreduce(&local_i, best_i, 1, MPI_INT32_T, MPI_MAX, MPI_COMM_WORLD);

  local_j = (local_i == *best_i ? *best_j : -1);
  MPI_Allreduce(&local_j, best_j, 1, MPI_INT32_T, MPI_MAX, MPI_COMM_WORLD);
}

// Main loop to compute shortest common superstring (keeps your structure but fixes index handling).
std::string
compute_shortest_superstring()
{
  double start, end;
  while (ctrl.strs.size() > 1) {
    int32_t n = ctrl.strs.size();
    int32_t best_i = -1, best_j = -1, best_ov = -1;

    // find best overlap within this node (returns absolute indices)
    find_max_overlap(&best_i, &best_j, &best_ov);

    // keep a local copy to know if this rank won after consensus
    int32_t backup = best_i;

    start = omp_get_wtime();
    // consensus across MPI ranks 
    find_max_overlap_across_nodes(&best_i, &best_j, &best_ov);
    end = omp_get_wtime();
    stotal += end - start;

    // determine whether this rank is responsible for recomputing the row
    bool won = (backup == best_i);
    
    ctrl.strs[best_i] = ctrl.strs[best_i] + ctrl.strs[best_j].substr(best_ov);

    // recompute the row corresponding to best_i if this node owns that row
    if (won) {
      int32_t row_idx = best_i - ctrl.ws_start;
      if (row_idx >= 0 && row_idx < ctrl.ws_size) {
        #pragma omp parallel for schedule(dynamic)
        for (int32_t k = 0; k < n; ++k) {
          if (k == best_i) continue;
          ctrl.overlaps[row_idx][k] = calculate_overlap(ctrl.strs[best_i], ctrl.strs[k]);
        }
      }
    }

    // recompute the column corresponding to best_i for all rows in working set
    #pragma omp parallel for schedule(dynamic)
    for (int32_t r = 0; r < ctrl.ws_size; ++r) {
      ctrl.overlaps[r][best_i] = calculate_overlap(ctrl.strs[ctrl.ws_start + r], ctrl.strs[best_i]);
    }

    // remove column best_j from every overlap row
    #pragma omp parallel for schedule(static)
    for (int32_t r = 0; r < ctrl.ws_size; ++r) {
        ctrl.overlaps[r].erase(ctrl.overlaps[r].begin() + best_j);
    }

    // if best_j is inside the working set rows, remove that row 
    if (best_j >= ctrl.ws_start && best_j < ctrl.ws_end) {
        ctrl.overlaps.erase(ctrl.overlaps.begin() +  best_j - ctrl.ws_start);
    }

    // fix working set indices after removing an element before ws_start or inside ws range
    if (best_j < ctrl.ws_start) ctrl.ws_start--;
    if (best_j < ctrl.ws_end)   ctrl.ws_end--;

    ctrl.ws_size = ctrl.ws_end - ctrl.ws_start;

    // remove the merged string (best_j) from strings vector
      ctrl.strs.erase(ctrl.strs.begin() + best_j);
  } 

  return ctrl.strs.front();
}
