#pragma once

#include <cmath>
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Catch {

struct TestCase {
    std::string name;
    std::function<void()> body;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

class AutoReg {
public:
    AutoReg(std::string name, std::function<void()> body) {
        registry().push_back({std::move(name), std::move(body)});
    }
};

class AssertionFailure : public std::runtime_error {
public:
    explicit AssertionFailure(const std::string& message) : std::runtime_error(message) {}
};

class ApproxMatcher {
public:
    explicit ApproxMatcher(double target) : target_(target) {}

    ApproxMatcher& epsilon(double epsilon) {
        epsilon_ = epsilon;
        return *this;
    }

    bool matches(double actual) const {
        return std::fabs(actual - target_) <= epsilon_;
    }

private:
    double target_;
    double epsilon_ = 1e-12;
};

inline bool operator==(double actual, const ApproxMatcher& matcher) {
    return matcher.matches(actual);
}

inline int runTests() {
    int failures = 0;
    for (const auto& test : registry()) {
        try {
            test.body();
            std::cout << "[ OK ] " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
        } catch (...) {
            ++failures;
            std::cerr << "[FAIL] " << test.name << ": unknown exception\n";
        }
    }

    std::cout << registry().size() << " test case(s), " << failures << " failure(s)\n";
    return failures == 0 ? 0 : 1;
}

}  // namespace Catch

inline Catch::ApproxMatcher Approx(double target) {
    return Catch::ApproxMatcher(target);
}

#define CATCH_INTERNAL_CONCAT_IMPL(left, right) left##right
#define CATCH_INTERNAL_CONCAT(left, right) CATCH_INTERNAL_CONCAT_IMPL(left, right)

#define TEST_CASE(name, ...)                                                             \
    static void CATCH_INTERNAL_CONCAT(catch_test_, __LINE__)();                          \
    namespace {                                                                          \
    const Catch::AutoReg CATCH_INTERNAL_CONCAT(catch_reg_, __LINE__)(                    \
        name, CATCH_INTERNAL_CONCAT(catch_test_, __LINE__));                             \
    }                                                                                    \
    static void CATCH_INTERNAL_CONCAT(catch_test_, __LINE__)()

#define REQUIRE(expression)                                                              \
    do {                                                                                 \
        if (!(expression)) {                                                             \
            std::ostringstream catch_require_stream;                                     \
            catch_require_stream << "REQUIRE failed: " << #expression                   \
                                 << " at " << __FILE__ << ":" << __LINE__;             \
            throw Catch::AssertionFailure(catch_require_stream.str());                   \
        }                                                                                \
    } while (false)

#define REQUIRE_FALSE(expression) REQUIRE(!(expression))

#define FAIL(message)                                                                    \
    do {                                                                                 \
        throw Catch::AssertionFailure(message);                                          \
    } while (false)

#ifdef CATCH_CONFIG_MAIN
int main() {
    return Catch::runTests();
}
#endif
