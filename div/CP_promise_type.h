template <typename T>
struct Generator
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        T value_;
        std::exception_ptr exception_;

        auto get_return_object()
        {
            return std::async([this] {
                if (exception_)
                    std::rethrow_exception(exception_);
                return value_;
            });
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Generator)
};

Generator<int> countTo(int n)
{
    for (int i = 0; i <= n; ++i)
    {
        co_return i;
    }
}

int main()
{
    auto future = countTo(10);
    std::cout << "Counted to " << future.get() << std::endl;
    return 0;
}
