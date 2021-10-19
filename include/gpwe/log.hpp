#ifndef GPWE_LOG_HPP
#define GPWE_LOG_HPP 1

#include <functional>
#include <chrono>

#include "fmt/core.h"

#include "util/List.hpp"
#include "util/Str.hpp"
#include "Manager.hpp"

namespace gpwe::log{
	class Manager;
}

namespace gpwe{
	using LogManager = log::Manager;

	template<typename String, typename ... Args>
	inline Str format(String &&str, Args &&... args){
		Str ret;
		fmt::format_to(std::back_inserter(ret), std::forward<String>(str), std::forward<Args>(args)...);
		return ret;
	}
}

namespace gpwe::sys{
	LogManager *logManager() noexcept;
}

namespace gpwe::log{
	enum class Kind{
		info, warning, error,
		count
	};

	struct Entry{
		using TimePoint = std::chrono::system_clock::time_point;
		TimePoint time;
		Kind kind;
		Str str;
	};

	class Manager: public gpwe::Manager<Manager, ManagerKind::log>{
		public:
			using Clock = std::chrono::system_clock;

			template<typename String, typename ... Args>
			void log(Kind kind, String &&str, Args &&... args){
				auto &&entry = m_entries.emplace_back(Entry{ Clock::now(), kind, format(std::forward<String>(str), std::forward<Args>(args)...) });
				return doLog(&entry);
			}

			template<typename String, typename ... Args>
			void logLn(Kind kind, String &&str, Args &&... args){
				return log(kind, "{}\n", format(std::forward<String>(str), std::forward<Args>(args)...));
			}

			template<typename String, typename ... Args>
			void logInfo(String &&str, Args &&... args){
				return log(Kind::info, std::forward<String>(str), std::forward<Args>(args)...);
			}

			template<typename String, typename ... Args>
			void logInfoLn(String &&str, Args &&... args){
				return logLn(Kind::info, std::forward<String>(str), std::forward<Args>(args)...);
			}

			template<typename String, typename ... Args>
			void logWarn(String &&str, Args &&... args){
				return log(Kind::warning, std::forward<String>(str), std::forward<Args>(args)...);
			}

			template<typename String, typename ... Args>
			void logWarnLn(String &&str, Args &&... args){
				return logLn(Kind::warning, std::forward<String>(str), std::forward<Args>(args)...);
			}

			template<typename String, typename ... Args>
			void logError(String &&str, Args &&... args){
				return log(Kind::error, std::forward<String>(str), std::forward<Args>(args)...);
			}

			template<typename String, typename ... Args>
			void logErrorLn(String &&str, Args &&... args){
				return logLn(Kind::error, std::forward<String>(str), std::forward<Args>(args)...);
			}

			const List<Entry> &entries() const noexcept{ return m_entries; }

		protected:
			virtual void doLog(const Entry *entry){
				std::FILE *fd = stdout;

				switch(entry->kind){
					case Kind::warning:
					case Kind::error:{
						fd = stderr;
						break;
					}
					default: break;
				}


				std::fprintf(fd, "%s", entry->str.c_str());
				std::fflush(fd);
			}

		private:
			List<Entry> m_entries;
	};

	inline Manager *manager() noexcept{ return sys::logManager(); }

	template<typename String, typename ... Args>
	inline void out(Kind kind, String &&str, Args &&... args){
		sys::logManager()->log(kind, std::forward<String>(str), std::forward<Args>(args)...);
	}

	template<typename String, typename ... Args>
	inline void outLn(Kind kind, String &&str, Args &&... args){
		sys::logManager()->logLn(kind, std::forward<String>(str), std::forward<Args>(args)...);
	}

	template<typename String, typename ... Args>
	inline void out(String &&str, Args &&... args){
		out(Kind::info, std::forward<String>(str), std::forward<Args>(args)...);
	}

	template<typename String, typename ... Args>
	inline void outLn(String &&str, Args &&... args){
		outLn(Kind::info, std::forward<String>(str), std::forward<Args>(args)...);
	}

	template<typename String, typename ... Args>
	inline void info(String &&str, Args &&... args){
		out(Kind::info, std::forward<String>(str), std::forward<Args>(args)...);
	}

	template<typename String, typename ... Args>
	inline void infoLn(String &&str, Args &&... args){
		outLn(Kind::info, std::forward<String>(str), std::forward<Args>(args)...);
	}

	template<typename String, typename ... Args>
	inline void warn(String &&str, Args &&... args){
		out(Kind::warning, std::forward<String>(str), std::forward<Args>(args)...);
	}

	template<typename String, typename ... Args>
	inline void warnLn(String &&str, Args &&... args){
		outLn(Kind::warning, std::forward<String>(str), std::forward<Args>(args)...);
	}

	template<typename String, typename ... Args>
	inline void error(String &&str, Args &&... args){
		out(Kind::error, std::forward<String>(str), std::forward<Args>(args)...);
	}

	template<typename String, typename ... Args>
	inline void errorLn(String &&str, Args &&... args){
		outLn(Kind::error, std::forward<String>(str), std::forward<Args>(args)...);
	}
}

#endif // !GPWE_LOG_HPP
