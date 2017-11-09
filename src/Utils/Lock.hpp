#pragma once

#ifndef STD_INCLUDED
#error "Missing standard header"
#endif

namespace Utils
{
	class Lock
	{
	public:
		~Lock();

		void wait();
		void waitFor(std::chrono::nanoseconds nsecs);

		void notifyOne();
		void notifyAll();

	private:
		std::mutex mutex;
		std::atomic<bool> notified;
		std::condition_variable conditionVar;
	};
}
