#pragma once

#ifndef STD_INCLUDED
#error "Missing standard header"
#endif

namespace Utils
{
	template <typename T> using Slot = std::function<T>;
	template <typename T>
	class Signal
	{
	public:
		Signal()
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex);

			this->slots.clear();
		}

		Signal(Signal& obj) : Signal()
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex);
			std::lock_guard<std::recursive_mutex> __(obj.mutex);

			Utils::Merge(&this->slots, obj.getSlots());
		}

		void connect(Slot<T> slot)
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex);

			if (slot)
			{
				this->slots.push_back(slot);
			}
		}

		void clear()
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex);

			this->slots.clear();
		}

		std::vector<Slot<T>>& getSlots()
		{
			return this->slots;
		}

		template <class ...Args>
		void operator()(Args&&... args) const
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex);

			std::vector<Slot<T>> copiedSlots;
			Utils::Merge(&copiedSlots, this->slots);

			for (auto& slot : copiedSlots)
			{
				if (slot)
				{
					slot(std::forward<Args>(args)...);
				}
			}
		}

	private:
		mutable std::recursive_mutex mutex;
		std::vector<Slot<T>> slots;
	};
}