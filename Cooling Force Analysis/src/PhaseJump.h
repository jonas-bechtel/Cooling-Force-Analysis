#pragma once
struct PhaseJump
{
	bool ShowAsListItem(bool selected) const;
	void Plot() const;
	void LoadFromFile(std::filesystem::path file);

	// main data
	std::vector<double> time;
	std::vector<double> phase;
	std::vector<double> radius;

	// resulting values
	double phaseJumpValue = 0;
	double phaseJumpValueError = 0;

	// additional info
	std::string filename;

};

