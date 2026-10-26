#include <iostream>
#include <omp.h>
#include <random>

using namespace std;

int main() {
  //set the max thread available of teh system as teh thread nums used by omp so every thread gets used.
  int max_threads = omp_get_max_threads();
  omp_set_num_threads(max_threads);

  unsigned int seed = 0;
  default_random_engine re{seed};
  uniform_real_distribution<double> zero_to_one{0.0, 1.0};

  int n = 100000000; // number of points to generate
  int counter = 0; // counter for points lying in the first quadrant of a unit circle
  auto start_time = omp_get_wtime(); // omp_get_wtime() is an OpenMP library routine

    // use reduction so each  thread has its own private counter when forking. when mergin, each private counter of the thread gets saflz added to the global counter. 
    // compute n points and test if they lie within the first quadrant of a unit circle
#pragma omp parallel for reduction(+:counter)
    for (int i = 0; i < n; ++i) {
      auto x = zero_to_one(re); // generate random number between 0.0 and 1.0
      auto y = zero_to_one(re); // generate random number between 0.0 and 1.0
      if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
        ++counter;
      }
    }
  auto run_time = omp_get_wtime() - start_time;
  auto pi = 4 * (double(counter) / n);

  cout << "pi: " << pi << endl;
  cout << "run_time: " << run_time << " s" << endl;
  cout << "n: " << n << endl; 
}
