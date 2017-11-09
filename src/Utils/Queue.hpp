#pragma once

namespace Utils
{
	template<typename T>
	class Queue
	{
	public:
		bool empty()
		{
			//std::lock_guard<std::mutex> _(mutex);
			return this->queue.empty();
		}

		T front()
		{
			std::lock_guard<std::mutex> _(mutex);
			return this->queue.front();
		}
		
		void pop()
		{
			std::lock_guard<std::mutex> _(mutex);
			return this->queue.pop();
		}

		void push(T obj)
		{
			std::lock_guard<std::mutex> _(mutex);
			return this->queue.push(obj);
		}

		size_t size()
		{
			std::lock_guard<std::mutex> _(mutex);
			return this->queue.size();
		}

	private:
		std::mutex mutex;
		std::queue<T> queue;
	};
}
