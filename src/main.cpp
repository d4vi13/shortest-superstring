#include "common.h"
#include "overlap.h"
#include "shsup.h"

double ptotal = 0;
uint32_t size;

void 
load_strings (std::istream& in, std::vector<std::string>& strs)
{
  uint32_t i;
  std::string aux;
  getline (in, aux);

  size = std::stoi(aux);
  i = size;
  while (i--) 
  {
    getline (in, aux);
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
  double start, end, total;
  std::string res;

  if (argc >= 2)
  {
    file.open(argv[1]);
    if (!file)
      return 1;

    in = &file;
  }

  if (argc >= 3) 
    omp_set_num_threads(std::atoi(argv[2]));
  else
    omp_set_num_threads(NTHREADS);
  
  load_strings (*in, strs);
  
  start = omp_get_wtime();
  auto overlaps = compute_overlap_matrix(strs);
  res = compute_shortest_superstring (strs, overlaps);
  end = omp_get_wtime();

  total = end - start;

  std::cout << "Resposta: " << res << std::endl;
  std::cout << "Tamnho: " << res.size() << std::endl;
  std::cout << size << "," << total << "," << total - ptotal << "," << ptotal << std::endl;

  return 0;
}
