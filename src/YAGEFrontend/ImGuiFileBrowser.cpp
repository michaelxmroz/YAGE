#include "ImGuiFileBrowser.h"
#include "PlatformDefines.h"
#include "imgui.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <iostream>

#if YAGE_PLATFORM_UNIX
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#elif YAGE_PLATFORM_WINDOWS 
#include <Windows.h>
#include <ShlObj.h>
#endif

namespace fs = std::filesystem;

ImGuiFileBrowser::ImGuiFileBrowser()
    : m_type(FileDialogType::Open)
    , m_showHidden(false)
    , m_result(FileDialogResult::NONE)
{
    // Get home directory
#if YAGE_PLATFORM_UNIX
    const char* homeDir = getenv("HOME");
    
    if (!homeDir) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            homeDir = pw->pw_dir;
        }
    }
#elif YAGE_PLATFORM_WINDOWS
    char homeDir[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, homeDir))) {
        // Windows path retrieved successfully
    }
    else {
        // Fallback to environment variable
        homeDir[0] = '\0';
        const char* userProfile = getenv("USERPROFILE");
        if (userProfile) {
            strncpy(homeDir, userProfile, MAX_PATH - 1);
            homeDir[MAX_PATH - 1] = '\0';
        }
    }
#endif
    
    if (homeDir && homeDir[0] != '\0') {
        m_homeDir = homeDir;
#if YAGE_PLATFORM_UNIX
        m_desktopDir = m_homeDir + "/Desktop";
        m_documentsDir = m_homeDir + "/Documents";
#elif YAGE_PLATFORM_WINDOWS
        m_desktopDir = m_homeDir + "\\Desktop";
        m_documentsDir = m_homeDir + "\\Documents";
#endif
        m_currentPath = m_homeDir;
    } else {
        // Fallback to current directory
        m_currentPath = fs::current_path().string();
    }
    
    RefreshFileList();
}

ImGuiFileBrowser::~ImGuiFileBrowser()
{
}

void ImGuiFileBrowser::OpenDialog(const char* title, const char* fileTypeDescription, const char* fileTypeEndings)
{
    m_type = FileDialogType::Open;
    m_title = title ? title : "Open File";
    m_fileTypeDesc = fileTypeDescription ? fileTypeDescription : "All Files";
    m_fileTypeEndings = fileTypeEndings ? fileTypeEndings : "*.*";
    m_selectedFilename.clear();
    m_selectedFullPath.clear();
    m_inputFilename.clear();
    memset(m_inputFilenameBuf, 0, sizeof(m_inputFilenameBuf));
    m_result = FileDialogResult::NONE;
    
    RefreshFileList();
}

void ImGuiFileBrowser::OpenDialog(const wchar_t* title, const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings)
{
    m_type = FileDialogType::Open;
    m_title = title ? WideToUtf8(title) : "Open File";
    m_fileTypeDesc = fileTypeDescription ? WideToUtf8(fileTypeDescription) : "All Files";
    m_fileTypeEndings = fileTypeEndings ? WideToUtf8(fileTypeEndings) : "*.*";
    m_selectedFilename.clear();
    m_selectedFullPath.clear();
    m_inputFilename.clear();
    memset(m_inputFilenameBuf, 0, sizeof(m_inputFilenameBuf));
    m_result = FileDialogResult::NONE;
    
    RefreshFileList();
}

void ImGuiFileBrowser::SaveDialog(const char* title, const char* fileTypeDescription, const char* fileTypeEndings, const char* fileExtension)
{
    m_type = FileDialogType::Save;
    m_title = title ? title : "Save File";
    m_fileTypeDesc = fileTypeDescription ? fileTypeDescription : "All Files";
    m_fileTypeEndings = fileTypeEndings ? fileTypeEndings : "*.*";
    m_fileExtension = fileExtension ? fileExtension : "";
    m_selectedFilename.clear();
    m_selectedFullPath.clear();
    m_inputFilename.clear();
    memset(m_inputFilenameBuf, 0, sizeof(m_inputFilenameBuf));
    m_result = FileDialogResult::NONE;
    
    RefreshFileList();
}

