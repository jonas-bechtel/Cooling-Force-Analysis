#pragma once

namespace PhysicalConstants {

	// electron mass [kg]
	constexpr double electronMass = 9.10938291e-31;
	// electron mass [amu]
	constexpr double electronMassAMU = 5.4857990946e-4;
	// electron mass [eV/c^2]
	constexpr double electronMassEV = 0.5109989461e6;
	// permitivity of vacuum [Coulomb / (V * m)]
	constexpr double epsilon_0 = 8.854187817e-12;
	// atomic mass unit [kg]
	constexpr double atomicMassUnit = 1.66053886e-27;
	// speed of light [m/s]
	constexpr double speedOfLight = 299792458.0;
	// elementary charge [Coulomb]
	constexpr double elementaryCharge = 1.602176634e-19;
	// pi
	constexpr double pi = 3.1415926535897932;
}

namespace CSR
{
	// taken from Daniel Paul PHD thesis Appendix C equation C.29 [m]
	constexpr double overlapLength = 1.13614;

	// drift Tube length where cooling force is doing something [m]
	constexpr double coolerLength = 0.8;

	// drift Tube radius [m]
	constexpr double driftTubeRadius = 0.05;
}