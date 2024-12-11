#include "semaforo.h"

// Semaphore::Semaphore(int count_) : count(count_) {}

// void Semaphore::notify() {
//     std::unique_lock<std::mutex> lock(mtx);
//     count++;
//     cv.notify_one();
// }

// void Semaphore::wait() {
//     std::unique_lock<std::mutex> lock(mtx);
//     cv.wait(lock, [this]() { return count > 0; });
//     count--;
// }
