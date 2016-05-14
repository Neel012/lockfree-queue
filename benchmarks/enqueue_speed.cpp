#include <assert.h>
#include <thread>
#include "../queue.h"
#include "../ms_queue.h"
#include "../mutex_queue.h"
#include <iostream>
#include <ctime>


void pure_enqueue_speed(lockfree::queue<int> &queue, int threads_count, int number_of_enqueues) {
  std::vector<std::thread> threads;
  bool wait = true;
  int number_of_enqueues_per_thread = number_of_enqueues / threads_count;
  std::cout << number_of_enqueues_per_thread << std::endl;

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
  std::cout << diff.count() << std::endl;


}

int main() {


  for (int i = 0; i < 8; ++i) {
    lockfree::ms_queue<int> queue1;
    pure_enqueue_speed(queue1, i + 1, 20000000);
  }

  for (int i = 0; i < 8; ++i) {
    lockfree::mutex_queue<int> queue2;
    pure_enqueue_speed(queue2, i + 1, 20000000);
  }


}