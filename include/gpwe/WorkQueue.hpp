#ifndef GPWE_WORKQUEUE_HPP
#define GPWE_WORKQUEUE_HPP 1

#include <future>

#include "List.hpp"

namespace gpwe{
	class WorkQueue{
		public:

		private:
			List<std::packaged_task<void()>> m_tasks;
	};
}

#endif // !GPWE_WORKQUEUE_HPP
