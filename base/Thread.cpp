#include "pthread.h"

#include "gpwe/util/Thread.hpp"

using namespace gpwe;

Thread::Thread(Fn<void()> f)
	: m_fn(std::move(f))
{
	pthread_t id;
	auto err = pthread_create(&id, nullptr, pthread_routine, &m_fn);
	if(err){
		log::errorLn("Error in pthread_create: {}", strerror(err));
		return;
	}

	m_id = id;
}

Str Thread::name() const noexcept{
	char buf[16] = "";
	pthread_getname_np(m_id, buf, 16);
	return buf;
}

void Thread::setName(StrView name_){
	char buf[16] = "";
	std::size_t copiedLen = std::min<std::size_t>(15, name_.length());
	std::memcpy(buf, name_.data(), copiedLen);
	buf[copiedLen] = '\0';
	pthread_setname_np(m_id, buf);
}

void Thread::join(){
	if(auto id = m_id.exchange(0)){
		void *ret;
		pthread_join(id, &ret);
	}
}
