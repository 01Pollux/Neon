#ifndef EFSW_PLATFORMIMPL_HPP
#define EFSW_PLATFORMIMPL_HPP

#include <Private/IO/efsw/base.hpp>

#if defined( EFSW_PLATFORM_POSIX )
#include <Private/IO/efsw/platform/posix/ThreadImpl.hpp>
#include <Private/IO/efsw/platform/posix/MutexImpl.hpp>
#include <Private/IO/efsw/platform/posix/SystemImpl.hpp>
#include <Private/IO/efsw/platform/posix/FileSystemImpl.hpp>
#elif EFSW_PLATFORM == EFSW_PLATFORM_WIN32
#include <Private/IO/efsw/platform/win/ThreadImpl.hpp>
#include <Private/IO/efsw/platform/win/MutexImpl.hpp>
#include <Private/IO/efsw/platform/win/SystemImpl.hpp>
#include <Private/IO/efsw/platform/win/FileSystemImpl.hpp>
#else
#error Thread, Mutex, and System not implemented for this platform.
#endif

#endif
