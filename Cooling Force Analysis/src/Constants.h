#pragma once

namespace PhysicalConstants {

	// electron mass [kg]
	const double electronMass = 9.10938291e-31;
	// electron mass [amu]
	const double electronMassAMU = 5.4857990946e-4;
	// electron mass [eV/c^2]
	const double electronMassEV = 0.5109989461e6;
	// permitivity of vacuum [Coulomb / (V * m)]
	const double epsilon_0 = 8.854187817e-12;
	// atomic mass unit [kg]
	const double atomicMassUnit = 1.66053886e-27;
	// speed of light [m/s]
	const double speedOfLight = 299792458.0;
	// elementary charge [Coulomb]
	const double elementaryCharge = 1.602176634e-19;
	// pi
	constexpr double pi = 3.1415926535897932;
}

namespace CSR
{
	// taken from Daniel Paul PHD thesis Appendix C equation C.29 [m]
	const double overlapLength = 1.13614;

	// drift Tube length where cooling force is doing something [m]
	const double coolerLength = 0.8;

	// drift Tube radius [m]
	const double driftTubeRadius = 0.05;
}