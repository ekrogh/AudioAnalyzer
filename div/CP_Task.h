#include <future>

template<typename T>
struct Task {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T value_;
        std::promise<T> promise_;
        auto get_return_object() {
            return handle_type::from_promise(*this);
        }
        auto initial_suspend() {
            return std::suspend_always{};
        }
        auto final_suspend() noexcept {
            return std::suspend_always{};
        }
        void return_value(T value) {
            value_ = value;
            promise_.set_value(value_);
        }
        void unhandled_exception() {
            promise_.set_exception(std::current_exception());
        }
    };

    handle_type coro_;

    Task(handle_type h) : coro_(h) {}
    ~Task() {
        if (coro_) coro_.destroy();
    }

    auto get_future() {
        return coro_.promise().promise_.get_future();
    }
};

Task<bool> readerToFftDataCopy() {
    // ... your existing code here ...
    co_return false;
}


// File: FFTModuleTaskCoroutine.h

#include <coroutine>
#include <future>

template<typename T>
struct Task
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        T value_;
        std::promise<T> promise_;
        auto get_return_object()
        {
            return handle_type::from_promise(*this);
        }
        auto initial_suspend()
        {
            return std::suspend_always{};
        }
        auto final_suspend() noexcept
        {
            return std::suspend_always{};
        }
        void return_value(T value)
        {
            value_ = value;
            promise_.set_value(value_);
        }
        void unhandled_exception()
        {
            promise_.set_exception(std::current_exception());
        }
    };

    handle_type coro_;

    Task(handle_type h) : coro_(h) {}
    ~Task()
    {
        if (coro_) coro_.destroy();
    }

    auto get_future()
    {
        return coro_.promise().promise_.get_future();
    }
};

Task<bool> readerToFftDataCopy()
{
    // ... your existing code here ...

    // At the end of your function, use co_return to return the result.
    // Replace "result" with the actual result of your computation.
    bool result = /* your computation */;
    co_return result;
}

Task<bool> task = readerToFftDataCopy();
std::future<bool> future = task.get_future();
bool result = future.get();
