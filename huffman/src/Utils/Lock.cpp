#include "STDInclude.hpp"

namespace Utils
{
	Lock::~Lock()
	{
		this->notifyAll();
		std::this_thread::yield();
	}

	void Lock::wait()
	{
		std::unique_lock<std::mutex> lock(this->mutex);
		this->notified = false;

		while (!this->notified)
		{
			this->conditionVar.wait(lock);
		}
	}

	void Lock::waitFor(std::chrono::nanoseconds nsecs)
	{
		std::unique_lock<std::mutex> lock(this->mutex);
		this->notified = false;

		auto start = std::chrono::high_resolution_clock::now();
		while (!this->notified && (start + nsecs) < std::chrono::high_resolution_clock::now())
		{
			this->conditionVar.wait_for(lock, nsecs - (std::chrono::high_resolution_clock::now() - start));
		}
	}

	void Lock::notifyOne()
	{
		std::this_thread::yield();
		std::lock_guard<std::mutex> _(this->mutex);

		this->notified = true;
		this->conditionVar.notify_one();
	}

	void Lock::notifyAll()
	{
		std::this_thread::yield();
		std::lock_guard<std::mutex> _(this->mutex);

		this->notified = true;
		this->conditionVar.notify_all();
	}
}
