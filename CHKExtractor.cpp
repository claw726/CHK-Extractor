#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#include <sstream>
#include <chrono>
#include <ctime>
#include <direct.h>

#include "fileTypes.h"

//Logger file name
std::string logFileName = "CHKExtractor.log";

namespace fs = std::filesystem;



// Function to display an informational message box
void showMessage(const std::string& message)
{
    MessageBoxA(nullptr, message.c_str(), "Information", MB_OK | MB_ICONINFORMATION);
}

//Function to display an error message box
void showError(const std::string& message)
{
    MessageBoxA(nullptr, message.c_str(), "Error", MB_ICONERROR | MB_OK);
}



Logger logger(logFileName); // logger to be used by whole program

//Takes a std::string and converts it to a wstring
std::wstring ConvertToWideString(const std::string& source)
{
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, nullptr, 0);
    if (bufferSize == 0)
    {
        logger.Log(LogLevel::Error, "Failed to get buffer size to convert " + source + "to a wstring");
        return L"";
    }

    std::vector<wchar_t> buffer(bufferSize);
    if (MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, buffer.data(), bufferSize) == 0)
    {
        logger.Log(LogLevel::Error, "Failed to convert " + source + " to a wstring.");
        return L"";
    }

    return std::wstring(buffer.data());
}

//Opens a file and parses it. Returns false if it finds a single byte that is not zero.
bool isFileAllZeros(fs::directory_entry fileName)
{
    std::ostringstream logMessage;

    constexpr size_t bufferSize = static_cast<size_t>(128) * 1024; //128kb buffer
    std::vector<char> buffer(bufferSize);

    std::ifstream file(fileName.path().string(), std::ios::binary);

    if (!file)
    {
        logger.Log(LogLevel::Error, "Failed to open file: " + fileName.path().string());
        showError("Failed to open file: " + fileName.path().string());
        return true;
    }

    logger.Log(LogLevel::Info, "Checking if there is data in file " + fileName.path().filename().string());

    try
    {
        while (file.read(buffer.data(), bufferSize))
        {
            if (!std::all_of(buffer.begin(), buffer.end(), [](byte byte) { return byte == 0; }))
            {
                logMessage << "Data found in file " + fileName.path().filename().string();
                logger.Log(LogLevel::Info, logMessage.str());
                logMessage.str("");
                file.close();
                return false;
            }
        }

        if (file.bad())
        {
            logMessage << "Exception occured while reading the file: " << fileName.path().filename().string();
            logger.Log(LogLevel::Error, logMessage.str());
            logMessage.str("");
            showError("Exception Occured while reading the file: " + fileName.path().filename().string());
        }
    }
    catch (const std::exception& ex)
    {
        logMessage << "Exception occured while reading the file: " << fileName.path().filename().string() <<". Error code: " << ex.what();
        logger.Log(LogLevel::Error, logMessage.str());
        logMessage.str("");
        showError("Exception occured while reading the file: " + fileName.path().string() + "\n" + ex.what());
    }

    logMessage << "File " << fileName.path().filename().string() << " did not contain any data.";
    logger.Log(LogLevel::Warning, logMessage.str());
    logMessage.str("");
    file.close();
    return true;
}

//Given a filename, this function removes the extension, or the part after the filename
std::string removeFileExtension(const std::string& fileName)
{
    // Find the position of the last dot (.)
    size_t lastDotPos = fileName.find_last_of(".");

    if (lastDotPos != std::string::npos)
    {
        // Return the substring before the last dot
        return fileName.substr(0, lastDotPos);
    }

    // If no dot found, return the original file name
    return fileName;
}

//creates an output directory in the location of the CHK files
void createOutputDirectory()
{
    // Create output directory if it doesn't exist
    if (CreateDirectoryA("exported_CHK_files", nullptr) == 0) {
        //Check for Errors
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS) {
            std::ostringstream errorMessage;
            errorMessage << "Failed to Create Output Directory. Error Code: " << error;
            logger.Log(LogLevel::Error, errorMessage.str());
            showError(errorMessage.str());
        }
    }
}

