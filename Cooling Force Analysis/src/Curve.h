#pragma once
#include "PhaseJump.h"

class Curve
{
public:
	void ShowJumpList();

	void AddPhaseJump(PhaseJump& jump);

	void Save();
	void LoadPhaseJumpFolder(std::filesystem::path folder);
	void LoadFromFile(std::filesystem::path file);

private:
	void SelectedItemChanged();

private:
	std::vector<PhaseJump> jumps;
	int selectedIndex = -1;

	std::vector<double> jumpValues;
	std::vector<double> detuningVelocities;
	std::vector<double> jumpValueErrors;

	std::string name = "";
	std::filesystem::path folder = "";
};

