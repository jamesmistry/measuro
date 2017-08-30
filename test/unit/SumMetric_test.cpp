#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>

#include "Measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(SumMetric, single_target)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        auto target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 10);
        SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject({target}, "test_name", "tst", "test desc 1", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(std::string(subject), "10");
        EXPECT_EQ(std::uint64_t(subject), 10);
    }

    TEST(SumMetric, multi_target)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        auto target1 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 10);
        auto target2 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 35);
        auto target3 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 100);
        SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject({target1, target2, target3}, "test_name", "tst", "test desc 1", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(std::string(subject), "145");
        EXPECT_EQ(std::uint64_t(subject), 145);
    }

    TEST(SumMetric, floating_point)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        auto target1 = std::make_shared<NumberMetric<Metric::Kind::FLOAT, float> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 10.25);
        auto target2 = std::make_shared<NumberMetric<Metric::Kind::FLOAT, float> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 35.25);
        auto target3 = std::make_shared<NumberMetric<Metric::Kind::FLOAT, float> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 100.25);
        SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > subject({target1, target2, target3}, "test_name", "tst", "test desc 1", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(std::string(subject), "145.75");
        EXPECT_FLOAT_EQ(float(subject), 145.75);
    }

    TEST(SumMetric, signed_int)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        auto target1 = std::make_shared<NumberMetric<Metric::Kind::INT, std::int64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, -100);
        auto target2 = std::make_shared<NumberMetric<Metric::Kind::INT, std::int64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 10);
        auto target3 = std::make_shared<NumberMetric<Metric::Kind::INT, std::int64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 20);
        SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > subject({target1, target2, target3}, "test_name", "tst", "test desc 1", [&dummy_clock]{return dummy_clock;});

        EXPECT_EQ(std::string(subject), "-70");
        EXPECT_FLOAT_EQ(std::int64_t(subject), -70);
    }

    TEST(SumMetric, add_target)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        auto target1 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 10);
        auto target2 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 35);
        auto target3 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 100);
        SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject("test_name", "tst", "test desc 1", [&dummy_clock]{return dummy_clock;});

        subject.add_target(target1);
        subject.add_target(target2);
        subject.add_target(target3);

        EXPECT_EQ(std::string(subject), "145");
        EXPECT_EQ(std::uint64_t(subject), 145);
    }

}
