#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(StringMetric, init)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        StringMetric subject("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, "init");
        EXPECT_EQ(subject.kind(), Metric::Kind::STR);

        EXPECT_EQ(std::string(subject), "init");
    }

    TEST(StringMetric, assign_str)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        StringMetric subject("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, "");
        EXPECT_EQ(subject.kind(), Metric::Kind::STR);

        EXPECT_EQ(std::string(subject), "");

        std::string new_val = "test value";
        subject = new_val;
        EXPECT_EQ(std::string(subject), "test value");
    }

    TEST(StringMetric, assign_char)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        StringMetric subject("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, "");
        EXPECT_EQ(subject.kind(), Metric::Kind::STR);

        EXPECT_EQ(std::string(subject), "");

        subject = "test value";
        EXPECT_EQ(std::string(subject), "test value");
    }

}
