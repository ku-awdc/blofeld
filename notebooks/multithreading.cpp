/*
 * NOTE: we can't safely call R:: (or Rcpp:: ??) from threads, as R is single threaded
 * But we can pass in a wrapper class/struct with wrappers around e.g. R::rpois which are used with mutexes
 * This means we can't return control to R, but we can put the main thread in a loop until all threads are done, with periodic checks to checkUserInterrupt with the same mutex
 * (Or we can just join the threads, without checkUserInterrupt, or get the threads to call checkUserInterrupt through the mutex)
 * If the threads have no R calls (pure C++ code) we can detach them and return control to R safely, I think
 */

class MultiThread
{
  std::vector<std::jthread> m_thread_pool;

};
