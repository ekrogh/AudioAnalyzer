#include <vector>
#include <coroutine>
#include <iostream>

template <typename T>
class VectorGenerator {
public:
    struct promise_type {
        std::vector<T> value;
        std::exception_ptr exception;

        VectorGenerator get_return_object() {
            return VectorGenerator{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(std::vector<T> v) {
            value = std::move(v);
            return {};
        }

        void return_void() {}
        void unhandled_exception() { exception = std::current_exception(); }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    VectorGenerator(handle_type h) : handle(h) {}
    ~VectorGenerator() { if (handle) handle.destroy(); }

    std::vector<T> operator()() {
        if (!handle.done()) {
            handle.resume();
        }
        if (handle.promise().exception) {
            std::rethrow_exception(handle.promise().exception);
        }
        return handle.promise().value;
    }

private:
    handle_type handle;
};

template <typename T>
VectorGenerator<T> processBuffer(std::vector<T> inputBuffer) {
    while (true) {
        std::vector<T> outputBuffer;
        outputBuffer.reserve(inputBuffer.size());

        for (const auto& sample : inputBuffer) {
            // Example processing: simply copy the input to the output
            outputBuffer.push_back(sample);
        }

        co_yield outputBuffer;
    }
}

int main() {
    std::vector<float> inputBuffer1 = { 1.0f, 2.0f, 3.0f, 4.0f };
    auto coroutine = processBuffer(inputBuffer1);
    auto generatorOut = coroutine();

    for (const auto& value : generatorOut) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    std::vector<float> inputBuffer2 = { 5.0f, 6.0f, 7.0f, 8.0f };
    coroutine = processBuffer(inputBuffer2);
    generatorOut = coroutine();

    for (const auto& value : generatorOut) {
        std::cout << value << " ";
    }

    return 0;
}

