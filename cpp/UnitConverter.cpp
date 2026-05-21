#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr double METER_TO_FEET = 328084.0 / 100000.0;
constexpr double METER_TO_YARD = 109361.0 / 100000.0;
constexpr double METER_RATE = 1.0;
constexpr double FEET_RATE_TO_METER = METER_RATE / METER_TO_FEET;
constexpr double YARD_RATE_TO_METER = METER_RATE / METER_TO_YARD;

struct ParsedInput {
    std::string unit;
    double value;
};

struct ConversionResult {
    std::string unit;
    double value;
};

struct BoundaryResult {
    ParsedInput source;
    std::vector<ConversionResult> conversions;
};

bool isValidUnitName(const std::string& unit) {
    if (unit.empty() || !std::islower(static_cast<unsigned char>(unit[0]))) {
        return false;
    }

    for (const char character : unit) {
        const auto value = static_cast<unsigned char>(character);
        if (!std::islower(value) && !std::isdigit(value) && character != '_') {
            return false;
        }
    }

    return true;
}

class UnitRegistry {
public:
    void resetDefaults() {
        rates_.clear();
        rates_["meter"] = METER_RATE;
        rates_["feet"] = FEET_RATE_TO_METER;
        rates_["yard"] = YARD_RATE_TO_METER;
    }

    void registerUnit(const std::string& unit, double rateToMeterValue) {
        if (!isValidUnitName(unit) || rateToMeterValue <= 0.0) {
            throw std::invalid_argument("INVALID_UNIT");
        }

        rates_[unit] = rateToMeterValue;
    }

    double rateToMeter(const std::string& unit) const {
        const auto found = rates_.find(unit);
        if (found == rates_.end()) {
            throw std::invalid_argument("UNKNOWN_UNIT");
        }

        return found->second;
    }

    const std::map<std::string, double>& units() const {
        return rates_;
    }

private:
    std::map<std::string, double> rates_;
};

UnitRegistry& unitRegistry() {
    static UnitRegistry registry;
    return registry;
}

double rateToMeter(const std::string& unit) {
    return unitRegistry().rateToMeter(unit);
}

void registerConfiguredUnit(const std::string& unit, double rateToMeterValue) {
    unitRegistry().registerUnit(unit, rateToMeterValue);
}

class InputParser {
public:
    ParsedInput parse(const std::string& input) const {
        const std::size_t pos = input.find(':');
        if (pos == std::string::npos || pos != input.rfind(':')) {
            throw std::invalid_argument("INVALID_FORMAT");
        }

        const std::string unit = input.substr(0, pos);
        const std::string valueText = input.substr(pos + 1);
        if (!isValidUnitName(unit)) {
            throw std::invalid_argument("INVALID_UNIT_NAME");
        }
        if (valueText.empty()) {
            throw std::invalid_argument("INVALID_VALUE");
        }

        std::size_t parsedLength = 0;
        double value = 0.0;
        try {
            value = std::stod(valueText, &parsedLength);
        } catch (...) {
            throw std::invalid_argument("INVALID_VALUE");
        }

        if (parsedLength != valueText.size()) {
            throw std::invalid_argument("INVALID_VALUE");
        }
        if (value < 0.0) {
            throw std::invalid_argument("NEGATIVE_VALUE");
        }

        rateToMeter(unit);
        return {unit, value};
    }
};

class OutputFormatter {
public:
    std::string formatPlain(const BoundaryResult& result) const {
        std::ostringstream output;
        for (const auto& conversion : result.conversions) {
            output << std::defaultfloat << result.source.value << ' ' << result.source.unit
                   << " = " << std::fixed << std::setprecision(6) << conversion.value << ' '
                   << conversion.unit << '\n';
        }

        return output.str();
    }

    std::string formatJson(const BoundaryResult& result) const {
        std::ostringstream output;
        output << "{\"sourceUnit\":\"" << result.source.unit << "\",\"sourceValue\":"
               << result.source.value << ",\"conversions\":[";
        for (std::size_t index = 0; index < result.conversions.size(); ++index) {
            if (index != 0) {
                output << ',';
            }
            output << "{\"unit\":\"" << result.conversions[index].unit << "\",\"value\":"
                   << result.conversions[index].value << '}';
        }
        output << "]}";

        return output.str();
    }
};
}

void resetDefaultUnits() {
    unitRegistry().resetDefaults();
}

void registerUnit(const std::string& unit, double rateToMeterValue) {
    registerConfiguredUnit(unit, rateToMeterValue);
}

ParsedInput parseInput(const std::string& input) {
    return InputParser().parse(input);
}

double convert(const std::string& fromUnit, double value, const std::string& toUnit) {
    const double meters = value * rateToMeter(fromUnit);
    return meters / rateToMeter(toUnit);
}

std::vector<ConversionResult> convertAll(const std::string& fromUnit, double value) {
    rateToMeter(fromUnit);

    std::vector<ConversionResult> conversions;
    for (const auto& unit : unitRegistry().units()) {
        if (unit.first != fromUnit) {
            conversions.push_back({unit.first, convert(fromUnit, value, unit.first)});
        }
    }

    return conversions;
}

BoundaryResult handleInput(const std::string& input) {
    const ParsedInput parsed = parseInput(input);
    return {parsed, convertAll(parsed.unit, parsed.value)};
}

std::string formatPlain(const BoundaryResult& result) {
    return OutputFormatter().formatPlain(result);
}

std::string formatJson(const BoundaryResult& result) {
    return OutputFormatter().formatJson(result);
}

bool loadConfig(const std::string& path) {
    std::ifstream config(path);
    if (!config) {
        return false;
    }

    const std::string content((std::istreambuf_iterator<char>(config)),
                              std::istreambuf_iterator<char>());
    const std::regex unitPattern(
        R"((?:\"name\"\s*:\s*\"([a-z][a-z0-9_]*)\"|name\s*:\s*([a-z][a-z0-9_]*))[\s\S]*?(?:\"rateToMeter\"\s*:\s*|rateToMeter\s*:\s*)([0-9]+(?:\.[0-9]+)?))");

    bool loaded = false;
    for (std::sregex_iterator match(content.begin(), content.end(), unitPattern), end;
         match != end; ++match) {
        const std::string unit =
            (*match)[1].matched ? (*match)[1].str() : (*match)[2].str();
        registerConfiguredUnit(unit, std::stod((*match)[3].str()));
        loaded = true;
    }

    return loaded;
}

#ifndef UNIT_CONVERTER_TEST
int main() {
    resetDefaultUnits();
    std::cerr << "Insert value for converting (ex: meter:2.5): ";

    std::string input;
    std::getline(std::cin, input);

    try {
        std::cout << formatPlain(handleInput(input));
    } catch (const std::invalid_argument& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    return 0;
}
#endif
