#ifndef EFSW_THREADIMPLPOSIX_HPP
#define EFSW_THREADIMPLPOSIX_HPP

#include <Private/IO/efsw/base.hpp>

#if defined( EFSW_PLATFORM_POSIX )

#include <Private/IO/efsw/Atomic.hpp>
#include <pthread.h>

namespace efsw {

class Thread;

namespace Platform {

class ThreadImpl {
  public:
	explicit ThreadImpl( efsw::Thread* owner );

	~ThreadImpl();

	void wait();

	void terminate();

  protected:
	static void* entryPoint( void* userData );

	pthread_t mThread;
	Atomic<bool> mIsActive;
};

} // namespace Platform
} // namespace efsw

#endif

#endif
