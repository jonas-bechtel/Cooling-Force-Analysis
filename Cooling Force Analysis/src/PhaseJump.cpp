#include "pch.h"
#include "PhaseJump.h"
#include "FileUtils.h"
#include "HelpFunctions.h"
#include "Curve.h"

JumpEvaluationParameter PhaseJump::params;

bool PhaseJump::ShowAsListItem(bool selected)
{
	bool clicked = false;
	if (ImGui::Selectable(filename.c_str(), selected, ImGuiSelectableFlags_AllowItemOverlap))
	{
		clicked = true;
	}
	if (useIndividualJumpTime)
	{
		ImGui::SameLine();
		ImGui::Text("different jump time");
	}
		

	return clicked;
}

void PhaseJump::ShowParameterInputs()
{
	ImGui::Checkbox("different jump time", &useIndividualJumpTime);
	if (useIndividualJumpTime)
	{
		ImGui::InputDouble("jump time", &individualJumpTime);
	}
	else
	{
		ImGui::InputDouble("jump time", &params.JumpTime);
	}
	

}

void PhaseJump::CalculateMovingAverage()
{
	movingAveragePhase = MovingAverage(phase, params.movingAverageWindowSize);
}

void PhaseJump::CalculateTemporaryJumpValues()
{
	double usedJumpTime = params.JumpTime;
	if (useIndividualJumpTime)
	{
		usedJumpTime = individualJumpTime;
	}
	// there is a difference in the index between the main data and the averaged data that needs to be compensated for
	int indexOffset = std::floor(params.movingAverageWindowSize / 2.0);
	int index = TimeToIndex(time, usedJumpTime) - indexOffset;
	double phaseBefore = movingAveragePhase.at(index);
	std::cout << "phase before: " << phaseBefore << std::endl;
	double phaseAfter = movingAveragePhase.at(TimeToIndex(time, usedJumpTime + params.timePassedJump) - indexOffset);
	temporaryJumpValue = phaseAfter - phaseBefore;
}

void PhaseJump::AddTempValueToList()
{
	jumpValueList.push_back(temporaryJumpValue);
	
	phaseJumpValue = CalculateMean(jumpValueList);
	phaseJumpValueError = CalculateStdDev(jumpValueList);

}

void PhaseJump::ClearValueList()
{
	jumpValueList.clear();
	phaseJumpValue = 0;
	phaseJumpValueError = 0;
}

void PhaseJump::ClampJumpTimeToAllowedRange()
{
	// keep line within range of moving average data
	double lowerLimit = time.at(std::floor(params.movingAverageWindowSize / 2));
	double upperLimit = time.back() - time.at(std::floor(params.movingAverageWindowSize / 2)) - params.timePassedJump;
	params.JumpTime = std::clamp(params.JumpTime, lowerLimit, upperLimit);
	individualJumpTime = std::clamp(individualJumpTime, lowerLimit, upperLimit);
}

void PhaseJump::Plot()
{
	ImPlot::SetupAxes("Time [s]", "Phase [deg]");
	ImPlot::PushStyleColor(ImPlotCol_MarkerFill, ImPlot::GetColormapColor(0));
	ImPlot::PushStyleColor(ImPlotCol_MarkerOutline, ImPlot::GetColormapColor(0));
	ImPlot::PlotScatter(filename.c_str(), time.data(), phase.data(), time.size());
	ImPlot::PopStyleColor();
	ImPlot::PopStyleColor();

	if (params.showJumpLine)
	{
		ImVec4 color = ImVec4(1, 0, 0, 1);
		if (useIndividualJumpTime)
		{
			if (ImPlot::DragLineX(0, &individualJumpTime, color, 1, ImPlotDragToolFlags_Delayed))
			{
				ClampJumpTimeToAllowedRange();
				CalculateTemporaryJumpValues();
			}
			double timePointPassedJump = individualJumpTime + params.timePassedJump;
			ImPlot::PlotInfLines("##time passed jump", &timePointPassedJump, 1);
			ImPlot::TagX(individualJumpTime, color, "jump");
		}
		else
		{
			if (ImPlot::DragLineX(0, &params.JumpTime, color, 1, ImPlotDragToolFlags_Delayed))
			{
				ClampJumpTimeToAllowedRange();
				curve->RecalculateAllTemporaryJumpValues();
			}
			double timePointPassedJump = params.JumpTime + params.timePassedJump;
			ImPlot::PlotInfLines("##time passed jump", &timePointPassedJump, 1);
			ImPlot::TagX(params.JumpTime, color, "jump");
		}
	}
}

void PhaseJump::PlotMovingAverage() const
{
	if (!params.plotMovingAverage)
		return;

	int offset = std::floor(PhaseJump::params.movingAverageWindowSize / 2);
	ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 5);
	ImPlot::PlotLine("##moving average", time.data() + offset, movingAveragePhase.data(), movingAveragePhase.size());
	ImPlot::PopStyleVar();
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

		CalculateMovingAverage();
	}
}
