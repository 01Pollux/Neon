#include <CorePCH.hpp>
#include <Private/IO/efsw/FileSystem.hpp>
#include <Private/IO/efsw/FileWatcherGeneric.hpp>
#include <Private/IO/efsw/FileWatcherImpl.hpp>
#include <IO/FileWatcher.hpp>

#if EFSW_PLATFORM == EFSW_PLATFORM_WIN32
#include <Private/IO/efsw/FileWatcherWin32.hpp>
#define FILEWATCHER_IMPL FileWatcherWin32
#define BACKEND_NAME     "Win32"
#elif EFSW_PLATFORM == EFSW_PLATFORM_INOTIFY
#include <Private/IO/efsw/FileWatcherInotify.hpp>
#define FILEWATCHER_IMPL FileWatcherInotify
#define BACKEND_NAME     "Inotify"
#elif EFSW_PLATFORM == EFSW_PLATFORM_KQUEUE
#include <Private/IO/efsw/FileWatcherKqueue.hpp>
#define FILEWATCHER_IMPL FileWatcherKqueue
#define BACKEND_NAME     "Kqueue"
#elif EFSW_PLATFORM == EFSW_PLATFORM_FSEVENTS
#include <Private/IO/efsw/FileWatcherFSEvents.hpp>
#define FILEWATCHER_IMPL FileWatcherFSEvents
#define BACKEND_NAME     "FSEvents"
#else
#define FILEWATCHER_IMPL FileWatcherGeneric
#define BACKEND_NAME     "Generic"
#endif

#include <Log/Logger.hpp>

namespace efsw
{

    FileWatcher::FileWatcher() :
        mFollowSymlinks(false), mOutOfScopeLinks(false)
    {
        mImpl = new FILEWATCHER_IMPL(this);

        if (!mImpl->initOK())
        {
            efSAFE_DELETE(mImpl);

            mImpl = new FileWatcherGeneric(this);
        }
    }

    FileWatcher::FileWatcher(bool useGenericFileWatcher) :
        mFollowSymlinks(false), mOutOfScopeLinks(false)
    {
        if (useGenericFileWatcher)
        {
            mImpl = NEON_NEW FileWatcherGeneric(this);
        }
        else
        {
            mImpl = NEON_NEW FILEWATCHER_IMPL(this);

            if (!mImpl->initOK())
            {
                efSAFE_DELETE(mImpl);

                mImpl = NEON_NEW FileWatcherGeneric(this);
            }
        }
    }

    FileWatcher::~FileWatcher()
    {
        efSAFE_DELETE(mImpl);
    }

    WatchID FileWatcher::addWatch(const std::string& directory, FileWatchListener* watcher)
    {
        return addWatch(directory, watcher, false, {});
    }

    WatchID FileWatcher::addWatch(const std::string& directory, FileWatchListener* watcher,
                                  bool recursive)
    {
        return addWatch(directory, watcher, recursive, {});
    }

    WatchID FileWatcher::addWatch(const std::string& directory, FileWatchListener* watcher,
                                  bool recursive, const std::vector<WatcherOption>& options)
    {
        if (mImpl->mIsGeneric || !FileSystem::isRemoteFS(directory))
        {
            return mImpl->addWatch(directory, watcher, recursive, options);
        }
        else
        {
            return Errors::Log::createLastError(Errors::FileRemote, directory);
        }
    }

    void FileWatcher::removeWatch(const std::string& directory)
    {
        mImpl->removeWatch(directory);
    }

    void FileWatcher::removeWatch(WatchID watchid)
    {
        mImpl->removeWatch(watchid);
    }

    void FileWatcher::watch()
    {
        mImpl->watch();
    }

    std::vector<std::string> FileWatcher::directories()
    {
        return mImpl->directories();
    }

    void FileWatcher::followSymlinks(bool follow)
    {
        mFollowSymlinks = follow;
    }

    const bool& FileWatcher::followSymlinks() const
    {
        return mFollowSymlinks;
    }

    void FileWatcher::allowOutOfScopeLinks(bool allow)
    {
        mOutOfScopeLinks = allow;
    }

    const bool& FileWatcher::allowOutOfScopeLinks() const
    {
        return mOutOfScopeLinks;
    }

} // namespace efsw
