#include "pch.h"
#include "PhaseJump.h"
#include "FileUtils.h"

bool PhaseJump::ShowAsListItem(bool selected) const
{
	bool clicked = false;
	if (ImGui::Selectable(filename.c_str(), selected, ImGuiSelectableFlags_AllowItemOverlap))
	{
		clicked = true;
	}

	return clicked;
}

void PhaseJump::Plot() const
{
	ImPlot::SetupAxes("Time [s]", "Phase[deg]");
	ImPlot::PlotScatter(filename.c_str(), time.data(), phase.data(), time.size());
}

void PhaseJump::LoadFromFile(std::filesystem::path inputfile)
{
	std::ifstream file;
	file.open(inputfile, std::ios::in);

	FileUtils::GetHeaderFromFile(file);
	time.reserve(20000);
	radius.reserve(20000);
	phase.reserve(20000);

	std::string line;
	if (file.is_open())
	{
		while (std::getline(file, line))
		{
			std::vector<std::string> tokens = FileUtils::SplitLine(line, ",");

			time.push_back(std::stod(tokens[0]));
			radius.push_back(std::stod(tokens[1]));
			phase.push_back(std::stod(tokens[2]));
		}
		time.shrink_to_fit();
		radius.shrink_to_fit();
		phase.shrink_to_fit();

		file.close();

		filename = inputfile.filename().string();
	}
}
