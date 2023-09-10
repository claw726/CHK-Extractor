#ifndef FILETYPES_H
#define FILETYPES_H

#include <string>

// Variables to hold the counts
int fileCount = 0; // Number of files extracted
int emptyCount = 0; //Number of CHK files with no data

/**
 * Enums
 */

 // Logging levels
enum class LogLevel {
    Info,
    Warning,
    Error
};

// IDs for the controls
enum {
    // Control IDs for count displays
    IDC_TOTAL_COUNT = 1001,
    IDC_BAD_COUNT,

    // Control IDs for labels

    IDC_TOTAL_COUNT_LABEL = 2001,
    IDC_BAD_COUNT_LABEL,
};

namespace fs = std::filesystem;

// Window Class and Title
const wchar_t* windowClassName = L"MyWindowClass";
const wchar_t* windowTitle = L"File Extraction Report";

/**
 * @brief Displays an informational message box.
 * @param message The message to be displayed.
 */
void showMessage(const std::string& message);

/**
 * @brief Displays an error message box.
 * @param message The error message to be displayed.
 */
void showError(const std::string& message);

/**
 * @brief Removes the file extension from a given filename.
 * @param fileName The filename from which to remove the extension.
 * @return The filename without the extension.
 */
std::string removeFileExtension(const std::string& fileName);

bool isFileAllZeros(fs::directory_entry);

void extractFiles(fs::directory_entry);

// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        //Create the Labels
        CreateWindow(L"STATIC", L"Total Count:", WS_CHILD | WS_VISIBLE, 10, 10, 250, 20, hwnd, (HMENU)IDC_TOTAL_COUNT_LABEL, nullptr, nullptr);
        CreateWindow(L"STATIC", L"Bad Count:", WS_CHILD | WS_VISIBLE, 10, 40, 250, 20, hwnd, (HMENU)IDC_BAD_COUNT_LABEL, nullptr, nullptr);

        // Create the count displays
        CreateWindow(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 270, 10, 50, 20, hwnd, (HMENU)IDC_TOTAL_COUNT, nullptr, nullptr);
        CreateWindow(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 270, 40, 50, 20, hwnd, (HMENU)IDC_BAD_COUNT, nullptr, nullptr);

        // Set the initial count values

        SetWindowText(GetDlgItem(hwnd, IDC_TOTAL_COUNT), std::to_wstring(fileCount).c_str());
        SetWindowText(GetDlgItem(hwnd, IDC_BAD_COUNT), std::to_wstring(emptyCount).c_str());

        //Set Labels

        SetWindowText(GetDlgItem(hwnd, IDC_TOTAL_COUNT_LABEL), L"Total files extracted: ");
        SetWindowText(GetDlgItem(hwnd, IDC_BAD_COUNT_LABEL), L"Number of empty CHK Files: ");
    }
    break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}



/**
 *   Classes
 */

//File Parser Class
class FileParser {
public:
    //Parses the passed .CHK files
    void parseCHKFiles(const fs::directory_entry directory)
    {
        if (!isFileAllZeros(directory))
            extractFiles(directory);
        else
            emptyCount++;
    }

    // Prints how many of each file was extracted
    void printStatistics(HINSTANCE hInstance, int nCmdShow) const {
        // Register the window class
        WNDCLASS wc = { };
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = windowClassName;

        RegisterClass(&wc);

        // Create the main window
        HWND hwnd = CreateWindowEx(0, windowClassName, windowTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 400, 200, nullptr, nullptr, hInstance, nullptr);

        if (hwnd == nullptr)
        {
            return;
        }

        // Show the window
        ShowWindow(hwnd, nCmdShow);

        // Message loop
        MSG msg = { };
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


};

//Logger Class
class Logger {
public:
    Logger(const std::string& logFilePath)
        : logFileStream(logFilePath, std::ios::app)
    {
        if (!logFileStream)
        {
            showError("Failed to open log file: " + logFilePath);
        }
    }

    void Log(LogLevel level, const std::string& message)
    {
        std::string levelStr;
        switch (level)
        {
        case LogLevel::Info:
            levelStr = "INFO";
            break;
        case LogLevel::Warning:
            levelStr = "WARNING";
            break;
        case LogLevel::Error:
            levelStr = "ERROR";
            break;
        }

        //Get current time
        std::time_t now;
        std::time(&now);
        char timeStr[26];
        ctime_s(timeStr, sizeof(timeStr), &now);
        timeStr[24] = '\0'; //Remove trailing newline character

        //Write log to file
        if (logFileStream)
        {
            logFileStream << "[" << timeStr << "] [" << levelStr << "] " << message << std::endl;
        }

        //Output log entry to console
        std::cout << "[" << timeStr << "] [" << levelStr << "] " << message << std::endl;
    }
private:
    std::ofstream logFileStream;
};



static const uint8_t jfif[4] = { 0xff, 0xd8, 0xff, 0xe0 };
static const uint8_t jpeg[4] = { 0xff, 0xd8, 0xff, 0xe1 };
static const uint8_t mov0[12] = { 0x00, 0x00, 0x00, 0x14, 0x66, 0x74, 0x79, 0x70, 0x71, 0x74, 0x20, 0x20 };
static const uint8_t mov1[12] = { 0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70, 0x69, 0x73, 0x6f, 0x6d };
static const uint8_t heic[12] = { 0x00, 0x00, 0x00, 0x28, 0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x63 };
static const uint8_t mp4[12] = { 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70, 0x6D, 0x70, 0x34, 0x32 };
static const uint8_t png[8] = { 0x89, 0x50, 0x48, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
static const uint8_t gif89[6] = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 };
static const uint8_t gif87[6] = { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 };
static const uint8_t pdf[4] = { 0x25, 0x50, 0x44, 0x46 };
static const uint8_t zip[4] = { 0x50, 0x4B, 0x03, 0x04 };
static const uint8_t mkv[4] = { 0x1A, 0x45, 0xDF, 0xA3 };
static const uint8_t webp[4] = { 0x52, 0x49, 0x46, 0x46 };
static const uint8_t p3g[11] = { 0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70, 0x33, 0x67, 0x70 };
static const uint8_t wav[4] = { 0x52, 0x49, 0x46, 0x46 };

struct FileHeader
{
    const uint8_t* header; // Hex sequence of header
    size_t size; // size of the hex sequence in bytes
    const char* extension; //extension of the file (.jpg, .gif, .pdf, etc.)
};

extern const std::vector<FileHeader> headers = {
        {jfif, sizeof(jfif), ".jfif"},
        {jpeg, sizeof(jpeg), ".jpeg"},
        {mov0, sizeof(mov0), ".mov"},
        {mov1, sizeof(mov1), ".mov"},
        {heic, sizeof(heic), ".heic"},
        {mp4,  sizeof(mp4),  ".mp4"},
        {png,  sizeof(png),  ".png"},
        {gif89,sizeof(gif89),".gif"},
        {gif87,sizeof(gif87),".gif"},
        {pdf,  sizeof(pdf),  ".pdf"},
        {zip,  sizeof(zip),  ".zip"},
        {mkv,  sizeof(mkv),  ".mkv"},
        {webp, sizeof(webp), ".webp"},
        {p3g,  sizeof(p3g),  ".3gp"},
        {wav,  sizeof(wav),  ".wav"}
};

#endif // !fileTypes.h
