#include <WindowPCH.hpp>
#include <FileSystem/Dialog.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

#include <Private/Windows/API/WindowHeaders.hpp>
#include <Private/Windows/API/WinPtr.hpp>

#include <commdlg.h>
#include <shobjidl.h>

#include <Log/Logger.hpp>

namespace Neon::FileSystem
{
#if NEON_DEBUG
#define RETURN_IF_FAILED(Hr)                                                                    \
    if (FAILED(Hr))                                                                             \
    {                                                                                           \
        NEON_ERROR("Failed to create file dialog instance with error code: {0}", uint64_t(Hr)); \
        return Paths;                                                                           \
    }
#endif

    std::vector<std::filesystem::path> OpenFile(
        GLFWwindow*                  Window,
        const String&                Title,
        const std::filesystem::path& InitialAbsoluteDir,
        std::span<FileDialogFilter>  Filter)
    {
        std::vector<std::filesystem::path> Paths;
        WinAPI::ComPtr<IFileDialog>        Dialog;

        RETURN_IF_FAILED(CoCreateInstance(
            CLSID_FileOpenDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&Dialog)));

        FILEOPENDIALOGOPTIONS Options;
        RETURN_IF_FAILED(Dialog->GetOptions(&Options));

        // Set the file dialog to multiselect
        RETURN_IF_FAILED(Dialog->SetOptions(Options | FOS_ALLOWMULTISELECT | FOS_PICKFOLDERS));

        Dialog->SetTitle(Title.c_str());

        if (!InitialAbsoluteDir.empty())
        {
            String InitialDir = InitialAbsoluteDir.wstring();
            // Swap all '/' with '\'
            std::replace(InitialDir.begin(), InitialDir.end(), '/', '\\');

            WinAPI::ComPtr<IShellItem> DefaultPathItem;
            if (SUCCEEDED(SHCreateItemFromParsingName(InitialDir.c_str(), nullptr, IID_PPV_ARGS(&DefaultPathItem))))
            {
                Dialog->SetFolder(DefaultPathItem.Get());
            }
        }

        HWND WindowHandle = nullptr;
        if (Window)
        {
            WindowHandle = glfwGetWin32Window(Window);
        }

        {
            std::vector<COMDLG_FILTERSPEC> FilterSpec;
            FilterSpec.reserve(Filter.size());

            for (const FileDialogFilter& FilterItem : Filter)
            {
                FilterSpec.emplace_back(
                    COMDLG_FILTERSPEC{
                        .pszName = FilterItem.Description,
                        .pszSpec = FilterItem.Extension });
            }

            Dialog->SetFileTypes(UINT(Filter.size()), FilterSpec.data());
            Dialog->SetFileTypeIndex(0);
        }

        RETURN_IF_FAILED(Dialog->Show(WindowHandle));

        WinAPI::ComPtr<IShellItemArray> Result;
        WinAPI::ComPtr<IFileOpenDialog> FileOpenDialog;

        RETURN_IF_FAILED(Dialog.As(&FileOpenDialog));
        RETURN_IF_FAILED(FileOpenDialog->GetResults(&Result));

        DWORD Count = 0;
        RETURN_IF_FAILED(Result->GetCount(&Count));

        for (DWORD i = 0; i < Count; ++i)
        {
            WinAPI::ComPtr<IShellItem> Item;
            RETURN_IF_FAILED(Result->GetItemAt(i, &Item));

            PWSTR Path = nullptr;
            RETURN_IF_FAILED(Item->GetDisplayName(SIGDN_FILESYSPATH, &Path));

            Paths.emplace_back(Path);
            CoTaskMemFree(Path);
        }

        return Paths;
    }

#undef RETURN_IF_FAILED

#if NEON_DEBUG
#define RETURN_IF_FAILED(Hr)                                                                    \
    if (FAILED(Hr))                                                                             \
    {                                                                                           \
        NEON_ERROR("Failed to create file dialog instance with error code: {0}", uint64_t(Hr)); \
        return Path;                                                                            \
    }
#endif

    std::filesystem::path SaveFile(
        const std::filesystem::path& InitialAbsoluteDir,
        std::span<FileDialogFilter>  Filter)
    {
        std::filesystem::path       Path;
        WinAPI::ComPtr<IFileDialog> Dialog;

        RETURN_IF_FAILED(CoCreateInstance(
            CLSID_FileSaveDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&Dialog)));

        FILEOPENDIALOGOPTIONS Options;
        RETURN_IF_FAILED(Dialog->GetOptions(&Options));

        // Set the file dialog to multiselect
        RETURN_IF_FAILED(Dialog->SetOptions(Options | FOS_ALLOWMULTISELECT | FOS_PICKFOLDERS));

        if (!InitialAbsoluteDir.empty())
        {
            String InitialDir = InitialAbsoluteDir.wstring();
            // Swap all '/' with '\'
            std::replace(InitialDir.begin(), InitialDir.end(), '/', '\\');

            WinAPI::ComPtr<IShellItem> DefaultPathItem;
            if (SUCCEEDED(SHCreateItemFromParsingName(InitialDir.c_str(), nullptr, IID_PPV_ARGS(&DefaultPathItem))))
            {
                Dialog->SetFolder(DefaultPathItem.Get());
            }
        }

        {
            std::vector<COMDLG_FILTERSPEC> FilterSpec;
            FilterSpec.reserve(Filter.size());

            for (const FileDialogFilter& FilterItem : Filter)
            {
                FilterSpec.emplace_back(
                    COMDLG_FILTERSPEC{
                        .pszName = FilterItem.Description,
                        .pszSpec = FilterItem.Extension });
            }

            Dialog->SetFileTypes(UINT(Filter.size()), FilterSpec.data());
            Dialog->SetFileTypeIndex(0);
        }

        RETURN_IF_FAILED(Dialog->Show(nullptr));

        WinAPI::ComPtr<IShellItem>      Result;
        WinAPI::ComPtr<IFileSaveDialog> FileSaveDialog;

        RETURN_IF_FAILED(Dialog.As(&FileSaveDialog));
        RETURN_IF_FAILED(FileSaveDialog->GetResult(&Result));

        PWSTR PathStr = nullptr;
        RETURN_IF_FAILED(Result->GetDisplayName(SIGDN_FILESYSPATH, &PathStr));

        Path = PathStr;
        CoTaskMemFree(PathStr);

        return Path;
    }
} // namespace Neon::FileSystem

#undef RETURN_IF_FAILED