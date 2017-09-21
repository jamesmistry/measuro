#include <gtest/gtest.h>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(NumberMetric, constructability_uint)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        EXPECT_EQ(subject.kind(), Metric::Kind::UINT);

        EXPECT_EQ(subject.name(), "test_name");
        EXPECT_EQ(subject.unit(), "bps");
        EXPECT_EQ(subject.description(), "test desc");
    }

    TEST(NumberMetric, constructability_int)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::INT, std::int64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        EXPECT_EQ(subject.kind(), Metric::Kind::INT);

        EXPECT_EQ(subject.name(), "test_name");
        EXPECT_EQ(subject.unit(), "bps");
        EXPECT_EQ(subject.description(), "test desc");
    }

    TEST(NumberMetric, constructability_float)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::FLOAT, float> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        EXPECT_EQ(subject.kind(), Metric::Kind::FLOAT);

        EXPECT_EQ(subject.name(), "test_name");
        EXPECT_EQ(subject.unit(), "bps");
        EXPECT_EQ(subject.description(), "test desc");
    }

    TEST(NumberMetric, initial_value)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 1024);
        EXPECT_EQ(subject.kind(), Metric::Kind::UINT);

        EXPECT_EQ(std::uint64_t(subject), 1024);
    }

    TEST(NumberMetric, deadline)
    {
        StubTimeFunction time_f({0, 1001, 999});

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", time_f, 0, std::chrono::milliseconds(1000));
        StubHookMetric hook_metric(subject, "test_hook", "rate_unit", "test desc", time_f);

        subject = 1; // First update
        EXPECT_EQ(std::uint64_t(subject), 1);
        EXPECT_EQ(std::uint64_t(hook_metric), 1);

        subject = 2; // Second update (rate limiter should be active)
        EXPECT_EQ(std::uint64_t(subject), 2);
        EXPECT_EQ(std::uint64_t(hook_metric), 1);
    }

    TEST(NumberMetric, str_conv_uint)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 1024);

        EXPECT_EQ(std::string(subject), "1024");
    }

    TEST(NumberMetric, str_conv_int)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::INT, std::int64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, -1024);

        EXPECT_EQ(std::string(subject), "-1024");
    }

    TEST(NumberMetric, str_conv_float)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::FLOAT, float> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 100.879);

        EXPECT_EQ(std::string(subject), "100.88");
    }

    TEST(NumberMetric, op_uint)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 1024);

        EXPECT_EQ(std::uint64_t(subject), 1024);
    }

    TEST(NumberMetric, op_int)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::INT, std::int64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, -1024);

        EXPECT_EQ(std::int64_t(subject), -1024);
    }

    TEST(NumberMetric, op_float)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::FLOAT, float> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 100.879);

        EXPECT_FLOAT_EQ(float(subject), 100.879);
    }

    TEST(NumberMetric, op_updates)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 100);

        subject = 101;
        EXPECT_EQ(std::uint64_t(subject), 101);

        EXPECT_EQ(subject++, 101);
        EXPECT_EQ(std::uint64_t(subject), 102);

        EXPECT_EQ(++subject, 103);
        EXPECT_EQ(std::uint64_t(subject), 103);

        EXPECT_EQ(subject--, 103);
        EXPECT_EQ(std::uint64_t(subject), 102);

        EXPECT_EQ(--subject, 101);
        EXPECT_EQ(std::uint64_t(subject), 101);

        EXPECT_EQ(subject += 1, 102);
        EXPECT_EQ(std::uint64_t(subject), 102);

        EXPECT_EQ(subject -= 1, 101);
        EXPECT_EQ(std::uint64_t(subject), 101);
    }

}
