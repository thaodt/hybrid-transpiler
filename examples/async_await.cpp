/**
 * @file async_await.cpp
 * @brief C++20 Coroutine Examples - Demonstrates async/await patterns
 */

#include <iostream>
#include <future>
#include <coroutine>
#include <chrono>
#include <thread>

// Example 1: Simple coroutine with co_return
struct Task {
    struct promise_type {
        Task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

Task simpleAsyncFunction() {
    std::cout << "Starting async operation..." << std::endl;

    // Simulate async work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Async operation completed!" << std::endl;
    co_return;
}

// Example 2: Coroutine with co_await
template<typename T>
struct Future {
    struct promise_type {
        T value;

        Future get_return_object() {
            return Future{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_value(T v) {
            value = v;
        }

        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> handle;

    T get() {
        return handle.promise().value;
    }
};

Future<int> computeAsync(int x, int y) {
    // Simulate computation
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    int result = x + y;

    co_return result;
}

Future<int> chainedAsync(int value) {
    std::cout << "Starting chained async..." << std::endl;

    // Await another async operation
    auto result1 = co_await computeAsync(value, 10);
    auto result2 = co_await computeAsync(result1, 20);

    co_return result2;
}

// Example 3: Generator with co_yield
template<typename T>
struct Generator {
    struct promise_type {
        T current_value;

        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }

        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> handle;

    bool move_next() {
        handle.resume();
        return !handle.done();
    }

    T current() {
        return handle.promise().current_value;
    }
};

Generator<int> rangeGenerator(int start, int end) {
    for (int i = start; i < end; ++i) {
        co_yield i;
    }
}

// Example 4: std::async usage
class AsyncCalculator {
public:
    int performCalculation(int a, int b) {
        // Launch async computation
        auto future1 = std::async(std::launch::async, [](int x) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return x * 2;
        }, a);

        auto future2 = std::async(std::launch::async, [](int x) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return x * 3;
        }, b);

        // Wait for results
        int result1 = future1.get();
        int result2 = future2.get();

        return result1 + result2;
    }

    void fireAndForget(int value) {
        // Detached async operation
        std::async(std::launch::async, [](int x) {
            std::cout << "Processing: " << x << std::endl;
        }, value);
    }
};

// Example 5: Promise and Future
class DataFetcher {
private:
    std::promise<std::string> dataPromise;
    std::future<std::string> dataFuture;

public:
    DataFetcher() : dataFuture(dataPromise.get_future()) {}

    void fetchDataAsync() {
        std::thread([this]() {
            // Simulate network request
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            // Fulfill promise
            dataPromise.set_value("Fetched data!");
        }).detach();
    }

    std::string getData() {
        // Wait for data
        return dataFuture.get();
    }
};

int main() {
    // Example 1: Simple coroutine
    simpleAsyncFunction();

    // Example 2: Chained async
    auto chainResult = chainedAsync(5);
    std::cout << "Chained result: " << chainResult.get() << std::endl;

    // Example 3: Generator
    auto gen = rangeGenerator(0, 5);
    while (gen.move_next()) {
        std::cout << "Generated: " << gen.current() << std::endl;
    }

    // Example 4: std::async
    AsyncCalculator calc;
    int result = calc.performCalculation(10, 20);
    std::cout << "Calculation result: " << result << std::endl;

    calc.fireAndForget(42);

    // Example 5: Promise/Future
    DataFetcher fetcher;
    fetcher.fetchDataAsync();
    std::string data = fetcher.getData();
    std::cout << "Fetched: " << data << std::endl;

    return 0;
}
