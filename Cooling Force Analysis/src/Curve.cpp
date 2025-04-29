#include "pch.h"
#include "Curve.h"
#include "FileUtils.h"
#include "Constants.h"

void Curve::ShowJumpList()
{
	ImGuiChildFlags flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
	if (ImGui::BeginChild("cc tabs", ImVec2(400.0f, -1), flags))
	{
		ImGui::Text(folder.string().c_str());
		if (ImGui::BeginListBox("##jump list", ImVec2(-1, 500)))
		{
			int i = 0;
			for (const PhaseJump& jump : jumps)
			{
				ImGui::PushID(i);
				if (jump.ShowAsListItem(selectedIndex == i))
				{
					selectedIndex = i;
					SelectedItemChanged();
				}
				i++;
				ImGui::PopID();
			}

			ImGui::EndListBox();
		}
	}
	ImGui::EndChild();
}

void Curve::AddPhaseJump(PhaseJump& jump)
{
	jumpValues.push_back(jump.phaseJumpValue);
	jumpValueErrors.push_back(jump.phaseJumpValueError);

	// will call move Constructor
	jumps.emplace_back(std::move(jump));

	if (jumps.size() == 1)
	{
		selectedIndex = 0;
		SelectedItemChanged();
	}
}

void Curve::Plot() const
{
	ImPlot::PlotScatter(name.c_str(), detuningVelocities.data(), coolingForceValues.data(), detuningVelocities.size());
	ImPlot::PlotErrorBars(name.c_str(), detuningVelocities.data(), coolingForceValues.data(), coolingForceErrors.data(), detuningVelocities.size());
}

void Curve::PlotSelectedJump() const
{
	if (selectedIndex >= 0)
	{
		jumps.at(selectedIndex).Plot();
	}
}

void Curve::LoadPhaseJumpFolder(std::filesystem::path inputFolder)
{
	if (std::filesystem::is_directory(inputFolder))
	{
		std::vector<std::filesystem::path> files;
		for (const auto& entry : std::filesystem::directory_iterator(inputFolder))
		{
			if (entry.is_regular_file() && (entry.path().extension() == ".CSV" || entry.path().extension() == ".csv"))
			{
				files.push_back(entry.path());
			}
		}

		// Sort file paths
		std::sort(files.begin(), files.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
			return FileUtils::GetNumberFromFilename(a.filename().string()) < FileUtils::GetNumberFromFilename(b.filename().string());
			});

		for (const auto& file : files)
		{
			PhaseJump jump;
			jump.LoadFromFile(file);
			AddPhaseJump(std::move(jump));
		}
		
		folder = inputFolder.parent_path().parent_path().filename() / inputFolder.parent_path().filename();
	}
}

void Curve::SelectedItemChanged()
{

}

double CalculateDetuningVelocity(double coolingEenrgy, double labEnergy)
{
	return sqrt(2 / PhysicalConstants::electronMass) * (sqrt(coolingEenrgy * TMath::Qe()) - sqrt(labEnergy * TMath::Qe()));
}

double CalculateCoolingForce(double phaseJump, double effectiveBunchingVoltage, int ionCharge)
{
	return ionCharge * effectiveBunchingVoltage * sin(phaseJump * TMath::Pi() / 180) / CSR::coolerLength;
}
