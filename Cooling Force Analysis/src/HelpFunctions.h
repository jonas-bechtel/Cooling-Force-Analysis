#pragma once

double CalculateDetuningVelocity(double coolingEenrgy, double labEnergy);
double CalculateCoolingForce(double phaseJump, double effectiveBunchingVoltage, int ionCharge);
double CalculateCoolingForceError(double phaseJump, double phaseJumpError, double effectiveBunchingVoltage, int ionCharge);

std::vector<double> MovingAverage(const std::vector<double>& data, std::size_t windowSize);
int TimeToIndex(const std::vector<double>& list, double time);

double CalculateMean(const std::vector<double>& data);

double CalculateStdDev(const std::vector<double>& data);

