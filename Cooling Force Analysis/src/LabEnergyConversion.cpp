#include "pch.h"
#include "LabEnergyConversion.h"
#include "Constants.h"

// Calibration factors (private to this module)
namespace 
{
    const double a_off = 3.05e-5;
    const double b_off = -10.0145;
    const double c_off = -0.0574;
}

inline double AccelerationFromCtrlVoltage(double ctrlVoltage) 
{
    return a_off * ctrlVoltage * ctrlVoltage + b_off * ctrlVoltage + c_off;
}

inline double CtrlVoltageFromAcceleration(double accelerationVoltage)
{
	// abc formula for quadratic equation: a * x^2 + b * x + c = 0
	return (-b_off - std::sqrt(b_off * b_off - 4.0 * a_off * (c_off - accelerationVoltage))) / (2.0 * a_off);
}

inline double ElectronVelocity(double labEnergy) 
{
    // labEnergy in [eV], mass in [eV/c^2]
    return PhysicalConstants::speedOfLight * std::sqrt(1.0 - std::pow(1.0 / (1.0 + labEnergy / PhysicalConstants::electronMassEV), 2.0));
}

inline double ElectronDensity(double v_e, double electronCurrent, double beamRadius)
{
    // electronCurrent [A], cathodeRadius [m]
    return electronCurrent / (PhysicalConstants::elementaryCharge * v_e * PhysicalConstants::pi * beamRadius * beamRadius);
}

inline double SpaceChargePotential(double v_e, double electronCurrent, double beamRadius)
{
    double log_arg = CSR::driftTubeRadius / beamRadius;
    double log_val = 1.0 + 2.0 * std::log(log_arg);
    return electronCurrent * log_val / (4.0 * PhysicalConstants::pi * PhysicalConstants::epsilon_0 * v_e);
}

inline double LabEnergy(double Uacc, double U_sc0, double contactPotential, double energyCorrection)
{
    return (Uacc - U_sc0 - contactPotential - energyCorrection);
}

double ConvertCtrlVoltageToLabEnergy(
    double ctrlVoltage,
    double contactPotential,
    double energyCorrection,
    double electronCurrent,
    double cathodeRadius,
    double expansionFactor,
    int niter
)
{
	double beamRadius = cathodeRadius * sqrt(expansionFactor); 
    double Uacc = AccelerationFromCtrlVoltage(ctrlVoltage);
	//std::cout << "Uacc: " << Uacc << std::endl;
    double labEnergy = LabEnergy(Uacc, 0, contactPotential, energyCorrection);
	//std::cout << "Initial lab energy: " << labEnergy << std::endl;
    for (int i = 0; i < niter; i++)
    {
        double v_e = ElectronVelocity(labEnergy);
		//std::cout << "Iteration " << i << ": v_e = " << v_e << std::endl;
        double n_e = ElectronDensity(v_e, electronCurrent, beamRadius) * 1e-11;
        double U_sc = SpaceChargePotential(v_e, electronCurrent, beamRadius);
		//std::cout << "Iteration " << i << ": U_sc = " << U_sc << std::endl;
        labEnergy = LabEnergy(Uacc, U_sc, contactPotential, energyCorrection);
		//std::cout << "Iteration " << i << ": labEnergy = " << labEnergy << std::endl;
    }
	
    return labEnergy;
}

double ConvertLabEnergyToCtrlVoltage(
    double labEnergy, 
    double contactPotential,
    double energyCorrection, 
    double electronCurrent,
    double cathodeRadius, 
    double expansionFactor)
{
    double beamRadius = cathodeRadius * sqrt(expansionFactor);
    double v_e = ElectronVelocity(labEnergy);
    double U_sc = SpaceChargePotential(v_e, electronCurrent, beamRadius);
	double Uacc = labEnergy + U_sc + contactPotential + energyCorrection;

    return CtrlVoltageFromAcceleration(Uacc);
}
