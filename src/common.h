#ifndef COMMON_H
#define COMMON_H

// IO

#include <fstream>
#include <iostream>

// types

#include <algorithm>
#include <cstdint>

// Data structures

#include <map>
#include <unordered_map>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <vector>

// Parallelization

#include <mpi.h>
#include <omp.h>

struct ctrl
{
  int rank;
  int nproc;
  int cluster_size;

  int ws_start;
  int ws_end;
  int ws_size;
  std::vector<std::string> strs;

  std::set<uint32_t> working_set;
  std::vector<std::vector<uint32_t>> overlaps;
  MPI_Status status;
};

extern double ptotal;
extern struct ctrl ctrl;

#endif
