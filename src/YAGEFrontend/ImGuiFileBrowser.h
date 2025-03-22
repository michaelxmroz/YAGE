#pragma once

#include "PlatformDefines.h"
#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <algorithm>

// Simple ImGui-based file browser that works without external dependencies
class ImGuiFileBrowser
{
public:
    enum class FileDialogType
    {
        Open,
        Save
    };

    enum class FileDialogResult
    {
        NONE,
        OK,
        Cancel
    };

    struct FileDialogItem
    {
        bool isDirectory;
        std::string name;
        std::string fullPath;
        std::uintmax_t size;
        std::string lastModified;
    };

    ImGuiFileBrowser();
    ~ImGuiFileBrowser();

    // Start a dialog for opening a file (UTF-8 version)
    void OpenDialog(const char* title = nullptr, const char* fileTypeDescription = nullptr, const char* fileTypeEndings = nullptr);
    
    // Start a dialog for opening a file (wide char version)
    void OpenDialog(const wchar_t* title, const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings);
    
    // Start a dialog for saving a file (UTF-8 version)
    void SaveDialog(const char* title = nullptr, const char* fileTypeDescription = nullptr, const char* fileTypeEndings = nullptr, const char* fileExtension = nullptr);
    
    // Start a dialog for saving a file (wide char version)
    void SaveDialog(const wchar_t* title, const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings, const wchar_t* fileExtension);
    
    // Render and process the file browser UI
    // Returns: FileDialogResult::NONE if dialog is still open
    //          FileDialogResult::OK if user selected a file
    //          FileDialogResult::Cancel if user canceled
    FileDialogResult Render();
    
    // Get the selected file path (valid after Render() returns FileDialogResult::OK)
    std::string GetSelectedFile() const;
    
private:
    void UpdateCurrentPath(const std::string& path);
    void NavigateToParentDirectory();
    void RefreshFileList();
    void FilterFiles();
    bool MatchesFilter(const std::string& filename) const;
    void SortFileList();
    void RenderPathBar();
    void RenderFileList();
    void RenderControls();
    
    // Helper functions for wide/UTF-8 string conversions
    std::string WideToUtf8(const wchar_t* wstr) const;
    std::wstring Utf8ToWide(const std::string& str) const;
    
    FileDialogType m_type;
    FileDialogResult m_result;
    
    std::string m_title;
    std::string m_fileTypeDesc;
    std::string m_fileTypeEndings;
    std::string m_fileExtension;
    
    std::string m_homeDir;
    std::string m_desktopDir;
    std::string m_documentsDir;
    std::string m_currentPath;
    
    std::string m_selectedFilename;
    std::string m_selectedFullPath;
    std::string m_inputFilename;
    char m_inputFilenameBuf[256] = {0}; // Buffer for ImGui::InputText
    
    bool m_showHidden;
    
    std::vector<FileDialogItem> m_allFiles;
    std::vector<FileDialogItem> m_filteredFiles;
}; 