void ImGuiFileBrowser::SaveDialog(const wchar_t* title, const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings, const wchar_t* fileExtension)
{
    m_type = FileDialogType::Save;
    m_title = title ? WideToUtf8(title) : "Save File";
    m_fileTypeDesc = fileTypeDescription ? WideToUtf8(fileTypeDescription) : "All Files";
    m_fileTypeEndings = fileTypeEndings ? WideToUtf8(fileTypeEndings) : "*.*";
    m_fileExtension = fileExtension ? WideToUtf8(fileExtension) : "";
    m_selectedFilename.clear();
    m_selectedFullPath.clear();
    m_inputFilename.clear();
    memset(m_inputFilenameBuf, 0, sizeof(m_inputFilenameBuf));
    m_result = FileDialogResult::NONE;
    
    RefreshFileList();
}

std::string ImGuiFileBrowser::WideToUtf8(const wchar_t* wstr) const
{
    if (!wstr) return std::string();
    
    // Determine required buffer size
    size_t size_needed = wcstombs(nullptr, wstr, 0) + 1;
    if (size_needed == 0) return std::string();
    
    // Allocate buffer
    std::string result(size_needed, 0);
    
    // Convert wide string to UTF-8
    size_t bytes_written = wcstombs(&result[0], wstr, size_needed);
    if (bytes_written == (size_t)-1) return std::string();
    
    // Resize to actual length (excluding null terminator)
    result.resize(bytes_written);
    return result;
}

std::wstring ImGuiFileBrowser::Utf8ToWide(const std::string& str) const
{
    if (str.empty()) return std::wstring();
    
    // Determine required buffer size
    size_t size_needed = mbstowcs(nullptr, str.c_str(), 0) + 1;
    if (size_needed == 0) return std::wstring();
    
    // Allocate buffer
    std::wstring result(size_needed, 0);
    
    // Convert UTF-8 to wide string
    size_t chars_written = mbstowcs(&result[0], str.c_str(), size_needed);
    if (chars_written == (size_t)-1) return std::wstring();
    
    // Resize to actual length (excluding null terminator)
    result.resize(chars_written);
    return result;
}

ImGuiFileBrowser::FileDialogResult ImGuiFileBrowser::Render()
{
    if (m_result != FileDialogResult::NONE) {
        return m_result;
    }
    
    ImGui::SetNextWindowSize(ImVec2(700, 450), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin(m_title.c_str(), nullptr, ImGuiWindowFlags_NoCollapse))
    {
        RenderPathBar();
        RenderFileList();
        RenderControls();
    }
    ImGui::End();
    
    return m_result;
}

std::string ImGuiFileBrowser::GetSelectedFile() const
{
    return m_selectedFullPath;
}

void ImGuiFileBrowser::UpdateCurrentPath(const std::string& path)
{
    m_currentPath = path;
    m_inputFilename.clear();
    RefreshFileList();
}

void ImGuiFileBrowser::NavigateToParentDirectory()
{
    fs::path current(m_currentPath);
    if (current.has_parent_path()) {
        UpdateCurrentPath(current.parent_path().string());
    }
}

