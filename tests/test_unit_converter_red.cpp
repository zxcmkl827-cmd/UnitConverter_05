#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <fstream>
#include <stdexcept>

#define UNIT_CONVERTER_TEST
#include "../cpp/UnitConverter.cpp"

namespace {
void requireInvalidArgument(void (*action)()) {
    try {
        action();
        FAIL("Expected std::invalid_argument");
    } catch (const std::invalid_argument&) {
        REQUIRE(true);
    }
}
}

TEST_CASE("parse_meter_input_returns_conversion_result", "[boundary]") {
    resetDefaultUnits();

    const auto result = handleInput("meter:2.5");

    REQUIRE(result.source.unit == "meter");
    REQUIRE(result.source.value == Approx(2.5).epsilon(1e-5));
    REQUIRE(result.conversions.size() == 2);
    REQUIRE(result.conversions[0].unit == "feet");
    REQUIRE(result.conversions[0].value == Approx(8.20210).epsilon(1e-5));
}

TEST_CASE("parse_missing_colon_throws_invalid_argument", "[boundary]") {
    requireInvalidArgument([] { parseInput("meter"); });
}

TEST_CASE("parse_negative_value_throws_invalid_argument", "[boundary]") {
    requireInvalidArgument([] { parseInput("meter:-1.0"); });
}

TEST_CASE("parse_unknown_unit_throws_invalid_argument", "[boundary]") {
    requireInvalidArgument([] { parseInput("parsec:1.0"); });
}

TEST_CASE("parse_invalid_decimal_throws_invalid_argument", "[boundary]") {
    requireInvalidArgument([] { parseInput("meter:abc"); });
}

TEST_CASE("parse_rejects_empty_unit_empty_value_and_trailing_text", "[boundary]") {
    requireInvalidArgument([] { parseInput(":1.0"); });
    requireInvalidArgument([] { parseInput("meter:"); });
    requireInvalidArgument([] { parseInput("meter:1.0x"); });
}

TEST_CASE("format_plain_output_preserves_source_unit_and_value", "[boundary]") {
    resetDefaultUnits();

    const auto result = handleInput("meter:2.5");
    const auto output = formatPlain(result);

    REQUIRE(output.find("2.5 meter = ") != std::string::npos);
    REQUIRE(output.find(" feet") != std::string::npos);
}

TEST_CASE("format_json_output_returns_expected_schema", "[boundary]") {
    resetDefaultUnits();

    const auto result = handleInput("meter:0");
    const auto output = formatJson(result);

    REQUIRE(output.find("\"sourceUnit\":\"meter\"") != std::string::npos);
    REQUIRE(output.find("\"sourceValue\":0") != std::string::npos);
    REQUIRE(output.find("\"conversions\"") != std::string::npos);
}

TEST_CASE("convert_meter_to_feet_returns_correct_ratio", "[domain]") {
    resetDefaultUnits();
    REQUIRE(convert("meter", 2.5, "feet") == Approx(8.20210).epsilon(1e-5));
}

TEST_CASE("convert_meter_to_yard_returns_correct_ratio", "[domain]") {
    resetDefaultUnits();
    REQUIRE(convert("meter", 1.0, "yard") == Approx(1.09361).epsilon(1e-5));
}

TEST_CASE("convert_feet_to_meter_returns_inverse_ratio", "[domain]") {
    resetDefaultUnits();
    REQUIRE(convert("feet", 1.0, "meter") == Approx(0.30480).epsilon(1e-5));
}

TEST_CASE("convert_yard_to_meter_returns_inverse_ratio", "[domain]") {
    resetDefaultUnits();
    REQUIRE(convert("yard", 1.0, "meter") == Approx(0.91440).epsilon(1e-5));
}

TEST_CASE("convert_all_returns_all_registered_unit_results", "[domain]") {
    resetDefaultUnits();

    const auto conversions = convertAll("meter", 1.0);

    REQUIRE(conversions.size() == 2);
    REQUIRE(conversions[0].unit == "feet");
    REQUIRE(conversions[0].value == Approx(3.28084).epsilon(1e-5));
    REQUIRE(conversions[1].unit == "yard");
    REQUIRE(conversions[1].value == Approx(1.09361).epsilon(1e-5));
}

TEST_CASE("register_unit_enables_new_unit_conversion", "[domain]") {
    resetDefaultUnits();

    registerUnit("cubit", 0.4572);

    REQUIRE(convert("cubit", 1.0, "meter") == Approx(0.4572).epsilon(1e-5));
}

TEST_CASE("register_unit_rejects_invalid_definition", "[domain]") {
    requireInvalidArgument([] { registerUnit("bad-unit", 1.0); });
}

TEST_CASE("load_json_config_applies_configured_rates", "[domain]") {
    resetDefaultUnits();
    const char* path = "unit_converter_test_units.json";
    {
        std::ofstream config(path);
        config << "{\"units\":[{\"name\":\"inch\",\"rateToMeter\":0.0254}]}";
    }

    REQUIRE(loadConfig(path));
    REQUIRE(convert("inch", 1.0, "meter") == Approx(0.0254).epsilon(1e-5));
    std::remove(path);
}

TEST_CASE("load_yaml_config_applies_configured_rates", "[domain]") {
    resetDefaultUnits();
    const char* path = "unit_converter_test_units.yaml";
    {
        std::ofstream config(path);
        config << "- name: hand\n  rateToMeter: 0.1016\n";
    }

    REQUIRE(loadConfig(path));
    REQUIRE(convert("hand", 1.0, "meter") == Approx(0.1016).epsilon(1e-5));
    std::remove(path);
}

TEST_CASE("load_invalid_config_path_keeps_default_rates", "[domain]") {
    resetDefaultUnits();

    REQUIRE_FALSE(loadConfig("missing_unit_converter_config.json"));
    REQUIRE(convert("meter", 1.0, "feet") == Approx(3.28084).epsilon(1e-5));
}
