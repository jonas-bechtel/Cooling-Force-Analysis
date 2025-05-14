#pragma once
#include "PhaseJump.h"

#include "yaml-cpp/yaml.h"

class Curve
{
public:
	Curve();

	void ShowJumpList();
	void ShowParameterInputs();
	void ShowCurrentPhaseJumpParameters();

	void SetName(std::string newName);
	std::string GetName();

	void RecalculateAllForcesAndDetungingVels();
	void RecalculateAllMovingAverages();
	void RecalculateAllTemporaryJumpValues();
	void AddAllTempJumpValuesToList();
	void ClearAllValueList();
	void ClampJumpTimesToAllowedRange();
	void UpdatePointPastJump();

	void FitSlope();
	void ClearSlopeList();

	void AddPhaseJump(PhaseJump& jump);

	void Plot();
	void PlotSelectedJump();
	void PlotSlopes();

	void Save();
	void LoadPhaseJumpFolder(std::filesystem::path folder);
	void LoadLabEnergiesFile(std::filesystem::path inputFile);
	void LoadFromFile(std::filesystem::path file);
	void LoadParameterFromMap();

private:
	void SelectedItemChanged();

private:
	// main data
	std::vector<PhaseJump> jumps;
	int selectedIndex = -1;

	std::vector<double> jumpValues;
	std::vector<double> jumpValueErrors;

	std::vector<double> labEnergies;

	std::vector<double> detuningVelocities;
	std::vector<double> coolingForceValues;
	std::vector<double> coolingForceErrors;

	// for slope determination
	std::vector<double> slopeValues;
	std::vector<double> offsetValues;
	std::vector<double> fitRanges;
	double finalSlopeValue = 0;
	double finalSlopeError = 0;
	double currentFitRange[2] = { -1000, 1000 };
	bool showSlopeFitRange = false;

	// parameters
	int ionCharge = 1;
	double coolingEnergy = 1;
	double effectiveBunchingVoltageDirect = 1;
	double effectiveBunchingVoltageSync = 1;
	bool useDirectBunchingVoltage = true;

	// labelling
	std::string name = "name";
	std::filesystem::path jumpDataFolder = "jump folder";
	std::filesystem::path labEnergyFile = "lab energy file";

	static YAML::Node parameterMap;

	friend class PhaseJump;
};

