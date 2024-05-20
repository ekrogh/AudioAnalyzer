
#include <coroutine>
#include <iostream>

struct task
{
	struct promise_type
	{
		bool current_value;

		auto get_return_object()
		{
			return task{ std::coroutine_handle<promise_type>::from_promise(*this) };
		}
		auto initial_suspend()
		{
			return std::suspend_always{};
		}
		auto final_suspend() noexcept
		{
			return std::suspend_always{};
		}
		void unhandled_exception()
		{
			std::terminate();
		}
		auto yield_value(bool value)
		{
			current_value = value;
			return std::suspend_always{};
		}
		void return_value(bool value)
		{
			current_value = value;
			return; /*std::suspend_always{};*/
		}
	};

	std::coroutine_handle<promise_type> coro;

	explicit task(std::coroutine_handle<promise_type> h) : coro(h) {}

	void resume()
	{
		coro.resume();
	}

	bool done() const
	{
		return coro.done();
	}

	bool get_result() const
	{
		return coro.promise().current_value;
	}

	~task()
	{
		if (coro)
		{
			coro.destroy();
		}
	}
};

task generator()
{
	for (size_t i = 0; i < 10; i++)
	{
		co_yield true;
	}
	
	co_yield false;
	//co_return false;
}

int main()
{
	task t = generator();

	for (size_t i = 0; i < 2; i++)
	{
		while (!t.done())
		{
			t.resume();
			std::cout << "Value: " << t.get_result() << std::endl;
		}
	}

	return 0;
}
