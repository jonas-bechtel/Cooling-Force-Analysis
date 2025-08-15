#pragma once

// Computes lab energy from ctrl voltage for given parameters.
// niter: number of iterations (default: 100)
double ConvertCtrlVoltageToLabEnergy(
    double ctrlVoltage,
    double contactPotential,
    double energyCorrection,
    double electronCurrent,
    double cathodeRadius,
	double expansionFactor,
    int niter = 100
);

// Computes ctrl voltage from lab energy for given parameters.
double ConvertLabEnergyToCtrlVoltage(
    double labEnergy,
    double contactPotential,
    double energyCorrection,
    double electronCurrent,
    double cathodeRadius,
    double expansionFactor
);