#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
constexpr double kMeterToFeet = 3.28084;
constexpr double kMeterToYard = 1.09361;
}

double convert(const std::string& fromUnit, double value, const std::string& toUnit) {
    if (fromUnit == "meter" && toUnit == "feet") {
        return value * kMeterToFeet;
    }

    throw std::invalid_argument("Unsupported conversion");
}

#ifndef UNIT_CONVERTER_TEST
int main() {
    std::cout << "Insert value for converting (ex: meter:2.5): ";

    std::string input;
    std::getline(std::cin, input);

    std::string unit;
    double value = 0.0;

    std::size_t pos = input.find(':');
    if (pos == std::string::npos) {
        std::cerr << "Invalid format. Use unit:value (ex: meter:2.5)" << std::endl;
        return 1;
    }

    unit = input.substr(0, pos);
    std::string valueStr = input.substr(pos + 1);

    try {
        value = std::stod(valueStr);
    } catch (...) {
        std::cerr << "Invalid number: " << valueStr << std::endl;
        return 1;
    }

    double meterValue = 0.0;

    if (unit == "meter") {
        meterValue = value;
    } else if (unit == "feet") {
        meterValue = value / kMeterToFeet;
    } else if (unit == "yard") {
        meterValue = value / kMeterToYard;
    } else {
        std::cerr << "Unknown unit: " << unit << std::endl;
        return 1;
    }

    double inMeters = meterValue;
    double inFeet = meterValue * kMeterToFeet;
    double inYards = meterValue * kMeterToYard;

    std::cout << value << " " << unit << " = " << inMeters << " meter" << std::endl;
    std::cout << value << " " << unit << " = " << inFeet << " feet" << std::endl;
    std::cout << value << " " << unit << " = " << inYards << " yard" << std::endl;

    return 0;
}
#endif
