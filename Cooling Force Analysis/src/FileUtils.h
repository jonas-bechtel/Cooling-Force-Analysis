#pragma once
namespace FileUtils
{
	std::filesystem::path GetDataFolder();
	std::filesystem::path GetOutputFolder();
	std::filesystem::path GetCoolingForceCurveFolder();
	std::filesystem::path GetParameterMapFile();

	std::filesystem::path SelectFile(const std::filesystem::path& startPath = "data\\", const std::vector<const char*>& filterPatterns = { "*.asc" });
	std::filesystem::path SelectFolder(const std::filesystem::path& startPath = "data\\");

	std::string GetHeaderFromFile(std::ifstream& file);
	std::vector<std::string> SplitLine(std::string& line, const std::string& delimiter);
	std::string RemoveLeadingTrailingSpaces(const std::string& str);

	int GetNumberFromFilename(const std::string& filename);

	std::filesystem::path FindFileWithSubstring(const std::filesystem::path& folderPath, const std::string& searchString);
}

