#include <assert.h>
#include <chrono>
#include <thread>
#include <ms_queue.hpp>
#include <my_queue.hpp>
#include <epoch_queue.hpp>
#include <mutex_queue.hpp>
#include <iostream>
#include <condition_variable>

struct Barrier {
  void wait() {
    std::unique_lock< std::mutex > lk( _m );
    if (_wait) {
      _cv.wait(lk, [this] { return !_wait; } );
    }

  }

  void start() {
    std::unique_lock < std::mutex > lk( _m );
    _wait = false;
    _cv.notify_all();
  }

private:
  bool _wait = true;
  std::condition_variable _cv;
  std::mutex _m;
};

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
    Barrier barrier;
    int size_per_thread = size / threads_count;

    for (int i = 0; i < threads_count; ++i) {
      prepare_fn(queue, size_per_thread);
    }

    for (int i = 0; i < threads_count; ++i) {
      threads.push_back(std::thread([&] {
        barrier.wait();
        benchmark_fn(queue, size_per_thread);
      }));
    }

    auto start = std::chrono::system_clock::now();
    barrier.start();

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
      std::this_thread::sleep_for(std::chrono::microseconds(10000));
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

void help(char name[]) {
  std::cout << "Usage: " << name << " <queue type | --help>\n";
  std::cout << "queue types: mutex_queue, my_queue, ms_queue, epoch_queue\n";
  std::cout << "--help: show this help\n\n";
}

int main(int argc, char* argv[]) {
  int size = 10000000;
  int number_of_runs = 2;
  if (argc != 2) {
    help(argv[0]);
    return -1;
  }
  std::string queue_type{argv[1]};
  if (queue_type == "mutex_queue") {
    for (int threads = 1; threads <= 8; ++threads) EnqueueBenchmark<lockfree::mutex_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) DequeueBenchmark<lockfree::mutex_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) EndequeueHighBenchmark<lockfree::mutex_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) EndequeueLowBenchmark<lockfree::mutex_queue<int>>(size, threads, number_of_runs, true);
  } else if (queue_type == "my_queue") {
    for (int threads = 1; threads <= 8; ++threads) EnqueueBenchmark<lockfree::my_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) DequeueBenchmark<lockfree::my_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) EndequeueHighBenchmark<lockfree::my_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) EndequeueLowBenchmark<lockfree::my_queue<int>>(size, threads, number_of_runs, true);
  } else if (queue_type == "ms_queue") {
    for (int threads = 1; threads <= 8; ++threads) EnqueueBenchmark<lockfree::ms_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) DequeueBenchmark<lockfree::ms_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) EndequeueHighBenchmark<lockfree::ms_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) EndequeueLowBenchmark<lockfree::ms_queue<int>>(size, threads, number_of_runs, true);
  } else if (queue_type == "epoch_queue") {
    for (int threads = 1; threads <= 8; ++threads) EnqueueBenchmark<lockfree::epoch_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) DequeueBenchmark<lockfree::epoch_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) EndequeueHighBenchmark<lockfree::epoch_queue<int>>(size, threads, number_of_runs, true);
    for (int threads = 1; threads <= 8; ++threads) EndequeueLowBenchmark<lockfree::epoch_queue<int>>(size, threads, number_of_runs, true);
  } else {
    help(argv[0]);
    return -1;
  }
  return 0;
}
