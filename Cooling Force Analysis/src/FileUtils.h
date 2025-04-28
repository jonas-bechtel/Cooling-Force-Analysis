#pragma once
namespace FileUtils
{
	std::filesystem::path GetDataFolder();
	std::filesystem::path GetOutputFolder();
	std::filesystem::path GetCoolingForceCurveFolder();

	std::filesystem::path SelectFile(const std::filesystem::path& startPath = "data\\", const std::vector<const char*>& filterPatterns = { "*.asc" });
	std::filesystem::path SelectFolder(const std::filesystem::path& startPath = "data\\");

	std::string GetHeaderFromFile(std::ifstream& file);
	std::vector<std::string> SplitLine(std::string& line, const std::string& delimiter);
}

