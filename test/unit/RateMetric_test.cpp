#include <gtest/gtest.h>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(RateMetric, rate_calc)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        StubTimeFunction time_f({0, 5000, 5000, 2500, 2500, 500, 500});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 1);
        RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(target, [](float val){return val*2;}, "test_rate", "test_unit", "test desc", time_f);
        EXPECT_EQ(subject.kind(), Metric::Kind::RATE);

        EXPECT_FLOAT_EQ(subject.proxy_value(1.5), 3.0);

        (*target) = 0; // Baseline the clock
        subject.calculate();

        (*target) = 1000;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 400);
        EXPECT_EQ(std::string(subject), "400.00");

        (*target) = 1500;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 400);
        EXPECT_EQ(std::string(subject), "400.00");

        (*target) = 1512;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 9.6f);
        EXPECT_EQ(std::string(subject), "9.60");

        (*target) = 1518;
        subject.calculate(); // Only 0.5 seconds have passed
        subject.calculate();

        EXPECT_FLOAT_EQ(float(subject), 12);
        EXPECT_EQ(std::string(subject), "12.00");
    }

    TEST(RateMetric, rate_of_sum_calc)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        StubTimeFunction time_f({0, 0, 1000, 5000, 1000, 1500});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > uint_target_1 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("counter_1", "bps", "", [&dummy_clock]{return dummy_clock;}, 0);
        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > uint_target_2 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("counter_2", "bps", "", [&dummy_clock]{return dummy_clock;}, 0);
        std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > sum_target =
                std::make_shared<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >("counter_sum", "bps", "", time_f, std::chrono::milliseconds(1000));

        sum_target->add_target(uint_target_1);
        sum_target->add_target(uint_target_2);

        RateMetric<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > subject(sum_target, [](float val){return val*2;}, "test_rate", "test_unit", "test desc", time_f);
        EXPECT_EQ(subject.kind(), Metric::Kind::RATE);

        EXPECT_FLOAT_EQ(subject.proxy_value(1.5), 3.0);

        (*uint_target_1) = 0; // Baseline the clock
        (*uint_target_2) = 0; // Baseline the clock
        sum_target->calculate();
        subject.calculate();

        EXPECT_EQ(std::uint64_t(*uint_target_1), 0);
        EXPECT_EQ(std::uint64_t(*uint_target_2), 0);
        EXPECT_EQ(std::uint64_t(*sum_target), 0);

        (*uint_target_1) = 750;
        (*uint_target_2) = 250;
        sum_target->calculate();
        subject.calculate();
        EXPECT_EQ(std::uint64_t(*sum_target), 1000);

        EXPECT_FLOAT_EQ(float(subject), 2000);
        EXPECT_EQ(std::string(subject), "2000.00");

        (*uint_target_1) = 750;
        (*uint_target_2) = 750;
        sum_target->calculate();
        subject.calculate();
        EXPECT_EQ(std::uint64_t(*sum_target), 1500);

        EXPECT_FLOAT_EQ(float(subject), 200);
        EXPECT_EQ(std::string(subject), "200.00");

        (*uint_target_1) = 1500;
        (*uint_target_2) = 12;
        sum_target->calculate();
        subject.calculate();
        EXPECT_EQ(std::uint64_t(*sum_target), 1512);

        EXPECT_FLOAT_EQ(float(subject), 24);
        EXPECT_EQ(std::string(subject), "24.00");

        (*uint_target_1) = 1500;
        (*uint_target_2) = 18;
        sum_target->calculate();
        subject.calculate();
        EXPECT_EQ(std::uint64_t(*sum_target), 1518);

        EXPECT_FLOAT_EQ(float(subject), 8);
        EXPECT_EQ(std::string(subject), "8.00");
    }

    TEST(RateMetric, no_proxy_implicit)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        StubTimeFunction time_f({0, 0, 5000, 2500, 2500, 1000});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 1);
        RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(target, "test_rate", "test_unit", "test desc", time_f);
        EXPECT_EQ(subject.kind(), Metric::Kind::RATE);

        EXPECT_FLOAT_EQ(subject.proxy_value(1.5), 1.5);

        (*target) = 0;
        subject.calculate();

        (*target) = 1000;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 200);
        EXPECT_EQ(std::string(subject), "200.00");

        (*target) = 1500;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 200);
        EXPECT_EQ(std::string(subject), "200.00");

        (*target) = 1512;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 4.8f);
        EXPECT_EQ(std::string(subject), "4.80");

        (*target) = 1518;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 6);
        EXPECT_EQ(std::string(subject), "6.00");
    }

    TEST(RateMetric, no_proxy_explicit)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        StubTimeFunction time_f({0, 0, 5000, 2500, 2500, 1000});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 1);
        RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(target, nullptr, "test_rate", "test_unit", "test desc", time_f);
        EXPECT_EQ(subject.kind(), Metric::Kind::RATE);

        EXPECT_FLOAT_EQ(subject.proxy_value(1.5), 1.5);

        (*target) = 0;
        subject.calculate();

        (*target) = 1000;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 200);
        EXPECT_EQ(std::string(subject), "200.00");

        (*target) = 1500;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 200);
        EXPECT_EQ(std::string(subject), "200.00");

        (*target) = 1512;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 4.8f);
        EXPECT_EQ(std::string(subject), "4.80");

        (*target) = 1518;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 6);
        EXPECT_EQ(std::string(subject), "6.00");
    }

    TEST(RateMetric, rate_limiter)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        StubTimeFunction time_f({0, 0, 500, 500, 500, 500, 500});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 0,
                std::chrono::milliseconds(1000));
        RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(target, [](float val){return val*2;}, "test_rate", "test_unit", "test desc", time_f);
        EXPECT_EQ(subject.kind(), Metric::Kind::RATE);

        EXPECT_FLOAT_EQ(subject.proxy_value(1.5), 3.0);

        (*target) = 0; // Baseline the clock
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 0);
        EXPECT_EQ(std::string(subject), "0.00");

        (*target) = 100; // Rate limiter will prevent calculation
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 0);
        EXPECT_EQ(std::string(subject), "0.00");

        (*target) = 700;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 1400);
        EXPECT_EQ(std::string(subject), "1400.00");

        (*target) = 850; // Rate limiter will prevent hook from being called
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 1400);
        EXPECT_EQ(std::string(subject), "1400.00");

        (*target) = 852;
        subject.calculate();
        EXPECT_FLOAT_EQ(float(subject), 304);
        EXPECT_EQ(std::string(subject), "304.00");
    }

}

