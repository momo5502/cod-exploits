#pragma once

namespace Utils
{
	namespace Time
	{
		class Interval
		{
		protected:
			std::chrono::high_resolution_clock::time_point lastPoint;

		public:
			Interval() : lastPoint(std::chrono::high_resolution_clock::now()) {}

			void update();
			void zero();
			bool elapsed(std::chrono::nanoseconds nsecs);
		};
	}
}
