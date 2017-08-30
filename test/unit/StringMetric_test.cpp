#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>

#include "Measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(StringMetric, init)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        StringMetric subject("test_name", "tst", "test desc 1", [&dummy_clock]{return dummy_clock;}, "init");

        EXPECT_EQ(std::string(subject), "init");
    }

    TEST(StringMetric, assign)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        StringMetric subject("test_name", "tst", "test desc 1", [&dummy_clock]{return dummy_clock;}, "");

        EXPECT_EQ(std::string(subject), "");

        subject = "test value";
        EXPECT_EQ(std::string(subject), "test value");
    }

}
