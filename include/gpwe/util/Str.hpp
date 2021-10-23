#ifndef GPWE_STRING_HPP
#define GPWE_STRING_HPP 1

#include <string>

#include "fmt/format.h"

#include "Allocator.hpp"

namespace gpwe{
	using Str = std::basic_string<char, std::char_traits<char>, Allocator<char>>;
	using StrView = std::string_view;

	template<typename String>
	inline StrView strView(const String &str_){ return str_; }

	template<meta::CStr Str_>
	inline StrView strView(){ return StrView(Str_.str, Str_.length); }

	inline Str vformat(Allocator<Str::value_type> alloc, fmt::string_view fmtStr, fmt::format_args args){
		using MemoryBuffer =
		  fmt::basic_memory_buffer<Str::value_type, fmt::inline_buffer_size, Allocator<Str::value_type>>;

		MemoryBuffer buf(alloc);
		fmt::vformat_to(std::back_inserter(buf), std::locale("en_US.UTF-8"), fmtStr, args);
		return Str(buf.data(), buf.size(), alloc);
	}

	template<typename ... Args>
	inline Str format(Allocator<Str::value_type> alloc, fmt::string_view fmtStr, const Args&... args) {
		return vformat(alloc, fmtStr, fmt::make_format_args(args...));
	}

	template<typename ... Args>
	inline Str format(fmt::string_view fmtStr, const Args&... args){
		return format(Allocator<Str::value_type>{}, fmtStr, args...);
	}
}

#endif // !GPWE_STRING_HPP
