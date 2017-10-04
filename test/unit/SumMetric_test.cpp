#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(SumMetric, single_target)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        auto target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 10);
        SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject({target}, "test_name", "tst", "test desc 1", [&dummy_clock]{return dummy_clock;});
        EXPECT_EQ(subject.kind(), Metric::Kind::SUM);

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
        EXPECT_EQ(subject.kind(), Metric::Kind::SUM);

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
        EXPECT_EQ(subject.kind(), Metric::Kind::SUM);

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
        EXPECT_EQ(subject.kind(), Metric::Kind::SUM);

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
        EXPECT_EQ(subject.kind(), Metric::Kind::SUM);

        subject.add_target(target1);
        subject.add_target(target2);
        subject.add_target(target3);

        EXPECT_EQ(std::string(subject), "145");
        EXPECT_EQ(std::uint64_t(subject), 145);
    }

    TEST(SumMetric, sum_of_rate)
    {
        StubTimeFunction time_f({0, 0, 2500, 5000, 1600, 1500});
        std::chrono::steady_clock::time_point dummy_clock;
        auto rate_target1 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 0);
        auto rate_target2 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 0);

        auto rate1 = std::make_shared<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(rate_target1, [](float val){return val;}, "test_rate", "test_unit", "test desc", time_f);
        auto rate2 = std::make_shared<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(rate_target2, [](float val){return val;}, "test_rate", "test_unit", "test desc", time_f);

        SumMetric<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > subject("test_name", "tst", "test desc 1", [&dummy_clock]{return dummy_clock;});
        EXPECT_EQ(subject.kind(), Metric::Kind::SUM);

        subject.add_target(rate1);
        subject.add_target(rate2);

        *rate_target1 = 0;
        *rate_target2 = 0;
        rate1->calculate();
        rate2->calculate();
        subject.calculate();
        EXPECT_EQ(std::string(subject), "0.00");
        EXPECT_FLOAT_EQ(float(subject), 0);

        *rate_target1 = 250;
        *rate_target2 = 150;
        rate1->calculate();
        rate2->calculate();
        subject.calculate();
        EXPECT_EQ(std::string(subject), "160.00");
        EXPECT_FLOAT_EQ(float(subject), 160);

        *rate_target1 = 500;
        *rate_target2 = 852;
        rate1->calculate();
        rate2->calculate();
        subject.calculate();
        EXPECT_EQ(std::string(subject), "190.40");
        EXPECT_FLOAT_EQ(float(subject), 190.4);

        *rate_target1 = 550;
        *rate_target2 = 927;
        rate1->calculate();
        rate2->calculate();
        subject.calculate();
        EXPECT_EQ(std::string(subject), "78.12");
        EXPECT_FLOAT_EQ(float(subject), 78.125);

        *rate_target1 = 700;
        *rate_target2 = 1092;
        rate1->calculate();
        rate2->calculate();
        subject.calculate();
        EXPECT_EQ(std::string(subject), "210.00");
        EXPECT_FLOAT_EQ(float(subject), 210);
    }

}