void ImGuiFileBrowser::RefreshFileList()
{
    m_allFiles.clear();
    m_filteredFiles.clear();
    
    try {
        // Add parent directory entry
        fs::path current(m_currentPath);
        if (current.has_parent_path()) {
            FileDialogItem parentDir;
            parentDir.isDirectory = true;
            parentDir.name = "..";
            parentDir.fullPath = current.parent_path().string();
            parentDir.size = 0;
            parentDir.lastModified = "";
            m_allFiles.push_back(parentDir);
        }
        
        // List current directory contents
        for (const auto& entry : fs::directory_iterator(m_currentPath)) {
            try {
                // Skip hidden files unless explicitly shown
                if (!m_showHidden && entry.path().filename().string()[0] == '.') {
                    continue;
                }
                
                FileDialogItem item;
                item.isDirectory = entry.is_directory();
                item.name = entry.path().filename().string();
                item.fullPath = entry.path().string();
                
                // Get file size (0 for directories)
                if (item.isDirectory) {
                    item.size = 0;
                } else {
                    item.size = entry.file_size();
                }
                
                // Get last modified time
                auto lastWriteTime = entry.last_write_time();
                
                // Create a formatted time string (C++17 compatible approach)
                std::stringstream ss;
                
                // Use a simpler approach that works with most compilers
                // Convert to time_t directly from file_time_type
                #if YAGE_PLATFORM_WINDOWS
                    auto systemTime = std::chrono::system_clock::to_time_t(
                        std::chrono::system_clock::now()); // Fallback to current time on Windows
                    
                    std::tm timeinfo;
                    localtime_s(&timeinfo, &systemTime);
                    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M");
                #else
                    struct stat st;
                    if (stat(entry.path().string().c_str(), &st) == 0) {
                        auto systemTime = st.st_mtime;
                        
                        std::tm timeinfo;
                        localtime_r(&systemTime, &timeinfo);
                        ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M");
                    } else {
                        ss << "Unknown";
                    }
                #endif
                
                item.lastModified = ss.str();
                
                m_allFiles.push_back(item);
            } catch (const std::exception& e) {
                // Skip entries that can't be accessed
                continue;
            }
        }
        
        SortFileList();
        FilterFiles();
    } catch (const std::exception& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
        
        // Fallback to home directory if current path is invalid
        if (m_currentPath != m_homeDir && !m_homeDir.empty()) {
            m_currentPath = m_homeDir;
            RefreshFileList();
        }
    }
}

void ImGuiFileBrowser::FilterFiles()
{
    m_filteredFiles.clear();
    
    // Always include directories
    for (const auto& item : m_allFiles) {
        if (item.isDirectory || item.name == "..") {
            m_filteredFiles.push_back(item);
            continue;
        }
        
        // Filter files based on extension
        if (m_fileTypeEndings == "*.*" || m_fileTypeEndings.empty() || MatchesFilter(item.name)) {
            m_filteredFiles.push_back(item);
        }
    }
}

bool ImGuiFileBrowser::MatchesFilter(const std::string& filename) const
{
    if (m_fileTypeEndings == "*.*" || m_fileTypeEndings.empty()) {
        return true;
    }
    
    // Split filter by semicolons and check each one
    size_t start = 0;
    size_t end = m_fileTypeEndings.find(';');
    
    while (start < m_fileTypeEndings.size()) {
        std::string filter = m_fileTypeEndings.substr(start, (end == std::string::npos) ? end : end - start);
        
        // Convert "*.ext" to just ".ext"
        if (filter.size() > 1 && filter[0] == '*') {
            filter = filter.substr(1);
        }
        
        // Check if the filename ends with the filter
        if (filename.size() >= filter.size() && 
            filename.substr(filename.size() - filter.size()) == filter) {
            return true;
        }
        
        if (end == std::string::npos) {
            break;
        }
        
        start = end + 1;
        end = m_fileTypeEndings.find(';', start);
    }
    
    return false;
}

void ImGuiFileBrowser::SortFileList()
{
    // Sort: directories first, then files, alphabetical order
    std::sort(m_allFiles.begin(), m_allFiles.end(), [](const FileDialogItem& a, const FileDialogItem& b) {
        // Special case for parent directory
        if (a.name == "..") return true;
        if (b.name == "..") return false;
        
        // Directories come before files
        if (a.isDirectory && !b.isDirectory) return true;
        if (!a.isDirectory && b.isDirectory) return false;
        
        // Alphabetical order within each group
        return a.name < b.name;
    });
}

