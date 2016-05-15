#include <assert.h>
#include <thread>
#include "../queue.h"
#include "../ms_queue.h"
#include "../mutex_queue.h"
#include <iostream>
#include <ctime>
#include <unistd.h>


using benchmark_t = std::function<double(lockfree::queue<int> &, int, int)>;

double pure_deenqueue_speed(lockfree::queue<int> &queue, int threads_count, int number_of_enqueues) {
  std::vector<std::thread> threads;
  bool wait = true;
  int number_of_enqueues_per_thread = number_of_enqueues / threads_count;

  auto f = [&] {
    while (wait) { };
    for (int o = 0; o < number_of_enqueues_per_thread; ++o) {
      queue.enqueue(0);
      queue.dequeue();
    }
  };

  for (int i = 0; i < threads_count; ++i) {
    threads.push_back(std::thread(f));
  }

  auto start = std::chrono::system_clock::now();

  wait = false;
  for (auto &thread : threads) {
    thread.join();
  }

  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> diff = end - start;
  return diff.count();
}


double pure_dequeue_speed(lockfree::queue<int> &queue, int threads_count, int number_of_enqueues) {
  std::vector<std::thread> threads;
  bool wait = true;
  int number_of_enqueues_per_thread = number_of_enqueues / threads_count;

  for (int o = 0; o < number_of_enqueues; ++o) queue.enqueue(0);

  auto f = [&] {
    while (wait) { };
    for (int o = 0; o < number_of_enqueues_per_thread; ++o) {
      queue.enqueue(0);
      queue.dequeue();
    }

  };

  for (int i = 0; i < threads_count; ++i) {
    threads.push_back(std::thread(f));
  }

  auto start = std::chrono::system_clock::now();

  wait = false;
  for (auto &thread : threads) {
    thread.join();
  }

  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> diff = end - start;
  return diff.count();
}


double pure_enqueue_speed(lockfree::queue<int> &queue, int threads_count, int number_of_enqueues) {
  std::vector<std::thread> threads;
  bool wait = true;
  int number_of_enqueues_per_thread = number_of_enqueues / threads_count;

  auto f = [&] {
    while (wait) { };
    for (int o = 0; o < number_of_enqueues_per_thread; ++o) queue.enqueue(0);
  };

  for (int i = 0; i < threads_count; ++i) {
    threads.push_back(std::thread(f));
  }

  auto start = std::chrono::system_clock::now();

  wait = false;
  for (auto &thread : threads) {
    thread.join();
  }

  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> diff = end - start;
  return diff.count();
}


template<typename Queue>
double run(int threads_num, benchmark_t fn, int times = 1) {
  Queue queue;
  double sum = 0;
  for (int i = 0; i < times; ++i) {
    sum += fn(queue, threads_num, 1000000);
    usleep(100000);
  }
  return sum / times;
};

template<typename Queue>
void run_queue(std::vector<std::pair<std::string, benchmark_t>> &benchmarks) {
  for (auto fn : benchmarks) {
    std::cout << "benchmark: " << fn.first << std::endl;
    for (int i = 0; i < 4; ++i) {
      int ts = (1 << i);
      std::cout << "threads: " << ts << std::endl;
      std::cout << "time: " << run<Queue>(ts, fn.second, 10) << std::endl;
    }
    std::cout << std::endl;
  }
};

int main() {
  std::vector<std::pair<std::string, benchmark_t>> benchmarks;
  benchmarks.push_back(std::make_pair("pure_enqueue_speed", pure_enqueue_speed));
  benchmarks.push_back(std::make_pair("pure_dequeue_speed", pure_dequeue_speed));
  benchmarks.push_back(std::make_pair("pure_deenqueue_speed", pure_deenqueue_speed));

  std::cout << "mutex_queue" << std::endl;
  run_queue<lockfree::mutex_queue<int>>(benchmarks);
  std::cout << "ms_queue" << std::endl;
  run_queue<lockfree::ms_queue<int>>(benchmarks);
}
