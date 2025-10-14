#include "common.h"
#include "overlap.h"
#include "shsup.h"

void 
load_strings (std::istream& in, std::vector<std::string>& strs)
{
  std::string aux;
  while (getline (in, aux)) 
  {
    if (aux.empty())
      break;
    strs.push_back (aux);
  }

  return;
}

int 
main(int argc, char **argv) 
{
  std::vector<std::string> strs;
  std::ifstream file;
  std::istream* in = &std::cin;
  omp_set_num_threads(NTHREADS);

  if (argc >= 2)
  {
    file.open(argv[1]);
    if (!file)
      return 1;

    in = &file;
  }

  load_strings (*in, strs);
  auto overlaps = compute_overlap_matrix(strs);
  compute_shortest_superstring (strs, overlaps);

  return 0;
}
