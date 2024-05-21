template <typename T>
struct Generator
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        T value_;
        std::exception_ptr exception_;

        Generator get_return_object()
        {
            return Generator(handle_type::from_promise(*this));
        }

        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(T value) { value_ = value; }
        void unhandled_exception() { exception_ = std::current_exception(); }
    };

    handle_type h_;

    Generator(handle_type h)
        : h_(h)
    {
    }
    ~Generator() { h_.destroy(); }

    bool get()
    {
        return h_.promise().value_;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Generator)
};
