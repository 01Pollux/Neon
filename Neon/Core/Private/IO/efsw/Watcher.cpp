#include <CorePCH.hpp>
#include <Private/IO/efsw/Watcher.hpp>

namespace efsw
{

    Watcher::Watcher() :
        ID(0), Directory(""), Listener(NULL), Recursive(false)
    {
    }

    Watcher::Watcher(WatchID id, std::string directory, FileWatchListener* listener, bool recursive) :
        ID(id), Directory(directory), Listener(listener), Recursive(recursive)
    {
    }

} // namespace efsw
