#include <CorePCH.hpp>
#include <Private/IO/efsw/FileWatcherImpl.hpp>
#include <Private/IO/efsw/String.hpp>
#include <Private/IO/efsw/System.hpp>

namespace efsw
{

    FileWatcherImpl::FileWatcherImpl(FileWatcher* parent) :
        mFileWatcher(parent), mInitOK(false), mIsGeneric(false)
    {
        System::maxFD();
    }

    FileWatcherImpl::~FileWatcherImpl()
    {
    }

    bool FileWatcherImpl::initOK()
    {
        return static_cast<bool>(mInitOK);
    }

    bool FileWatcherImpl::linkAllowed(const std::string& curPath, const std::string& link)
    {
        return (mFileWatcher->followSymlinks() && mFileWatcher->allowOutOfScopeLinks()) ||
               -1 != String::strStartsWith(curPath, link);
    }

    int FileWatcherImpl::getOptionValue(const std::vector<WatcherOption>& options,
                                        Option option, int defaultValue)
    {
        for (int i = 0; i < options.size(); i++)
        {
            if (options[i].mOption == option)
            {
                return options[i].mValue;
            }
        }

        return defaultValue;
    }

} // namespace efsw