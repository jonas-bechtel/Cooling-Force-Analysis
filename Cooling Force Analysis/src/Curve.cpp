#include "pch.h"
#include "Curve.h"
#include "FileUtils.h"
#include "Constants.h"
#include "HelpFunctions.h"

void Curve::ShowJumpList()
{
	ImGuiChildFlags flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
	if (ImGui::BeginChild("cc tabs", ImVec2(400.0f, -1), flags))
	{
		ImGui::Text("jump data folder: %s", jumpDataFolder.string().c_str());
		ImGui::Text("lab energy file: %s", labEnergyFile.string().c_str());
		if (ImGui::BeginListBox("##jump list", ImVec2(-1, 500)))
		{
			int i = 0;
			for (PhaseJump& jump : jumps)
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

		ImGui::Separator();
		if (ImGui::Button("load phase jumps"))
		{
			std::filesystem::path folder = FileUtils::SelectFolder(FileUtils::GetDataFolder());
			if (!folder.empty())
			{
				LoadPhaseJumpFolder(folder);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("load cool curve"))
		{
			std::filesystem::path file = FileUtils::SelectFile(FileUtils::GetCoolingForceCurveFolder(), {"*.curve"});
			if (!file.empty())
			{
				LoadFromFile(file);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("load lab energies"))
		{
			std::filesystem::path startpath = FileUtils::GetDataFolder() / (jumpDataFolder.string() + "\\");
			std::cout << startpath << std::endl;
			std::filesystem::path file = FileUtils::SelectFile(startpath, {"*.txt"});
			if (!file.empty())
			{
				LoadLabEnergiesFile(file);
			}
		}

		ShowParameterInputs();
	}
	ImGui::EndChild();
}

void Curve::ShowParameterInputs()
{
	bool changed = false;
	ImGui::BeginGroup();
	ImGui::PushItemWidth(100.0f);
	changed |= ImGui::InputInt("ion charge", &ionCharge);
	changed |= ImGui::InputDouble("effective bunching voltage", &effectiveBunchingVoltage, 0, 0, "%.3f");
	changed |= ImGui::InputDouble("cooling energy", &coolingEnergy, 0, 0, "%.4f");
	ImGui::PopItemWidth();
	ImGui::EndGroup();

	if (changed)
	{
		RecalculateAllForcesAndDetungingVels();
	}
}

void Curve::ShowCurrentPhaseJumpParameters()
{
	if (selectedIndex >= 0)
	{
		jumps.at(selectedIndex).ShowParameterInputs();
	}
}

void Curve::SetName(std::string newName)
{
	name = newName;
}

std::string Curve::GetName()
{
	return name;
}

void Curve::RecalculateAllForcesAndDetungingVels()
{
	for (int i = 0; i < jumpValues.size(); i++)
	{
		coolingForceValues.at(i) = CalculateCoolingForce(jumpValues.at(i), effectiveBunchingVoltage, ionCharge);
		coolingForceErrors.at(i) = CalculateCoolingForceError(jumpValues.at(i), jumpValueErrors.at(i), effectiveBunchingVoltage, ionCharge);
	}

	for (int i = 0; i < labEnergies.size(); i++)
	{
		detuningVelocities.at(i) = CalculateDetuningVelocity(coolingEnergy, labEnergies.at(i));
	}
}

void Curve::RecalculateAllMovingAverages()
{
	for (PhaseJump& jump : jumps)
	{
		jump.CalculateMovingAverage();
	}
}

void Curve::RecalculateAllTemporaryJumpValues()
{
	for (PhaseJump& jump : jumps)
	{
		jump.CalculateTemporaryJumpValue();
	}
}

void Curve::AddAllTempJumpValuesToList()
{
	int i = 0;
	for (PhaseJump& jump : jumps)
	{
		jump.AddTempValueToList();
		jumpValues.at(i) = jump.phaseJumpValue;
		jumpValueErrors.at(i) = jump.phaseJumpValueError;
		i++;
	}
	RecalculateAllForcesAndDetungingVels();
}

void Curve::ClearAllValueList()
{
	int i = 0;
	for (PhaseJump& jump : jumps)
	{
		jump.ClearValueList();
		jumpValues.at(i) = 0;
		jumpValueErrors.at(i) = 0;
		i++;
	}
	RecalculateAllForcesAndDetungingVels();
}

void Curve::ClampJumpTimesToAllowedRange()
{
	for (PhaseJump& jump : jumps)
	{
		jump.ClampJumpTimeToAllowedRange();
	}
}

void Curve::UpdatePointPastJump()
{
	jumps.at(selectedIndex).UpdatePointPastJump();
}

void Curve::AddPhaseJump(PhaseJump& jump)
{
	jumpValues.push_back(jump.phaseJumpValue);
	jumpValueErrors.push_back(jump.phaseJumpValueError);

	coolingForceValues.push_back(CalculateCoolingForce(jump.phaseJumpValue, effectiveBunchingVoltage, ionCharge));
	coolingForceErrors.push_back(CalculateCoolingForceError(jump.phaseJumpValue, jump.phaseJumpValueError, effectiveBunchingVoltage, ionCharge));

	jump.curve = this;

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
	ImPlot::PlotErrorBars(name.c_str(), detuningVelocities.data(), coolingForceValues.data(), coolingForceErrors.data(), std::min(detuningVelocities.size(), coolingForceValues.size()));

	if (selectedIndex >= 0 && !detuningVelocities.empty() && !coolingForceValues.empty())
	{
		ImPlot::PushStyleColor(ImPlotCol_MarkerFill, ImVec4(1, 0, 0, 1));
		double selectedDetuningVel = detuningVelocities.at(selectedIndex);
		double selectedForceValue = coolingForceValues.at(selectedIndex);
		ImPlot::PlotScatter("##selected", &selectedDetuningVel, &selectedForceValue, 1);
		ImPlot::PopStyleColor();
	}
	
}

void Curve::PlotSelectedJump()
{
	if (selectedIndex >= 0)
	{
		jumps.at(selectedIndex).Plot();
		jumps.at(selectedIndex).PlotMovingAverage();
	}
}

void Curve::Save()
{
	std::filesystem::path outfolder = FileUtils::GetCoolingForceCurveFolder();

	if (!std::filesystem::exists(outfolder))
	{
		std::filesystem::create_directories(outfolder);
	}

	std::filesystem::path file = outfolder / (name + ".curve");
	std::ofstream outfile(file);

	if (!outfile.is_open())
	{
		std::cerr << "Error opening file" << std::endl;
		return;
	}

	outfile << "# data folder: " << jumpDataFolder.string() << "\n";
	outfile << "# lab energy file: " << labEnergyFile.string() << "\n";
	outfile << "# ion charge: " << ionCharge << "\n";
	outfile << "# cooling energy [eV]: " << coolingEnergy << "\n";
	outfile << "# effective bunching voltage [V]: " << effectiveBunchingVoltage << "\n";
	outfile << "# filename\tdetuning velocity [m/s]\tcooling force value [eV/m]\tcooling force error[eV/m]\tlab energy [eV]\tphase jump value [deg]\tphase jump error [deg]\n";

	for (int i = 0; i < detuningVelocities.size(); i++)
	{
		outfile << jumps.at(i).filename << "\t"
			<< detuningVelocities.at(i) << "\t"
			<< coolingForceValues.at(i) << "\t"
			<< coolingForceErrors.at(i) << "\t"
			<< labEnergies.at(i) << "\t"
			<< jumpValues.at(i) << "\t"
			<< jumpValueErrors.at(i) << "\n";
	}

	outfile.close();
}

void Curve::LoadPhaseJumpFolder(std::filesystem::path inputFolder)
{
	jumps.clear();
	jumpValues.clear();
	jumpValueErrors.clear();
	
	coolingForceValues.clear();
	coolingForceErrors.clear();

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
		
		jumpDataFolder = inputFolder.parent_path().filename() / inputFolder.filename();
		name = inputFolder.parent_path().filename().string() + "_" + inputFolder.filename().string();
	}
	RecalculateAllTemporaryJumpValues();
}

void Curve::LoadLabEnergiesFile(std::filesystem::path inputFile)
{
	detuningVelocities.clear();
	labEnergies.clear();

	std::ifstream file;
	file.open(inputFile, std::ios::in);

	//FileUtils::GetHeaderFromFile(file);

	std::string line;
	if (file.is_open())
	{
		while (std::getline(file, line))
		{
			labEnergies.push_back(std::stod(line));
			detuningVelocities.push_back(CalculateDetuningVelocity(coolingEnergy, std::stod(line)));
		}

		file.close();

		labEnergyFile = inputFile.filename().string();
	}
}

void Curve::LoadFromFile(std::filesystem::path inputFile)
{
	jumps.clear();
	jumpValues.clear();
	jumpValueErrors.clear();

	coolingForceValues.clear();
	coolingForceErrors.clear();

	labEnergies.clear();
	detuningVelocities.clear();

	std::ifstream file;
	file.open(inputFile, std::ios::in);

	if (file.is_open())
	{
		std::string header = FileUtils::GetHeaderFromFile(file);
		std::vector<std::string> tokens = FileUtils::SplitLine(header, "\n");
		jumpDataFolder = std::filesystem::path(FileUtils::RemoveLeadingTrailingSpaces(FileUtils::SplitLine(tokens[0], ":")[1]));
		labEnergyFile = std::filesystem::path(FileUtils::SplitLine(tokens[1], ":")[1]);
		ionCharge = std::stoi(FileUtils::SplitLine(tokens[2], ":")[1]);
		coolingEnergy = std::stod(FileUtils::SplitLine(tokens[3], ":")[1]);
		effectiveBunchingVoltage = std::stod(FileUtils::SplitLine(tokens[4], ":")[1]);

		std::string line;

		while (std::getline(file, line))
		{
			std::vector<std::string> tokens = FileUtils::SplitLine(line, "\t");
			std::string filename = tokens[0];
			double detuningVelocity = std::stod(tokens[1]);
			double coolingForceValue = std::stod(tokens[2]);
			double coolingForceError = std::stod(tokens[3]);
			double labEnergy = std::stod(tokens[4]);
			double jumpValue = std::stod(tokens[5]);
			double jumpValueError = std::stod(tokens[6]);

			PhaseJump jump;
			jump.LoadFromFile(FileUtils::GetDataFolder() / jumpDataFolder / filename);
			AddPhaseJump(jump);

			jumpValues.back() = jumpValue;
			jumpValueErrors.back() = jumpValueError;

			coolingForceValues.back() = coolingForceValue;
			coolingForceErrors.back() = coolingForceError;

			labEnergies.push_back(labEnergy);
			detuningVelocities.push_back(detuningVelocity);
		}

		file.close();

		name = inputFile.filename().replace_extension().string();
	}
}

void Curve::SelectedItemChanged()
{
	UpdatePointPastJump();
}

