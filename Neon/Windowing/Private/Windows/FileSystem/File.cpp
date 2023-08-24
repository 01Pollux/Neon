#include <WindowPCH.hpp>
#include <FileSystem/File.hpp>
#include <Core/String.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

#include <Private/Windows/API/WindowHeaders.hpp>
#include <Private/Windows/API/WinPtr.hpp>

namespace Neon::FileSystem
{
    void ShowFileInExplorer(
        const std::filesystem::path& Path)
    {
        auto Command = StringUtils::Format("explorer.exe /select,\"{}\"", Path.string());
        system(Command.c_str());
    }

    void OpenDirectoryInExplorer(
        const std::filesystem::path& Path)
    {
        ShellExecuteW(nullptr, L"open", Path.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
    }

    void OpenExternally(
        const std::filesystem::path& Path)
    {
        ShellExecuteW(nullptr, L"open", Path.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
    }
} // namespace Neon::FileSystem