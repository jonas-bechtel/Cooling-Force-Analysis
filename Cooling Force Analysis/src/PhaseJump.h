#pragma once
class Curve;

struct JumpEvaluationParameter
{
	double JumpTime = 1;
	double timePassedJump = 0.5;
	int movingAverageWindowSize = 11;

	bool useJumpBack = false;
	bool showJumpLine = true;
	bool plotMovingAverage = true;

	static double timePointPassedJump;

};

struct PhaseJump
{
	bool ShowAsListItem(bool selected);
	void ShowParameterInputs();
	void CalculateMovingAverage();
	void CalculateTemporaryJumpValue();
	void AddTempValueToList();
	void ClearValueList();
	void ClampJumpTimeToAllowedRange();
	void UpdatePointPastJump();

	void Plot();
	void PlotMovingAverage() const;
	void LoadFromFile(std::filesystem::path file);

	// main data
	std::vector<double> time;
	std::vector<double> phase;
	std::vector<double> radius;

	// secondary data
	std::vector<double> movingAveragePhase;

	// lists of jump values for different methods, mean is the main value and std is error
	double temporaryJumpValue = 0;
	std::vector<double> jumpValueList;     

	// individual time of jumps for some Data
	bool useIndividualJumpTime = false;
	double individualJumpTime = 1;
	
	// resulting values
	double phaseJumpValue = 0;
	double phaseJumpValueError = 0;

	// additional info
	std::string filename;
	int index = -1;
	Curve* curve = nullptr;

	// evaluation parameter
	static JumpEvaluationParameter params;

	
};

