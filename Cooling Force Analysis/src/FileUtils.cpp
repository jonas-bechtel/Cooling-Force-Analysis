#include "pch.h"
#include "FileUtils.h"

#include "tinyfiledialogs.h"

namespace FileUtils
{
    static std::filesystem::path dataFolder = "data\\";
    static std::filesystem::path outputFolder = "output\\";
    static std::filesystem::path curveFolder = outputFolder / "Cooling Force Curves\\";

    std::filesystem::path GetDataFolder()
    {
        return dataFolder;
    }
    std::filesystem::path GetOutputFolder()
    {
        return outputFolder;
    }
    std::filesystem::path GetCoolingForceCurveFolder()
    {
        return curveFolder;
    }

    std::filesystem::path SelectFile(const std::filesystem::path& startPath, const std::vector<const char*>& filterPatterns)
    {
        //const char* filterPatterns[] = { "*.asc" };
        const char* filePath = tinyfd_openFileDialog(
            "Choose a file",               // Dialog title
            startPath.string().c_str(),    // Default path or file
            1,                             // Number of filters
            filterPatterns.data(),                // Filter patterns (NULL for any file type)
            NULL,                          // Filter description (optional)
            0                              // Allow multiple selection (0 = false)
        );
        if (!filePath)
        {
            return std::filesystem::path();
        }
        return std::filesystem::path(filePath);
    }

    std::filesystem::path SelectFolder(const std::filesystem::path& startPath)
    {
        std::filesystem::path projectDir = std::filesystem::current_path();
        std::filesystem::path fullStartPath = projectDir / startPath;

        const char* folder = tinyfd_selectFolderDialog("Choose a folder", fullStartPath.string().c_str());

        if (!folder)
        {
            return std::filesystem::path();
        }
        return std::filesystem::path(folder);
    }

    std::string GetHeaderFromFile(std::ifstream& file)
    {
        std::string line;
        std::string header = "";

        while (std::getline(file, line))
        {
            header += line + "\n";

            if (!(file.peek() == '#'))
                break;
        }

        return header;
    }

    std::vector<std::string> SplitLine(std::string& string, const std::string& delimiter)
    {
        std::vector<std::string> tokens;

        size_t pos = 0;
        std::string token;

        while ((pos = string.find(delimiter)) != std::string::npos)
        {
            token = string.substr(0, pos);
            tokens.push_back(token);
            string.erase(0, pos + delimiter.length());
        }
        tokens.push_back(string);

        return tokens;
    }
    
    int GetNumberFromFilename(const std::string& filename)
    {
        std::regex re(".*_(\\d+)\\.csv$"); // Regular expression to match underscore followed by digits
        std::smatch match;  
        if (std::regex_search(filename, match, re) && match.size() > 1) {
            return std::stoi(match.str(1));  // Convert matched number to integer
        }
        return -1;  // Return -1 if no pattern match (to handle errors logically)
    }
}