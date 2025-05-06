#pragma once
#include "PhaseJump.h"


class Curve
{
public:
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

	void AddPhaseJump(PhaseJump& jump);

	void Plot() const;
	void PlotSelectedJump();

	void Save();
	void LoadPhaseJumpFolder(std::filesystem::path folder);
	void LoadLabEnergiesFile(std::filesystem::path inputFile);
	void LoadFromFile(std::filesystem::path file);

private:
	void SelectedItemChanged();

private:
	std::vector<PhaseJump> jumps;
	int selectedIndex = -1;

	std::vector<double> jumpValues;
	std::vector<double> jumpValueErrors;

	std::vector<double> labEnergies;

	std::vector<double> detuningVelocities;
	std::vector<double> coolingForceValues;
	std::vector<double> coolingForceErrors;

	// parameter
	int ionCharge = 1;
	double coolingEnergy = 1;
	double effectiveBunchingVoltage = 1;

	std::string name = "name";
	std::filesystem::path jumpDataFolder = "jump folder";
	std::filesystem::path labEnergyFile = "lab energy file";
};

