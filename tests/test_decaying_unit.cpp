#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>
#include <vector>

#define UNIT_CONVERTER_TEST
#include "../src/DecayingUnit.cpp"

namespace {
template <typename Action>
void requireInvalidArgument(Action action) {
    try {
        action();
        FAIL("Expected std::invalid_argument");
    } catch (const std::invalid_argument&) {
        REQUIRE(true);
    }
}

const ConversionResult* findConversion(const std::vector<ConversionResult>& conversions,
                                       const std::string& unit) {
    for (const auto& conversion : conversions) {
        if (conversion.unit == unit) {
            return &conversion;
        }
    }

    return nullptr;
}
}

TEST_CASE("감쇠 단위 변환", "[bonus]") {
    resetDefaultUnits();

    registerUnit("cubit", 0.4572);
    REQUIRE(convert("cubit", 1.0, "meter") == Approx(0.4572).epsilon(1e-5));

    resetDefaultUnits();
    registerUnit("cubit", 0.4572);
    REQUIRE(convert("meter", 1.0, "cubit") == Approx(1.0 / 0.4572).epsilon(1e-5));

    resetDefaultUnits();
    registerUnit("cubit", 0.4572);
    REQUIRE(convert("cubit", 1.0, "feet") == Approx(0.4572 * 3.28084).epsilon(1e-5));

    resetDefaultUnits();
    requireInvalidArgument([] { registerUnit("cubit", -0.4572); });

    resetDefaultUnits();
    registerUnit("cubit", 0.4572);

    const auto conversions = convertAll("cubit", 1.0);
    const auto* feet = findConversion(conversions, "feet");
    const auto* meter = findConversion(conversions, "meter");
    const auto* yard = findConversion(conversions, "yard");

    REQUIRE(conversions.size() == 3);
    REQUIRE(feet != nullptr);
    REQUIRE(meter != nullptr);
    REQUIRE(yard != nullptr);
    REQUIRE(feet->value == Approx(0.4572 * 3.28084).epsilon(1e-5));
    REQUIRE(meter->value == Approx(0.4572).epsilon(1e-5));
    REQUIRE(yard->value == Approx(0.4572 * 1.09361).epsilon(1e-5));

    resetDefaultUnits();
    REQUIRE(convert("meter", 1.0, "feet") == Approx(3.28084).epsilon(1e-5));
}
