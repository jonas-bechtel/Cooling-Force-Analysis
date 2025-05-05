#include "pch.h"
#include "PhaseJump.h"
#include "FileUtils.h"
#include "HelpFunctions.h"
#include "Curve.h"

JumpEvaluationParameter PhaseJump::params;
double JumpEvaluationParameter::timePointPassedJump = 0;

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
	if (ImGui::Checkbox("different jump time", &useIndividualJumpTime))
	{
		UpdatePointPastJump();
	}
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
	//std::cout << "phase before: " << phaseBefore << std::endl;
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

void PhaseJump::UpdatePointPastJump()
{
	if(useIndividualJumpTime)
		params.timePointPassedJump = individualJumpTime + params.timePassedJump;
	else
		params.timePointPassedJump = params.JumpTime + params.timePassedJump;
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
		ImVec4 colorPast = ImVec4(1, 0.3, 0.1, 1.0);
		if (useIndividualJumpTime)
		{
			if (ImPlot::DragLineX(0, &individualJumpTime, color, 1, ImPlotDragToolFlags_Delayed))
			{
				ClampJumpTimeToAllowedRange();
				UpdatePointPastJump();
				CalculateTemporaryJumpValues();
			}
			if (ImPlot::DragLineX(1, &params.timePointPassedJump, colorPast, 1, ImPlotDragToolFlags_Delayed))
			{
				params.timePointPassedJump = std::clamp(params.timePointPassedJump, individualJumpTime, time.back() - time.at(std::floor(params.movingAverageWindowSize / 2)));
				params.timePassedJump = params.timePointPassedJump - individualJumpTime;
				ClampJumpTimeToAllowedRange();
				CalculateTemporaryJumpValues();
			}
			ImPlot::TagX(individualJumpTime, color, "jump");
		}
		else
		{
			if (ImPlot::DragLineX(2, &params.JumpTime, color, 1, ImPlotDragToolFlags_Delayed))
			{
				ClampJumpTimeToAllowedRange();
				UpdatePointPastJump();
				curve->RecalculateAllTemporaryJumpValues();
			}
			if (ImPlot::DragLineX(3, &params.timePointPassedJump, colorPast, 1, ImPlotDragToolFlags_Delayed))
			{
				params.timePointPassedJump = std::clamp(params.timePointPassedJump, params.JumpTime, time.back() - time.at(std::floor(params.movingAverageWindowSize / 2)));
				params.timePassedJump = params.timePointPassedJump - params.JumpTime;
				ClampJumpTimeToAllowedRange();
				curve->RecalculateAllTemporaryJumpValues();
			}
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
	ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.7, 0.1, 0.1, 1.0));
	ImPlot::PlotLine("##moving average", time.data() + offset, movingAveragePhase.data(), movingAveragePhase.size());
	ImPlot::PopStyleColor();
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
