# Research of slide 20

There is a possible optimisation on the parrallel pi approximation. In OMP it is possible to use reduction in for loops instead of using atmoic to prevent race conditions and making the sum thread safe. 

## What does reduction do and why is it better (faster) than atomic?

Reduction will give each thread its own private instance of the sum when forking. While computing the thread then increases its own sum, not the global one, which will not lead into a race condition wich will cause no false values. When merging again each private instance of the sum variable then will get merged into the global one using an explicit operator, in this example the addition operator. This will make the global sum thread safe and prevents false values such atomic or critical omp pragma. 

Why is it faster? This is fatser, because each thread has its own private variable and does not need to wait for another thread to finish its operation on the atomic variable. When using reduction, these waits are not there anymore, because each thread has its own private variable. 

## Example optimisation of the loop using reduction
```cpp
#pragma omp parallel // parallel region starts here using reduction of the global sum variable
  { // idea for parallelisation: split up loop iterations between threads
    int num_threads = omp_get_num_threads(); // used as increment in for loop
    int thread_id = omp_get_thread_num();
    double sum_local = 0.0; // for summing up heights locally
    for (int i = thread_id; i < num_steps; i += num_threads) {
      double x = (i + 0.5) * width; // midpoint
      sum_local = sum_local + (1.0 / (1.0 + x * x)); // add new height
    }
    sum += sum_local;
  } // parallel region ends here
```