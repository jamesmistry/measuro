#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(BoolMetric, true_default_val)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        BoolMetric subject("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, true);
        EXPECT_EQ(subject.kind(), Metric::Kind::BOOL);

        EXPECT_EQ(bool(subject), true);
        EXPECT_EQ(std::string(subject), "TRUE");
    }

    TEST(BoolMetric, false_default_val)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        BoolMetric subject("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, false);
        EXPECT_EQ(subject.kind(), Metric::Kind::BOOL);

        EXPECT_EQ(bool(subject), false);
        EXPECT_EQ(std::string(subject), "FALSE");
    }

    TEST(BoolMetric, custom_vals)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        BoolMetric subject("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, true, "yes", "no");
        EXPECT_EQ(subject.kind(), Metric::Kind::BOOL);

        EXPECT_EQ(bool(subject), true);
        EXPECT_EQ(std::string(subject), "yes");

        subject = false;
        EXPECT_EQ(bool(subject), false);
        EXPECT_EQ(std::string(subject), "no");
    }

    TEST(BoolMetric, not_op)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        BoolMetric subject("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, false);
        EXPECT_EQ(subject.kind(), Metric::Kind::BOOL);

        EXPECT_EQ(bool(subject), false);
        EXPECT_EQ(std::string(subject), "FALSE");

        subject = !subject;
        EXPECT_EQ(bool(subject), true);
        EXPECT_EQ(std::string(subject), "TRUE");

        subject = !subject;
        EXPECT_EQ(bool(subject), false);
        EXPECT_EQ(std::string(subject), "FALSE");
    }

}