// Parses a given .CHK file, looking for prefixes for JPEG, MOV, and HEIC files, and splits them up into the output directory
void extractFiles(fs::directory_entry chkFileName)
{
    std::string outputDirectory = "exported_CHK_files";
    std::ostringstream logMessage;

    // Open input file
    std::ifstream inputFile(chkFileName.path().string(), std::ios::binary);
    if (!inputFile)
    {
        std::ostringstream errorMessage;
        errorMessage << "Failed to open input file: " << chkFileName.path().string();
        logger.Log(LogLevel::Error, errorMessage.str());
        showError(errorMessage.str());
        return;
    }

    std::ofstream outputFile;

    // Buffers
    constexpr size_t bufferSize = 4 * 1024; // 8KB buffer
    std::vector<uint8_t> buffer(bufferSize);

    while (inputFile.read(reinterpret_cast<char*>(buffer.data()), bufferSize))
    {
        const size_t bytesRead = inputFile.gcount();

        // Pointer to the first byte of the buffer
        const uint8_t* bufferPtr = buffer.data();

        // Remaining bytes to be processed
        size_t remainingBytes = bytesRead;

        while (remainingBytes >= 4)
        {
            bool headerMatched = false;

            for (const FileHeader& header : headers)
            {
                if (memcmp(bufferPtr, header.header, header.size) == 0)
                {
                    std::ostringstream oss;
                    oss << outputDirectory << "\\" << removeFileExtension(chkFileName.path().filename().string()) << "_" << fileCount << header.extension;
                    std::string fileName = oss.str();

                    if (outputFile.is_open())
                    {
                        outputFile.close();
                    }
                    outputFile.open(fileName, std::ios::binary);
                    if (!outputFile)
                    {
                        std::ostringstream errorMessage;
                        errorMessage << "Failed to create output file: " << fileName;
                        logger.Log(LogLevel::Error, errorMessage.str());
                        showError(errorMessage.str());
                        break;
                    }

                    logMessage << "Extracted " << header.extension << " file in " << chkFileName.path().filename().string() << " at kilobyte " << (inputFile.tellg() / 1024) - 4 << " to " << fileName;
                    logger.Log(LogLevel::Info, logMessage.str());
                    logMessage.str("");

                    fileCount++;
                    }
                }
            if (headerMatched)
                break;

            bufferPtr += 4;
            remainingBytes -= 4;
        }

        // Write the buffer to the file
        if (outputFile.is_open())
            outputFile.write(reinterpret_cast<const char*>(buffer.data()), bytesRead);
    }

    // Close the output file
    if (outputFile.is_open())
        outputFile.close();
}

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    std::ostringstream logMessage;
    logger.Log(LogLevel::Info, "Application started");

    std::ostringstream message;
    message << "This Program only extracts the file types specified on the GitHub page and is not guarenteed to restore all of the files.";
    showMessage(message.str());


    //displays GUI to get .CHK file directory from user
    char directoryPath[MAX_PATH];
    BROWSEINFOA browseInfo = { 0 };
    browseInfo.hwndOwner = nullptr;
    browseInfo.pszDisplayName = directoryPath;
    browseInfo.lpszTitle = "Select Directory";
    browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
    LPITEMIDLIST directoryItemIdList = SHBrowseForFolderA(&browseInfo);
    if (directoryItemIdList == nullptr)
    {
        showError("No directory selected. Exiting program.");
        logger.Log(LogLevel::Error, "No directory selected. Exiting program.");
        return EXIT_FAILURE;
    }

    if (!SHGetPathFromIDListA(directoryItemIdList, directoryPath))
    {
        logger.Log(LogLevel::Error, "Failed to get the directory path. Exiting program.");
        showError("Failed to get the directory path. Exiting program.");
        return EXIT_FAILURE;
    }

    CoTaskMemFree(directoryItemIdList);

    //Convert the input directory to wchar_t and set cwd
    std::wstring directoryStr;
    try {
        directoryStr = ConvertToWideString(directoryPath);
        if (!SetCurrentDirectory(directoryStr.c_str())) {
            std::ostringstream errorMessage;
            errorMessage << "Error Opening Directory: " << ConvertToWideString(directoryPath).c_str();
            showError(errorMessage.str());
            return EXIT_FAILURE;
        }
        
    }
    catch (const std::exception& ex) {
        std::ostringstream errorMessage;
        errorMessage << "Error: " << ex.what() << std::endl;
        showError(errorMessage.str());
        return EXIT_FAILURE;
    }
    logMessage << "Input directory set to: " << directoryPath;
    logger.Log(LogLevel::Info, logMessage.str());
    logMessage.str("");
    createOutputDirectory();

    showMessage("Program is ready to run. It may take some time to extract all of the files.");

    FileParser parser;

    //Iterate over each file in directoryStr
    for (fs::directory_entry entry : fs::directory_iterator(directoryStr))
    {
        //Check if file is .CHK file
        if (entry.is_regular_file() && entry.path().extension() == ".CHK")
        {
            logMessage << "Found file: " << entry.path().filename().string();
            logger.Log(LogLevel::Info, logMessage.str());
            logMessage.str("");
            try {
                //extract files from chk file
                parser.parseCHKFiles(entry);
            }
            catch (const std::exception& ex)
            {
                std::ostringstream errorMessage;
                logMessage << "Exception occured while processing file: " << entry.path().string() << " with code " << ex.what();
                logger.Log(LogLevel::Error, logMessage.str());
                logMessage.str("");
                errorMessage << "Exception occured while processing file: " << entry.path().string() << "\n" << ex.what();
                showError(errorMessage.str());
            }
            
        }
    }

    showMessage("Files have been extracted from the .CHK files.\nClick \"OK\" to view the statistics of extracted files.\nExtracted files are in the folder \"extracted_CHK_files\" in the same folder the CHK files were in.");

    parser.printStatistics(hInstance, nCmdShow);
    logger.Log(LogLevel::Info, "Program successfully completed.");
    return EXIT_SUCCESS;
}
