#include <CorePCH.hpp>
#include <Private/IO/efsw/Thread.hpp>
#include <Private/IO/efsw/platform/posix/ThreadImpl.hpp>

#if defined(EFSW_PLATFORM_POSIX)

#include <cassert>
#include <iostream>

#include <Log/Logger.hpp>

namespace efsw
{
    namespace Platform
    {

        ThreadImpl::ThreadImpl(efsw::Thread* owner) :
            mIsActive(false)
        {
            mIsActive = pthread_create(&mThread, NULL, &ThreadImpl::entryPoint, owner) == 0;

            NEON_ASSERT(mIsActive, "Failed to create thread");
        }

        ThreadImpl::~ThreadImpl()
        {
            terminate();
        }

        void ThreadImpl::wait()
        {
            // Wait for the thread to finish, no timeout
            if (mIsActive)
            {
                assert(pthread_equal(pthread_self(), mThread) == 0);

                mIsActive = pthread_join(mThread, NULL) != 0;
            }
        }

        void ThreadImpl::terminate()
        {
            if (mIsActive)
            {
#if !defined(__ANDROID__) && !defined(ANDROID)
                pthread_cancel(mThread);
#else
                pthread_kill(mThread, SIGUSR1);
#endif

                mIsActive = false;
            }
        }

        void* ThreadImpl::entryPoint(void* userData)
        {
// Tell the thread to handle cancel requests immediatly
#ifdef PTHREAD_CANCEL_ASYNCHRONOUS
            pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif

            // The Thread instance is stored in the user data
            Thread* owner = static_cast<Thread*>(userData);

            // Forward to the owner
            owner->run();

            return NULL;
        }

    } // namespace Platform
} // namespace efsw

#endif
