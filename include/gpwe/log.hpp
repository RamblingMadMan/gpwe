#ifndef GPWE_LOG_HPP
#define GPWE_LOG_HPP 1

#include <functional>

#include "fmt/core.h"

namespace gpwe{
	enum class LogKind{
		info,
		warning,
		error,

		count
	};

	template<bool Print = true, typename Str, typename ... Args>
	void log(LogKind kind, Str &&fmtStr, Args &&... args){
		std::FILE *fd = stdout;

		switch(kind){
			case LogKind::warning:
			case LogKind::error:{
				fd = stderr;
				break;
			}
			default: break;
		}

		auto str = fmt::format(std::forward<Str>(fmtStr), std::forward<Args>(args)...);

		if constexpr(Print){
			fmt::print(fd, "{}", str);
		}

		// TODO: log to buffer/file
	}

	template<bool Print = true, typename Str, typename ... Args>
	void log(Str &&fmtStr, Args &&... args){
		log<Print>(LogKind::info, std::forward<Str>(fmtStr), std::forward<Args>(args)...);
	}

	template<bool Print = true, typename Str, typename ... Args>
	void logLn(LogKind kind, Str &&fmtStr, Args &&... args){
		auto res = fmt::format(std::forward<Str>(fmtStr), std::forward<Args>(args)...);
		log<Print>(kind, "{}\n", res);
	}

	template<bool Print = true, typename Str, typename ... Args>
	void logLn(Str &&fmtStr, Args &&... args){
		logLn<Print>(LogKind::info, std::forward<Str>(fmtStr), std::forward<Args>(args)...);
	}

	template<bool Print = true, typename Str, typename ... Args>
	void logError(Str &&fmtStr, Args &&... args){
		log<Print>(LogKind::error, std::forward<Str>(fmtStr), std::forward<Args>(args)...);
	}

	template<bool Print = true, typename Str, typename ... Args>
	void logErrorLn(Str &&fmtStr, Args &&... args){
		logLn<Print>(LogKind::error, std::forward<Str>(fmtStr), std::forward<Args>(args)...);
	}
}

#endif // !GPWE_LOG_HPP
