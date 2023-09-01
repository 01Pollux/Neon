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
#define RETURN_IF_FAILED(Hr)                                                                      \
    if (FAILED(Hr))                                                                               \
    {                                                                                             \
        NEON_ERROR("Failed to create file dialog instance with error code: {0:X}", uint64_t(Hr)); \
        return Paths;                                                                             \
    }

    std::vector<std::filesystem::path> FileDialog::OpenFileDialog(
        DialogType Type) const
    {
        std::vector<std::filesystem::path> Paths;
        WinAPI::ComPtr<IFileDialog>        Dialog;

        const CLSID* ComId       = nullptr;
        const IID*   InterfaceId = nullptr;

        switch (Type)
        {
        case DialogType::Save:
            ComId       = &CLSID_FileSaveDialog;
            InterfaceId = &IID_IFileSaveDialog;
            break;
        case DialogType::Open:
            ComId       = &CLSID_FileOpenDialog;
            InterfaceId = &IID_IFileOpenDialog;
            break;
        case DialogType::Folder:
            ComId       = &CLSID_FileOpenDialog;
            InterfaceId = &IID_IFileOpenDialog;
            break;
        default:
            break;
        }

        RETURN_IF_FAILED(CoCreateInstance(
            *ComId,
            NULL,
            CLSCTX_INPROC_SERVER,
            *InterfaceId,
            IID_PPV_ARGS_Helper(&Dialog)));

        switch (Type)
        {
        case DialogType::Open:
        {
            // Set the file dialog to multiselect
            FILEOPENDIALOGOPTIONS Options = 0;
            RETURN_IF_FAILED(Dialog->GetOptions(&Options));
            RETURN_IF_FAILED(Dialog->SetOptions(Options | FOS_ALLOWMULTISELECT));
            break;
        }
        case DialogType::Folder:
        {
            // Set the file dialog to multiselect
            FILEOPENDIALOGOPTIONS Options = 0;
            RETURN_IF_FAILED(Dialog->GetOptions(&Options));
            RETURN_IF_FAILED(Dialog->SetOptions(Options | FOS_ALLOWMULTISELECT | FOS_PICKFOLDERS));
            break;
        }
        }

        Dialog->SetTitle(Title.c_str());

        if (!InitialDir.empty())
        {
            String Dir = InitialDir.wstring();
            // Swap all '/' with '\'
            std::replace(Dir.begin(), Dir.end(), '/', '\\');

            WinAPI::ComPtr<IShellItem> DefaultPathItem;
            if (SUCCEEDED(SHCreateItemFromParsingName(Dir.c_str(), nullptr, IID_PPV_ARGS(&DefaultPathItem))))
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
            std::vector<COMDLG_FILTERSPEC> FilterSpecs;
            FilterSpecs.reserve(Filters.size());

            for (const FileDialogFilter& FilterItem : Filters)
            {
                FilterSpecs.emplace_back(
                    COMDLG_FILTERSPEC{
                        .pszName = FilterItem.Description,
                        .pszSpec = FilterItem.Extension });
            }

            Dialog->SetFileTypes(UINT(Filters.size()), FilterSpecs.data());
            Dialog->SetFileTypeIndex(0);
        }

        if (SUCCEEDED(Dialog->Show(WindowHandle)))
        {
            if (Type == DialogType::Save)
            {
                WinAPI::ComPtr<IShellItem>      Result;
                WinAPI::ComPtr<IFileSaveDialog> FileSaveDialog;

                RETURN_IF_FAILED(Dialog.As(&FileSaveDialog));
                RETURN_IF_FAILED(FileSaveDialog->GetResult(&Result));

                PWSTR Path = nullptr;
                RETURN_IF_FAILED(Result->GetDisplayName(SIGDN_FILESYSPATH, &Path));

                Paths.emplace_back(Path);
                CoTaskMemFree(Path);
            }
            else
            {
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
            }
        }

        return Paths;
    }
} // namespace Neon::FileSystem

#undef RETURN_IF_FAILED