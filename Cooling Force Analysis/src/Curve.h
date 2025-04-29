#pragma once
#include "PhaseJump.h"

double CalculateDetuningVelocity(double coolingEenrgy, double labEnergy);
double CalculateCoolingForce(double phaseJump, double effectiveBunchingVoltage, int ionCharge);

class Curve
{
public:
	void ShowJumpList();

	void AddPhaseJump(PhaseJump& jump);

	void Plot() const;
	void PlotSelectedJump() const;

	//void Save();
	void LoadPhaseJumpFolder(std::filesystem::path folder);
	//void LoadLabEnergiesFile(std::filesystem::path inputFile);
	//void LoadFromFile(std::filesystem::path file);

private:
	void SelectedItemChanged();

private:
	std::vector<PhaseJump> jumps;
	int selectedIndex = -1;

	std::vector<double> jumpValues;
	std::vector<double> jumpValueErrors;

	std::vector<double> detuningVelocities;
	std::vector<double> coolingForceValues;
	std::vector<double> coolingForceErrors;

	// parameter
	int ionCharge = 1;
	double coolingEnergy = 0;
	double effectiveBunchingVoltage = 0;

	std::string name = "name";
	std::filesystem::path folder = "folder";
};

