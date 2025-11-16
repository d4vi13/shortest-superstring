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
  int working_set_size;
  std::set<uint32_t> working_set;
  std::vector<std::string> strs;
  MPI_Status status;
};

extern double ptotal;
extern struct ctrl ctrl;

#endif
