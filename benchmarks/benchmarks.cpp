#include <assert.h>
#include <thread>
#include "../ms_queue.hpp"
#include "../my_queue.hpp"
#include "../epoch_queue.hpp"
#include "../mutex_queue.hpp"
#include <iostream>
#include <ctime>
#include <unistd.h>


template<typename Queue>
struct BaseBenchmark {

  BaseBenchmark(int size, int threads_count, int num_of_runs) :
      size(size),
      threads_count(threads_count),
      num_of_runs(num_of_runs) { }

  virtual void prepare_fn(Queue &, int) = 0;

  virtual void benchmark_fn(Queue &, int) = 0;

  double one_run() {
    Queue queue;
    std::vector<std::thread> threads;
    volatile bool wait = true;
    int size_per_thread = size / threads_count;

    for (int i = 0; i < threads_count; ++i)
      prepare_fn(queue, size_per_thread);


    auto bnchm = [&] {
      while (wait) { };
      benchmark_fn(queue, size_per_thread);
    };

    for (int i = 0; i < threads_count; ++i)
      threads.push_back(std::thread(bnchm));


    usleep(10000);
    auto start = std::chrono::system_clock::now();
    wait = false;

    for (auto &thread : threads)
      thread.join();

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    return diff.count();
  }

  double run() {
    double sum = 0;
    for (int i = 0; i < num_of_runs; ++i) {
      sum += one_run();
      usleep(10000);
    }
    std::cout <<
    "name: " << typeid(*this).name() <<
    "\tthreads: " << threads_count <<
    "\ttime: " << sum / num_of_runs << std::endl;
    return sum / num_of_runs;
  }

  int size;
  int threads_count;
  int num_of_runs;

};

template<typename Queue>
struct DequeueBenchmark : public BaseBenchmark<Queue> {

  DequeueBenchmark(int size, int threads_count, int num_of_runs, bool autorun = false) :
      BaseBenchmark<Queue>(size, threads_count, num_of_runs) {
    if (autorun) this->run();
  }

  virtual void prepare_fn(Queue &queue, int size) {
    for (int i = 0; i < size; ++i) queue.enqueue(0);
  }

  virtual void benchmark_fn(Queue &queue, int size) {
    for (int i = 0; i < size; ++i) queue.dequeue();
  }

};

template<typename Queue>
struct EnqueueBenchmark : public BaseBenchmark<Queue> {

  EnqueueBenchmark(int size, int threads_count, int num_of_runs, bool autorun = false) :
      BaseBenchmark<Queue>(size, threads_count, num_of_runs) {
    if (autorun) this->run();
  }

  virtual void prepare_fn(Queue &, int) { }

  virtual void benchmark_fn(Queue &queue, int size) {
    for (int i = 0; i < size; ++i) queue.enqueue(0);
  }

};

template<typename Queue>
struct EndequeueLowBenchmark : public BaseBenchmark<Queue> {

  EndequeueLowBenchmark(int size, int threads_count, int num_of_runs, bool autorun = false) :
      BaseBenchmark<Queue>(size, threads_count, num_of_runs) {
    if (autorun) this->run();
  }

  virtual void prepare_fn(Queue &, int) { }

  virtual void benchmark_fn(Queue &queue, int size) {
    for (int i = 0; i < size; ++i) {
      queue.enqueue(0);
      queue.dequeue();
    }
  }

};

template<typename Queue>
struct EndequeueHighBenchmark : public BaseBenchmark<Queue> {

  EndequeueHighBenchmark(int size, int threads_count, int num_of_runs, bool autorun = false) :
      BaseBenchmark<Queue>(size, threads_count, num_of_runs) {
    if (autorun) this->run();
  }

  virtual void prepare_fn(Queue &queue, int size) {
    for (int i = 0; i < size; ++i) queue.enqueue(0);
  }

  virtual void benchmark_fn(Queue &queue, int size) {
    for (int i = 0; i < size; ++i) {
      queue.enqueue(0);
      queue.dequeue();
    }
  }

};


// 1 - mutex_queue
// 2 - my_queue
// 3 - ms_queue 
// 4 - epoch_queue
int main(int argc, char* argv[]) {
  int size = 10000000;
  int number_of_runs = 10;
  switch (atoi(argv[1])) {
    case 1:
      for (int threads = 1; threads <= 8; ++threads) EnqueueBenchmark<lockfree::mutex_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) DequeueBenchmark<lockfree::mutex_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) EndequeueHighBenchmark<lockfree::mutex_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) EndequeueLowBenchmark<lockfree::mutex_queue<int>>(size, threads, number_of_runs, true);
      break;
    case 2:
      for (int threads = 1; threads <= 8; ++threads) EnqueueBenchmark<lockfree::my_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) DequeueBenchmark<lockfree::my_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) EndequeueHighBenchmark<lockfree::my_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) EndequeueLowBenchmark<lockfree::my_queue<int>>(size, threads, number_of_runs, true);
      break;
    case 3:
      for (int threads = 1; threads <= 8; ++threads) EnqueueBenchmark<lockfree::ms_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) DequeueBenchmark<lockfree::ms_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) EndequeueHighBenchmark<lockfree::ms_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) EndequeueLowBenchmark<lockfree::ms_queue<int>>(size, threads, number_of_runs, true);
      break;
    case 4:
      for (int threads = 1; threads <= 8; ++threads) EnqueueBenchmark<lockfree::epoch_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) DequeueBenchmark<lockfree::epoch_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) EndequeueHighBenchmark<lockfree::epoch_queue<int>>(size, threads, number_of_runs, true);
      for (int threads = 1; threads <= 8; ++threads) EndequeueLowBenchmark<lockfree::epoch_queue<int>>(size, threads, number_of_runs, true);
      break;
  }
//  EndequeueLowBenchmark<lockfree::my_queue<int>>(10000000, 2, 5, true);
}
