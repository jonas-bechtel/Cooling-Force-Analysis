#include "pch.h"
#include "Curve.h"
#include "FileUtils.h"
#include "Constants.h"
#include "HelpFunctions.h"
#include "LabEnergyConversion.h"

YAML::Node Curve::parameterMap;

void printYamlNode(const YAML::Node& node, const std::string& indent = "") {
	if (node.IsMap()) {
		// Iterate over the key-value pairs in the map
		for (const auto& kv : node) {
			std::cout << indent << kv.first.as<std::string>() << ": ";
			printYamlNode(kv.second, indent + "  ");
		}
	}
	else if (node.IsSequence()) {
		// Iterate over the elements in the sequence
		for (size_t i = 0; i < node.size(); ++i) {
			std::cout << indent << "- ";
			printYamlNode(node[i], indent + "  ");
		}
	}
	else if (node.IsScalar()) {
		// Print scalar values directly
		std::cout << node.as<std::string>() << std::endl;
	}
}

Curve::Curve()
{
	parameterMap = YAML::LoadFile(FileUtils::GetParameterMapFile().string());
}

void Curve::ShowJumpList()
{
	ImGuiChildFlags flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
	if (ImGui::BeginChild("cc tabs", ImVec2(400.0f, -1), flags))
	{
		ImGui::Text("jump data folder: %s", jumpDataFolder.string().c_str());
		ImGui::Text("ctrl voltages file: %s", ctrlVoltagesFile.string().c_str());
		if (ImGui::BeginListBox("##jump list", ImVec2(-1, 430)))
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
				
				if (!ctrlVoltages.empty())
				{
					ImGui::SameLine();
					if(ImGui::SmallButton("x"))
					{
						std::cout << "deleting jump: " << jump.filename << std::endl;
						RemovePhaseJump(i);
					}
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
				std::filesystem::path ctrlVoltFile = FileUtils::FindFileWithSubstring(folder, "ctrl_voltages");
				LoadPhaseJumpFolder(folder);
				if (!ctrlVoltFile.empty())
				{
					LoadCtrlVoltagesFile(ctrlVoltFile);
				}
				else
				{
					std::cout << "no ctrl voltage found in " << folder << std::endl;
				}
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
		if (ImGui::Button("load ctrl voltages"))
		{
			std::filesystem::path startpath = FileUtils::GetDataFolder() / (jumpDataFolder.string() + "\\");
			std::cout << startpath << std::endl;
			std::filesystem::path file = FileUtils::SelectFile(startpath, {"*.txt"});
			if (!file.empty())
			{
				LoadCtrlVoltagesFile(file);
			}
		}

		ShowParameterInputs();
	}
	ImGui::EndChild();
}

void Curve::ShowParameterInputs()
{
	bool changed = false;
	bool coolEnergyChanged = false;
	ImGui::BeginGroup();
	ImGui::PushItemWidth(100.0f);
	changed |= ImGui::InputInt("ion charge", &ionCharge);
	changed |= ImGui::Checkbox("use direct eff bunching voltage", &useDirectBunchingVoltage);
	ImGui::BeginDisabled(!useDirectBunchingVoltage);
	changed |= ImGui::InputDouble("effective bunching voltage direct [V]", &effectiveBunchingVoltageDirect, 0, 0, "%.3f");
	ImGui::EndDisabled();
	ImGui::BeginDisabled(useDirectBunchingVoltage);
	changed |= ImGui::InputDouble("effective bunching voltage sync [V]", &effectiveBunchingVoltageSync, 0, 0, "%.3f");
	ImGui::EndDisabled();
	changed |= ImGui::InputDouble("cooling ctrl voltage [V]", &coolingCtrlVoltage, 0, 0, "%.6f");
	coolEnergyChanged |= ImGui::InputDouble("cooling energy [eV]", &coolingEnergy, 0, 0, "%.4f");
	changed |= ImGui::InputDouble("electron current [A]", &electronCurrent, 0, 0, "%.3e");
	changed |= ImGui::InputDouble("contact offset potential [V]", &contactOffsetPotential, 0, 0, "%.3f");
	changed |= ImGui::InputDouble("cathode radius [m]", &cathodeRadius, 0, 0, "%.4e");
	changed |= ImGui::InputDouble("expansion factor", &expansionFactor, 0, 0, "%.3f");
	ImGui::PopItemWidth();
	ImGui::EndGroup();

	if(coolEnergyChanged)
	{
		// recalculate ctrl voltage if cooling energy is changed
		coolingCtrlVoltage = ConvertLabEnergyToCtrlVoltage(
			coolingEnergy,
			contactOffsetPotential,
			0, // energy correction is not used here
			electronCurrent,
			cathodeRadius,
			expansionFactor
		);
	}
	if (changed || coolEnergyChanged)
	{
		coolingEnergy = ConvertCtrlVoltageToLabEnergy(
			coolingCtrlVoltage,
			contactOffsetPotential,
			0, // energy correction is not used here
			electronCurrent,
			cathodeRadius,
			expansionFactor
		);
		RecalculateAllForcesAndDetungingVels();
	}
	ImGui::Separator();
	if (ImGui::Button("Fit Slope"))
	{
		FitSlope();
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Slopes"))
	{
		ClearSlopeList();
	}
	ImGui::SameLine();
	ImGui::Checkbox("show fit range", &showSlopeFitRange);

	ImGui::Text("Slope: (%.3e +- %.3e) eV s/m^2", finalSlopeValue, finalSlopeError);
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
	double effectiveBunchingVoltage = useDirectBunchingVoltage ? effectiveBunchingVoltageDirect : effectiveBunchingVoltageSync;
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
	//int i = 0;
	//for (PhaseJump& jump : jumps)
	for (int i = 0; i < jumps.size(); i++)
	{
		PhaseJump& jump = jumps.at(i);
		jump.AddTempValueToList();
		jumpValues.at(i) = jump.phaseJumpValue;
		jumpValueErrors.at(i) = jump.phaseJumpValueError;
		//i++;
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

	double effectiveBunchingVoltage = useDirectBunchingVoltage ? effectiveBunchingVoltageDirect : effectiveBunchingVoltageSync;

	coolingForceValues.push_back(CalculateCoolingForce(jump.phaseJumpValue, effectiveBunchingVoltage, ionCharge));
	coolingForceErrors.push_back(CalculateCoolingForceError(jump.phaseJumpValue, jump.phaseJumpValueError, effectiveBunchingVoltage, ionCharge));

	jump.curve = this;

	jump.index = jumps.size();
	// will call move Constructor
	jumps.emplace_back(std::move(jump));

	if (jumps.size() == 1)
	{
		selectedIndex = 0;
		SelectedItemChanged();
	}
}

void Curve::RemovePhaseJump(int index)
{
	if (index < 0 || index >= jumps.size())
	{
		std::cerr << "Index out of bounds: " << index << std::endl;
		return;
	}
	jumps.erase(jumps.begin() + index);
	jumpValues.erase(jumpValues.begin() + index);
	jumpValueErrors.erase(jumpValueErrors.begin() + index);
	coolingForceValues.erase(coolingForceValues.begin() + index);
	coolingForceErrors.erase(coolingForceErrors.begin() + index);

	ctrlVoltages.erase(ctrlVoltages.begin() + index);
	labEnergies.erase(labEnergies.begin() + index);
	detuningVelocities.erase(detuningVelocities.begin() + index);

	selectedIndex = std::min(selectedIndex, (int)jumps.size() - 1);
	SelectedItemChanged();

	// update the index of the later jumps
	for (int i = index; i < jumps.size(); i++)
	{
		jumps.at(i).index = i;
	}
}

void Curve::Plot()
{
	ImPlot::SetupAxes("Detuning Velocity [m/s]", "Cooling Force || [eV/m]");
	ImPlot::PlotScatter(name.c_str(), detuningVelocities.data(), coolingForceValues.data(), detuningVelocities.size());
	ImPlot::PlotErrorBars(name.c_str(), detuningVelocities.data(), coolingForceValues.data(), coolingForceErrors.data(), std::min(detuningVelocities.size(), coolingForceValues.size()));

	if (selectedIndex >= 0 && !detuningVelocities.empty() && !coolingForceValues.empty())
	{
		ImPlot::PushStyleColor(ImPlotCol_MarkerFill, ImVec4(1, 0, 0, 1));
		ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, 5);
		double selectedDetuningVel = detuningVelocities.at(selectedIndex % detuningVelocities.size());
		double selectedForceValue = coolingForceValues.at(selectedIndex % detuningVelocities.size());
		ImPlot::PlotScatter("##selected", &selectedDetuningVel, &selectedForceValue, 1);
		ImPlot::PopStyleVar();
		ImPlot::PopStyleColor();
	}
	
	if (showSlopeFitRange)
	{
		ImVec4 color = ImVec4(0.1, 0.6, 0.2, 1);
		ImPlot::DragLineX(0, &currentFitRange[0], color, 1, ImPlotDragToolFlags_Delayed);
		ImPlot::DragLineX(1, &currentFitRange[1], color, 1, ImPlotDragToolFlags_Delayed);
		ImPlot::TagX(currentFitRange[0], color, "fit start");
		ImPlot::TagX(currentFitRange[1], color, "fit end");

		PlotSlopes();
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

void Curve::PlotSlopes()
{
	for (int i = 0; i < slopeValues.size(); i++)
	{
		double x[2] = { fitRanges.at(2 * i),fitRanges.at(2 * i + 1) };
		double y[2] = { offsetValues.at(i) + slopeValues.at(i) * x[0], offsetValues.at(i) + slopeValues.at(i) * x[1] };

		ImGui::PushID(i);
		ImPlot::PlotLine("##slope", x, y, 2);
		ImGui::PopID();
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
	outfile << "# ctrl voltages file: " << ctrlVoltagesFile.string() << "\n";
	outfile << "# ion charge: " << ionCharge << "\n";
	outfile << "# cooling ctrl voltage [V]: " << coolingCtrlVoltage << "\n";
	outfile << "# cooling energy [eV]: " << coolingEnergy << "\n";
	outfile << "# effective bunching voltage direct [V]: " << effectiveBunchingVoltageDirect << "\n";
	outfile << "# effective bunching voltage sync [V]: " << effectiveBunchingVoltageSync << "\n";
	outfile << "# direct bunching voltage used ?: " << (useDirectBunchingVoltage ? "True" : "False") << "\n";
	outfile << "# electron current [A]: " << electronCurrent << "\n";
	outfile << "# contact offset potential [V}: " << contactOffsetPotential << "\n";
	outfile << "# cathode radius [m}: " << cathodeRadius << "\n";
	outfile << "# expansion factor: " << expansionFactor << "\n";
	outfile << "# slope value [eV s/m^2]: " << finalSlopeValue << "\n";
	outfile << "# slope error [eV s/m^2]: " << finalSlopeError << "\n";
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

	ClearSlopeList();

	if (std::filesystem::is_directory(inputFolder))
	{
		std::vector<std::filesystem::path> files;
		for (const auto& entry : std::filesystem::directory_iterator(inputFolder))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".csv")
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

		LoadParameterFromMap();
	}
	RecalculateAllTemporaryJumpValues();
}

void Curve::LoadCtrlVoltagesFile(std::filesystem::path inputFile)
{
	detuningVelocities.clear();
	labEnergies.clear();
	ctrlVoltages.clear();

	std::ifstream file;
	file.open(inputFile, std::ios::in);

	//FileUtils::GetHeaderFromFile(file);

	std::string line;
	if (file.is_open())
	{
		while (std::getline(file, line))
		{
			ctrlVoltages.push_back(std::stod(line));
			labEnergies.push_back(ConvertCtrlVoltageToLabEnergy(
				ctrlVoltages.back(),
				contactOffsetPotential,
				0, // energy correction is not used here
				electronCurrent,
				cathodeRadius,
				expansionFactor
			));
			detuningVelocities.push_back(CalculateDetuningVelocity(coolingEnergy, labEnergies.back()));
		}

		file.close();

		ctrlVoltagesFile = inputFile.filename().string();
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

	ClearSlopeList();

	std::ifstream file;
	file.open(inputFile, std::ios::in);

	if (file.is_open())
	{
		std::string header = FileUtils::GetHeaderFromFile(file);
		std::vector<std::string> tokens = FileUtils::SplitLine(header, "\n");

		jumpDataFolder = std::filesystem::path(FileUtils::RemoveLeadingTrailingSpaces(FileUtils::SplitLine(tokens[0], ":")[1]));
		ctrlVoltagesFile = std::filesystem::path(FileUtils::SplitLine(tokens[1], ":")[1]);
		ionCharge = std::stoi(FileUtils::SplitLine(tokens[2], ":")[1]);
		coolingCtrlVoltage = std::stod(FileUtils::SplitLine(tokens[3], ":")[1]);
		coolingEnergy = std::stod(FileUtils::SplitLine(tokens[4], ":")[1]);
		effectiveBunchingVoltageDirect = std::stod(FileUtils::SplitLine(tokens[5], ":")[1]);
		effectiveBunchingVoltageSync = std::stod(FileUtils::SplitLine(tokens[6], ":")[1]);
		useDirectBunchingVoltage = (FileUtils::SplitLine(tokens[7], ":")[1] == "True") ? true : false;
		electronCurrent = std::stod(FileUtils::SplitLine(tokens[8], ":")[1]);
		contactOffsetPotential = std::stod(FileUtils::SplitLine(tokens[9], ":")[1]);
		cathodeRadius = std::stod(FileUtils::SplitLine(tokens[10], ":")[1]);
		expansionFactor = std::stod(FileUtils::SplitLine(tokens[11], ":")[1]);
		finalSlopeValue = std::stod(FileUtils::SplitLine(tokens[12], ":")[1]);
		finalSlopeError = std::stod(FileUtils::SplitLine(tokens[13], ":")[1]);

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

void Curve::LoadParameterFromMap()
{
	try
	{
		std::string ionTypeFolder = jumpDataFolder.parent_path().filename().string();
		std::string runNumber = jumpDataFolder.filename().string().substr(3, 4);

		if (!parameterMap.IsMap())
		{
			std::cout << "parameter Map is not a map" << std::endl;
			return;
		}
		
		if (parameterMap[ionTypeFolder] && parameterMap[ionTypeFolder][runNumber] && !parameterMap[ionTypeFolder][runNumber].IsNull()) 
		{
			YAML::Node node = parameterMap[ionTypeFolder][runNumber];
			effectiveBunchingVoltageDirect = ReadDoubleFromYamlNode(node, "U_eff_direct_V", 0);
			effectiveBunchingVoltageSync = ReadDoubleFromYamlNode(node, "U_eff_sync_V", 0);
			contactOffsetPotential = ReadDoubleFromYamlNode(node, "U_c_V", 0);
			expansionFactor = ReadDoubleFromYamlNode(node, "alpha", 1.0);
			electronCurrent = ReadDoubleFromYamlNode(node, "I_e_uA", 0.0) * 1e-6;
			coolingCtrlVoltage = ReadDoubleFromYamlNode(node, "U_cool_ctrl_V_out", 0.0);
			cathodeRadius = ReadDoubleFromYamlNode(node, "r_cath", 1.2) * 1e-3; // convert to m

			coolingEnergy = ConvertCtrlVoltageToLabEnergy(
				coolingCtrlVoltage,
				contactOffsetPotential,
				0, // energy correction is not used here
				electronCurrent,
				cathodeRadius,
				expansionFactor,
				20 // number of iterations
			);
			//std::cout << "Loaded parameters for " << ionTypeFolder << " " << runNumber << std::endl;
			//std::cout << contactOffsetPotential << " V contact offset potential" << std::endl;
		}
		else
		{
			std::cout << "Parameter file does not include " << ionTypeFolder << " " << runNumber << std::endl;
		}
	}
	catch (const YAML::Exception& e)
	{
		std::cerr << "Error parsing YAML file: " << e.what() << std::endl;
	}
	
}

void Curve::SelectedItemChanged()
{
	UpdatePointPastJump();
}

void Curve::FitSlope()
{
	// Create a TGraphErrors object
	TGraphErrors graph = TGraphErrors(coolingForceValues.size(), detuningVelocities.data(), coolingForceValues.data(), nullptr, coolingForceErrors.data());

	// Define the fit function (a linear function)
	TF1 fitFunc = TF1("slope fit", "pol1", currentFitRange[0], currentFitRange[1]); // "pol1" specifies a first-degree polynomial

	// Perform the fit
	graph.Fit(&fitFunc, "R"); // "R" specifies fitting in the range

	// Get the slope and y offset
	double offset = fitFunc.GetParameter(0);
	double slope = fitFunc.GetParameter(1);
	double slopeError = fitFunc.GetParError(1);

	slopeValues.push_back(slope);
	offsetValues.push_back(offset);
	fitRanges.push_back(currentFitRange[0]);
	fitRanges.push_back(currentFitRange[1]);

	finalSlopeValue = CalculateMean(slopeValues);
	finalSlopeError = std::max(CalculateStdDev(slopeValues), slopeError);
}

void Curve::ClearSlopeList()
{
	slopeValues.clear();
	offsetValues.clear();
	fitRanges.clear();
	fitRanges.clear();

	finalSlopeValue = 0;
	finalSlopeError = 0;
}