void ImGuiFileBrowser::RenderPathBar()
{
    ImGui::Text("Location: ");
    ImGui::SameLine();
    
    // Display current path with clickable segments
    fs::path current(m_currentPath);
    fs::path accumulated;
    
    // Max width for the path bar
    float maxWidth = ImGui::GetContentRegionAvail().x - 200;
    float usedWidth = 0;
    
    // Collect path parts
    std::vector<std::string> parts;
    for (const auto& part : current) {
        parts.push_back(part.string());
    }
    
    // Handle root directory
    if (current.has_root_name()) {
        accumulated = current.root_name();
        if (ImGui::Button(accumulated.string().c_str())) {
            UpdateCurrentPath(accumulated.string());
            return;
        }
        ImGui::SameLine();
        ImGui::Text("/");
        ImGui::SameLine();
        accumulated /= "/";
    }
    
    // Display path parts
    bool needsEllipsis = false;
    int startPart = 0;
    
    // If the path is long, start from later parts
    if (parts.size() > 4) {
        needsEllipsis = true;
        startPart = parts.size() - 4;
    }
    
    if (needsEllipsis) {
        ImGui::Text("...");
        ImGui::SameLine();
    }
    
    for (int i = startPart; i < parts.size(); i++) {
        accumulated /= parts[i];
        
        if (ImGui::Button(parts[i].c_str())) {
            UpdateCurrentPath(accumulated.string());
            break;
        }
        
        if (i < parts.size() - 1) {
            ImGui::SameLine();
            ImGui::Text("/");
            ImGui::SameLine();
        }
    }
    
    ImGui::SameLine();
    
    // Quick access buttons
    if (ImGui::Button("Home")) {
        UpdateCurrentPath(m_homeDir);
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Desktop")) {
        if (fs::exists(m_desktopDir)) {
            UpdateCurrentPath(m_desktopDir);
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Documents")) {
        if (fs::exists(m_documentsDir)) {
            UpdateCurrentPath(m_documentsDir);
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Refresh")) {
        RefreshFileList();
    }
    
    // Show/Hide hidden files
    ImGui::SameLine();
    if (ImGui::Checkbox("Show Hidden", &m_showHidden)) {
        RefreshFileList();
    }
}

void ImGuiFileBrowser::RenderFileList()
{
    // File list headers
    ImGui::Columns(4, "FileColumns", true);
    ImGui::SetColumnWidth(0, 250);  // Name
    ImGui::SetColumnWidth(1, 80);   // Type
    ImGui::SetColumnWidth(2, 80);   // Size
    ImGui::SetColumnWidth(3, 150);  // Modified
    
    ImGui::Separator();
    ImGui::Text("Name"); ImGui::NextColumn();
    ImGui::Text("Type"); ImGui::NextColumn();
    ImGui::Text("Size"); ImGui::NextColumn();
    ImGui::Text("Modified"); ImGui::NextColumn();
    ImGui::Separator();
    
    // Files and directories
    for (const auto& item : m_filteredFiles) {
        bool isSelected = m_selectedFilename == item.name;
        
        if (ImGui::Selectable(item.name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
            if (item.name == "..") {
                NavigateToParentDirectory();
            } else if (item.isDirectory) {
                UpdateCurrentPath(item.fullPath);
            } else {
                m_selectedFilename = item.name;
                m_selectedFullPath = item.fullPath;
                m_inputFilename = item.name;
                strncpy(m_inputFilenameBuf, item.name.c_str(), sizeof(m_inputFilenameBuf) - 1);
                m_inputFilenameBuf[sizeof(m_inputFilenameBuf) - 1] = '\0';
            }
        }
        
        // Handle double click
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            if (item.name == "..") {
                NavigateToParentDirectory();
            } else if (item.isDirectory) {
                UpdateCurrentPath(item.fullPath);
            } else {
                // Select file and confirm dialog
                m_selectedFilename = item.name;
                m_selectedFullPath = item.fullPath;
                m_inputFilename = item.name;
                strncpy(m_inputFilenameBuf, item.name.c_str(), sizeof(m_inputFilenameBuf) - 1);
                m_inputFilenameBuf[sizeof(m_inputFilenameBuf) - 1] = '\0';
                m_result = FileDialogResult::OK;
            }
        }
        
        ImGui::NextColumn();
        
        // Type column
        if (item.isDirectory) {
            ImGui::Text("Directory");
        } else {
            // Extract extension
            size_t dot = item.name.find_last_of('.');
            if (dot != std::string::npos) {
                std::string ext = item.name.substr(dot);
                ImGui::Text("%s", ext.c_str());
            } else {
                ImGui::Text("File");
            }
        }
        ImGui::NextColumn();
        
        // Size column
        if (item.isDirectory) {
            ImGui::Text(""); // No size for directories
        } else {
            if (item.size < 1024) {
                ImGui::Text("%lu B", item.size);
            } else if (item.size < 1024 * 1024) {
                ImGui::Text("%.1f KB", static_cast<float>(item.size) / 1024.0f);
            } else {
                ImGui::Text("%.1f MB", static_cast<float>(item.size) / (1024.0f * 1024.0f));
            }
        }
        ImGui::NextColumn();
        
        // Modified date column
        ImGui::Text("%s", item.lastModified.c_str());
        ImGui::NextColumn();
    }
    
    ImGui::Columns(1);
    ImGui::Separator();
}

void ImGuiFileBrowser::RenderControls()
{
    // File input for Save dialog
    if (m_type == FileDialogType::Save) {
        ImGui::Text("File name:");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 120);
        
        // If input changed, update the string
        if (ImGui::InputText("##filename", m_inputFilenameBuf, sizeof(m_inputFilenameBuf))) {
            m_inputFilename = m_inputFilenameBuf;
        }
        
        ImGui::PopItemWidth();
        
        // Add extension if needed
        std::string fullPath = m_currentPath + "/" + m_inputFilename;
        if (!m_inputFilename.empty() && !m_fileExtension.empty()) {
            size_t dot = m_inputFilename.find_last_of('.');
            if (dot == std::string::npos) {
                // No extension, add the default one
                fullPath += m_fileExtension;
            }
        }
        m_selectedFullPath = fullPath;
    } else {
        // Selected file for Open dialog
        ImGui::Text("Selected: %s", m_selectedFilename.c_str());
    }
    
    // Filter display
    ImGui::Text("Filter: %s (%s)", m_fileTypeDesc.c_str(), m_fileTypeEndings.c_str());
    
    // Buttons
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);
    ImGui::Separator();
    
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        m_result = FileDialogResult::Cancel;
    }
    
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 130);
    
    // OK button text and behavior based on dialog type
    std::string okText = (m_type == FileDialogType::Open) ? "Open" : "Save";
    bool okEnabled = true;
    
    if (m_type == FileDialogType::Open) {
        okEnabled = !m_selectedFilename.empty() && m_selectedFilename != "..";
    } else {
        okEnabled = !m_inputFilename.empty();
    }
    
    if (!okEnabled) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    
    if (ImGui::Button(okText.c_str(), ImVec2(120, 0)) && okEnabled) {
        if (m_type == FileDialogType::Save) {
            // For Save dialog, use the input filename
            std::string fullPath = m_currentPath + "/" + m_inputFilename;
            
            // Add extension if needed
            if (!m_fileExtension.empty()) {
                size_t dot = m_inputFilename.find_last_of('.');
                if (dot == std::string::npos) {
                    // No extension, add the default one
                    fullPath += m_fileExtension;
                }
            }
            
            m_selectedFullPath = fullPath;
        }
        
        m_result = FileDialogResult::OK;
    }
    
    if (!okEnabled) {
        ImGui::PopStyleVar();
    }
} 