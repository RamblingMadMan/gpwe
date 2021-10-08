#ifndef GPWE_LOG_HPP
#define GPWE_LOG_HPP 1

#include <functional>

#include "fmt/core.h"

namespace gpwe{
	template<typename Str, typename ... Args>
	void log(Str &&fmtStr, Args &&... args){
		auto res = fmt::format(std::forward<Str>(fmtStr), std::forward<Args>(args)...);
		// TODO: push res to a file/buffer
		fmt::print("{}", res);
	}
}

#endif // !GPWE_LOG_HPP
