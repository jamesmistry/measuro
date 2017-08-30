#include <gtest/gtest.h>
#include <cstdint>

#include "Measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(Metric, kind_props)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(subject.name(), "test_name");
        EXPECT_EQ(subject.unit(), "bps");
        EXPECT_EQ(subject.description(), "test desc");
    }

    TEST(Metric, kind_name_uint)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(subject.kind(), Metric::Kind::UINT);
        EXPECT_EQ(subject.kind_name(), "UINT");
    }

    TEST(Metric, kind_name_int)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::INT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(subject.kind(), Metric::Kind::INT);
        EXPECT_EQ(subject.kind_name(), "INT");
    }

    TEST(Metric, kind_name_float)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::FLOAT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(subject.kind(), Metric::Kind::FLOAT);
        EXPECT_EQ(subject.kind_name(), "FLOAT");
    }

    TEST(Metric, kind_name_rate)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::RATE, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(subject.kind(), Metric::Kind::RATE);
        EXPECT_EQ(subject.kind_name(), "RATE");
    }

    TEST(Metric, kind_name_str)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::STR, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(subject.kind(), Metric::Kind::STR);
        EXPECT_EQ(subject.kind_name(), "STR");
    }

    TEST(Metric, kind_name_bool)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::BOOL, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(subject.kind(), Metric::Kind::BOOL);
        EXPECT_EQ(subject.kind_name(), "BOOL");
    }

    TEST(Metric, kind_name_sum)
    {

        std::chrono::steady_clock::time_point dummy_clock;

        NumberMetric<Metric::Kind::SUM, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(subject.kind(), Metric::Kind::SUM);
        EXPECT_EQ(subject.kind_name(), "SUM");
    }

    TEST(Metric, rate_limit_disabled_explicit)
    {

        std::chrono::steady_clock::time_point dummy_clock = std::chrono::steady_clock::now();

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 0, std::chrono::milliseconds::zero());
        StubHookMetric hook_metric(subject, "test_hook", "rate_unit", "test desc", [&dummy_clock]{return dummy_clock;});

        subject = 1; // First update
        EXPECT_EQ(std::uint64_t(subject), 1);
        EXPECT_EQ(std::uint64_t(hook_metric), 1);

        subject = 2; // Second update (rate limiter should be inactive)
        EXPECT_EQ(std::uint64_t(subject), 2);
        EXPECT_EQ(std::uint64_t(hook_metric), 2);
    }

    TEST(Metric, rate_limit_disabled_implicit)
    {
        std::chrono::steady_clock::time_point dummy_clock = std::chrono::steady_clock::now();

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 0);
        StubHookMetric hook_metric(subject, "test_hook", "rate_unit", "test desc", [&dummy_clock]{return dummy_clock;});

        subject = 1; // First update
        EXPECT_EQ(std::uint64_t(subject), 1);
        EXPECT_EQ(std::uint64_t(hook_metric), 1);

        subject = 2; // Second update (rate limiter should be inactive)
        EXPECT_EQ(std::uint64_t(subject), 2);
        EXPECT_EQ(std::uint64_t(hook_metric), 2);
    }

    TEST(Metric, rate_limit_enabled_deadline_passed_eq)
    {
        StubTimeFunction time_f({0, 1001, 1000});

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", time_f, 0, std::chrono::milliseconds(1000));
        StubHookMetric hook_metric(subject, "test_hook", "rate_unit", "test desc", time_f);

        subject = 1; // First update
        EXPECT_EQ(std::uint64_t(subject), 1);
        EXPECT_EQ(std::uint64_t(hook_metric), 1);

        subject = 2; // Second update (rate limiter should be inactive)
        EXPECT_EQ(std::uint64_t(subject), 2);
        EXPECT_EQ(std::uint64_t(hook_metric), 2);
    }

    TEST(Metric, rate_limit_enabled_deadline_passed_gt)
    {
        StubTimeFunction time_f({0, 1001, 1001});

        NumberMetric<Metric::Kind::UINT, std::uint64_t> subject("test_name", "bps", "test desc", time_f, 0, std::chrono::milliseconds(1000));
        StubHookMetric hook_metric(subject, "test_hook", "rate_unit", "test desc", time_f);

        subject = 1; // First update
        EXPECT_EQ(std::uint64_t(subject), 1);
        EXPECT_EQ(std::uint64_t(hook_metric), 1);

        subject = 2; // Second update (rate limiter should be inactive)
        EXPECT_EQ(std::uint64_t(subject), 2);
        EXPECT_EQ(std::uint64_t(hook_metric), 2);
    }

    TEST(Metric, rate_limit_enabled_deadline_not_passed)
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

}


