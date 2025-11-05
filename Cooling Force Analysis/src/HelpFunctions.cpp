#include "pch.h"
#include "HelpFunctions.h"
#include "Constants.h"

double CalculateDetuningVelocity(double coolingEnergy, double labEnergy)
{
    return sqrt(2 / PhysicalConstants::electronMass) * (sqrt(coolingEnergy * PhysicalConstants::elementaryCharge) - sqrt(labEnergy * PhysicalConstants::elementaryCharge));
}

double CalculateCoolingForce(double phaseJump, double effectiveBunchingVoltage, int ionCharge)
{
    return ionCharge * effectiveBunchingVoltage * sin(phaseJump * PhysicalConstants::pi / 180) / CSR::coolerLength;
}

// error using gaussian error propagation
double CalculateCoolingForceError(double phaseJump, double phaseJumpError, double effectiveBunchingVoltage, int ionCharge)
{
    return abs(ionCharge * effectiveBunchingVoltage * cos(phaseJump * PhysicalConstants::pi / 180) / CSR::coolerLength * phaseJumpError * PhysicalConstants::pi / 180);
}

std::vector<double> MovingAverage(const std::vector<double>& data, std::size_t windowSize)
{
    if (windowSize == 0 || windowSize > data.size())
    {
        throw std::invalid_argument("Window size must be greater than 0 and less than or equal to the size of data.");
    }

    std::vector<double> averages;
    double windowSum = 0.0;

    // Initialize the sum with the first 'windowSize' elements
    for (std::size_t i = 0; i < windowSize; ++i) 
    {
        windowSum += data[i];
    }
    averages.push_back(windowSum / windowSize);

    // Slide the window across the vector
    for (std::size_t i = windowSize; i < data.size(); ++i)
    {
        windowSum += data[i] - data[i - windowSize];  // Update sum by adding new element and removing old element
        averages.push_back(windowSum / windowSize);   // Calculate average and store
    }

    return averages;
}

int TimeToIndex(const std::vector<double>& list, double time)
{
    if (list.size() < 2)
        return 0;

    float timestep = list[1] - list[0];
    return std::round(time / timestep);
}

double CalculateMean(const std::vector<double>& data)
{
    double sum = std::accumulate(data.begin(), data.end(), 0.0);

    return sum / data.size();
}

double CalculateStdDev(const std::vector<double>& data) 
{
    double mean = CalculateMean(data);
    double squaredDiffSum = 0.0;
    for (double value : data) 
    {
        squaredDiffSum += (value - mean) * (value - mean);
    }
    return std::sqrt(squaredDiffSum / data.size());
}

double ReadDoubleFromYamlNode(const YAML::Node& node, const std::string& key, double defaultValue)
{
    if (node[key].IsDefined() && node[key].IsScalar())
    {
        try
        {
            return node[key].as<double>();
        }
        catch (const YAML::Exception& e)
        {
            std::cerr << "Error reading double from YAML node: " << e.what() << std::endl;
        }
    }
    else
    {
        std::cout << "Key '" << key << "' not found or not a scalar in the YAML node." << std::endl;
	}
    return defaultValue;
}
