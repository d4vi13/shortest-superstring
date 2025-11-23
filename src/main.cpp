#include "common.h"
#include "overlap.h"
#include "shsup.h"
#define GET_THREAD_NUM_SCRIPT "./get_num_of_threads.pl hosts.txt"

double ptotal = 0;
struct ctrl ctrl;
uint32_t size;

void
load_strings (std::istream &in, std::vector<std::string> &strs)
{
  uint32_t i;
  std::string aux;
  getline (in, aux);

  size = std::stoi (aux);
  i = size;
  while (i--)
    {
      getline (in, aux);
      if (aux.empty ())
        break;
      strs.push_back(aux);
    }

  return;
}

int
get_num_of_threads ()
{
  FILE *script;
  char thread_num_str[16];

  if (!(script = popen (GET_THREAD_NUM_SCRIPT, "r")))
    {
      std::cerr << "failed to get number of threas\n";
      return 0;
    }

  if (!fgets (thread_num_str, 16, script))
    {
      std::cerr << "failed to read scritp result\n";
      return 0;
    }

  pclose (script);

  return atoi (thread_num_str);
}

int
get_working_set ()
{

  int start, end;
  int threads[ctrl.cluster_size];
  int working_set_min_size;
  int leftover;
  int total = ctrl.nproc;

  switch (ctrl.rank)
    {
    case 0:
      threads[0] = ctrl.nproc;

      for (int i = 1; i < ctrl.cluster_size; i++)
        {
          MPI_Recv (&threads[i], 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD,
                    &ctrl.status);
          total += threads[i];
        }

      working_set_min_size = ctrl.strs.size () / total;
      leftover = ctrl.strs.size () % total;

      ctrl.ws_start = 0;
      ctrl.ws_end = working_set_min_size * threads[0] + leftover;

      if (!working_set_min_size)
        {
          int value = 0;
          for (int r = 1; r < ctrl.cluster_size; r++) {
            MPI_Send(&value, 1, MPI_INT, r, 0, MPI_COMM_WORLD);
            MPI_Send(&value, 1, MPI_INT, r, 0, MPI_COMM_WORLD);
          }
          goto calc_ws_size;
        }

      start = working_set_min_size * threads[0] + leftover;
      end = start;
      for (int i = 1; i < ctrl.cluster_size; i++)
        {
          if ((end + working_set_min_size * threads[i])
              > (int)(ctrl.strs.size ()))
            end = ctrl.strs.size();
          else
            end += working_set_min_size * threads[i];
          MPI_Send (&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
          MPI_Send (&end, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
          start = end;
        }

      break;
    default:
      MPI_Send (&ctrl.nproc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

      MPI_Recv (&ctrl.ws_start, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                &ctrl.status);
      MPI_Recv (&ctrl.ws_end, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                &ctrl.status);

      break;
    }

calc_ws_size:

      ctrl.ws_size = ctrl.ws_end - ctrl.ws_start;
  return 1;
}

int
main (int argc, char **argv)
{
  std::ifstream file;
  std::istream *in = &std::cin;
  double start, end, total;
  std::string res;

  MPI_Init (&argc, &argv);

  MPI_Comm_rank (MPI_COMM_WORLD, &ctrl.rank);
  MPI_Comm_size (MPI_COMM_WORLD, &ctrl.cluster_size);
  if (!(ctrl.nproc = get_num_of_threads ()))
    {
      return 0;
    }

  if (argc >= 2)
    {
      file.open (argv[1]);
      if (!file)
        return 1;

      in = &file;
    }

  load_strings (*in, ctrl.strs);

  if (!get_working_set ())
    {
      std::cerr << "Failed to get working set\n";
      return 0;
    }

  start = omp_get_wtime ();
  ctrl.overlaps = compute_overlap_matrix (ctrl.strs);

  res = compute_shortest_superstring ();
 
  end = omp_get_wtime ();

  total = end - start;


  std::cout << "Resposta: " << res << std::endl;
  std::cout << "Tamanho: " << res.size () << std::endl;
  std::cout << size << "," << total << "," << total - ptotal << "," << ptotal
            << std::endl;

finish:
  MPI_Finalize ();
  return 0;
}
