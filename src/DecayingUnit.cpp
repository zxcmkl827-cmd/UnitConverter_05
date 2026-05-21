#include <string>
#include <vector>
#include <map>
#include <stdexcept>

struct ConversionResult {
    std::string unit;
    double value;
};

namespace {
std::map<std::string, double>& unitRates() {
    static std::map<std::string, double> rates;
    return rates;
}

double rateToMeter(const std::string& unit) {
    const auto found = unitRates().find(unit);
    if (found == unitRates().end()) {
        throw std::invalid_argument("UNKNOWN_UNIT");
    }

    return found->second;
}
}

void resetDefaultUnits() {
    auto& rates = unitRates();
    rates.clear();
    rates["meter"] = 1.0;
    rates["feet"] = 1.0 / 3.28084;
    rates["yard"] = 1.0 / 1.09361;
}

void registerUnit(const std::string& unit, double rateToMeterValue) {
    if (rateToMeterValue <= 0.0) {
        throw std::invalid_argument("INVALID_UNIT");
    }

    unitRates()[unit] = rateToMeterValue;
}

double convert(const std::string& fromUnit, double value, const std::string& toUnit) {
    const double meters = value * rateToMeter(fromUnit);
    return meters / rateToMeter(toUnit);
}

std::vector<ConversionResult> convertAll(const std::string& fromUnit, double value) {
    rateToMeter(fromUnit);

    std::vector<ConversionResult> conversions;
    for (const auto& unit : unitRates()) {
        if (unit.first != fromUnit) {
            conversions.push_back({unit.first, convert(fromUnit, value, unit.first)});
        }
    }

    return conversions;
}
