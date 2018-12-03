#include "STDInclude.hpp"

namespace Utils
{
	namespace Time
	{
		void Interval::update()
		{
			this->lastPoint = std::chrono::high_resolution_clock::now();
		}

		void Interval::zero()
		{
			this->lastPoint = std::chrono::high_resolution_clock::time_point();
		}

		bool Interval::elapsed(std::chrono::nanoseconds nsecs)
		{
			return ((std::chrono::high_resolution_clock::now() - this->lastPoint) >= nsecs);
		}
	}
}
