#ifndef DATAQUEUE_H
#define DATAQUEUE_H

#include <queue>
#include <cstddef>

#if defined(ESP_PLATFORM) || defined(FREERTOS_CONFIG_H) || defined(INC_FREERTOS_H)
#define DATAQUEUE_USE_FREERTOS 1
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#else
#include <mutex>
#define DATAQUEUE_USE_STD_MUTEX 1
#endif

/**
 * @brief Thread-safe generic queue template.
 *
 * Provides synchronized FIFO data transfer between concurrent tasks
 * (e.g., sensor acquisition task and telemetry/network transmission task).
 * Supports FreeRTOS Semaphore/Mutex on microcontrollers and std::mutex
 * for host/desktop unit tests.
 */
template <typename T>
class DataQueue {
private:
    std::queue<T> queue;
    size_t maxSize;

#if defined(DATAQUEUE_USE_FREERTOS)
    SemaphoreHandle_t mutex;
#else
    mutable std::mutex mtx;
#endif

public:
    explicit DataQueue(size_t max_size = 50) : maxSize(max_size) {
#if defined(DATAQUEUE_USE_FREERTOS)
        mutex = xSemaphoreCreateMutex();
#endif
    }

    ~DataQueue() {
#if defined(DATAQUEUE_USE_FREERTOS)
        if (mutex != nullptr) {
            vSemaphoreDelete(mutex);
        }
#endif
    }

    DataQueue(const DataQueue&) = delete;
    DataQueue& operator=(const DataQueue&) = delete;

    /**
     * @brief Thread-safely push an item to the back of the queue.
     * Drops oldest item if queue has reached maxSize.
     * @param item Item to insert.
     * @return true if inserted, false if mutex acquisition timed out.
     */
    bool push(const T& item) {
#if defined(DATAQUEUE_USE_FREERTOS)
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            if (queue.size() >= maxSize) {
                queue.pop();
            }
            queue.push(item);
            xSemaphoreGive(mutex);
            return true;
        }
        return false;
#else
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.size() >= maxSize) {
            queue.pop();
        }
        queue.push(item);
        return true;
#endif
    }

    /**
     * @brief Thread-safely pop an item from the front of the queue.
     * @param[out] item Destination for retrieved element.
     * @return true if element was retrieved, false if queue was empty or timeout.
     */
    bool pop(T& item) {
#if defined(DATAQUEUE_USE_FREERTOS)
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            if (queue.empty()) {
                xSemaphoreGive(mutex);
                return false;
            }
            item = queue.front();
            queue.pop();
            xSemaphoreGive(mutex);
            return true;
        }
        return false;
#else
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty()) {
            return false;
        }
        item = queue.front();
        queue.pop();
        return true;
#endif
    }

    /**
     * @brief Check whether the queue is currently empty.
     */
    bool isEmpty() const {
#if defined(DATAQUEUE_USE_FREERTOS)
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            bool empty = queue.empty();
            xSemaphoreGive(mutex);
            return empty;
        }
        return queue.empty();
#else
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
#endif
    }

    /**
     * @brief Return number of items currently in queue.
     */
    size_t size() const {
#if defined(DATAQUEUE_USE_FREERTOS)
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            size_t count = queue.size();
            xSemaphoreGive(mutex);
            return count;
        }
        return queue.size();
#else
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
#endif
    }
};

#endif // DATAQUEUE_H
