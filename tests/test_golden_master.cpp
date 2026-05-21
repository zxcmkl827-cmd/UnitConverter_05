#define APPROVALS_CATCH
#include "ApprovalTests.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

#define TEST_F(Fixture, TestName) TEST_CASE(#Fixture "." #TestName, "[golden-master]")

#ifndef UNIT_CONVERTER_EXE
#define UNIT_CONVERTER_EXE "./UnitConverter"
#endif

auto approvalDirectoryDisposer =
    ApprovalTests::Approvals::useApprovalsSubdirectory("approval_tests");

namespace {
class GoldenMasterTest {};

std::string readFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    REQUIRE(input.good());
    return std::string((std::istreambuf_iterator<char>(input)),
                       std::istreambuf_iterator<char>());
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream output(path, std::ios::binary);
    output << content;
}

std::string normalizeLineEndings(const std::string& text) {
    std::string normalized;
    normalized.reserve(text.size());
    for (const char ch : text) {
        if (ch != '\r') {
            normalized.push_back(ch);
        }
    }
    return normalized;
}

std::string captureStdoutForScenario(const std::string& testCaseId, const std::string& scenario) {
    const std::string inputPath = testCaseId + "_input.txt";
    const std::string actualPath = testCaseId + "_actual.txt";
    writeFile(inputPath, scenario + "\n");

#ifdef _WIN32
    const std::string command = "cmd.exe /D /C \"\"" + std::string(UNIT_CONVERTER_EXE) +
                                "\" < \"" + inputPath + "\" > \"" + actualPath + "\"\"";
#else
    const std::string command = "'" + std::string(UNIT_CONVERTER_EXE) + "' < '" + inputPath +
                                "' > '" + actualPath + "'";
#endif
    const int exitCode = std::system(command.c_str());
    REQUIRE(exitCode == 0);

    const std::string actual = normalizeLineEndings(readFile(actualPath));
    std::remove(inputPath.c_str());
    std::remove(actualPath.c_str());
    return actual;
}

void expectApprovedOutput(const std::string& testCaseId, const std::string& scenario) {
    const std::string actual = captureStdoutForScenario(testCaseId, scenario);

    ApprovalTests::Approvals::verify(
        actual, ApprovalTests::Options().fileOptions().withFileExtension(".txt"));
}
}

TEST_F(GoldenMasterTest, UnitConverter_meter_2_5) {
    expectApprovedOutput("GM_TC_01", "meter:2.5");
}

TEST_F(GoldenMasterTest, UnitConverter_feet_1_0) {
    expectApprovedOutput("GM_TC_02", "feet:1.0");
}

TEST_F(GoldenMasterTest, UnitConverter_yard_1_0) {
    expectApprovedOutput("GM_TC_03", "yard:1.0");
}

TEST_F(GoldenMasterTest, UnitConverter_meter_0_0) {
    expectApprovedOutput("GM_TC_04", "meter:0.0");
}
