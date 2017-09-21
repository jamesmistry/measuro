#include <gtest/gtest.h>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(RateMetric, rate_calc)
    {
        StubTimeFunction tgt_time_f({0, 5000, 5000, 2500, 2500, 500});
        StubTimeFunction sub_time_f({0, 5000, 5000, 2500, 2500, 500});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", tgt_time_f, 1);
        RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(target, [](float val){return val*2;}, "test_rate", "test_unit", "test desc", sub_time_f);
        EXPECT_EQ(subject.kind(), Metric::Kind::RATE);

        EXPECT_FLOAT_EQ(subject.proxy_value(1.5), 3.0);

        (*target) = 0; // Baseline the clock

        (*target) = 1000;
        EXPECT_FLOAT_EQ(float(subject), 400);
        EXPECT_EQ(std::string(subject), "400.00");

        (*target) = 1500;
        EXPECT_FLOAT_EQ(float(subject), 400);
        EXPECT_EQ(std::string(subject), "400.00");

        (*target) = 1512;
        EXPECT_FLOAT_EQ(float(subject), 9.6f);
        EXPECT_EQ(std::string(subject), "9.60");

        (*target) = 1518;
        EXPECT_FLOAT_EQ(float(subject), 24);
        EXPECT_EQ(std::string(subject), "24.00");
    }

    TEST(RateMetric, no_proxy_implicit)
    {
        StubTimeFunction tgt_time_f({0, 5000, 5000, 2500, 2500, 500});
        StubTimeFunction sub_time_f({0, 5000, 5000, 2500, 2500, 500});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", tgt_time_f, 1);
        RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(target, "test_rate", "test_unit", "test desc", sub_time_f);
        EXPECT_EQ(subject.kind(), Metric::Kind::RATE);

        EXPECT_FLOAT_EQ(subject.proxy_value(1.5), 1.5);

        (*target) = 0; // Baseline the clock

        (*target) = 1000;
        EXPECT_FLOAT_EQ(float(subject), 200);
        EXPECT_EQ(std::string(subject), "200.00");

        (*target) = 1500;
        EXPECT_FLOAT_EQ(float(subject), 200);
        EXPECT_EQ(std::string(subject), "200.00");

        (*target) = 1512;
        EXPECT_FLOAT_EQ(float(subject), 4.8f);
        EXPECT_EQ(std::string(subject), "4.80");

        (*target) = 1518;
        EXPECT_FLOAT_EQ(float(subject), 12);
        EXPECT_EQ(std::string(subject), "12.00");
    }

    TEST(RateMetric, no_proxy_explicit)
    {
        StubTimeFunction tgt_time_f({0, 5000, 5000, 2500, 2500, 500});
        StubTimeFunction sub_time_f({0, 5000, 5000, 2500, 2500, 500});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", tgt_time_f, 1);
        RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(target, nullptr, "test_rate", "test_unit", "test desc", sub_time_f);
        EXPECT_EQ(subject.kind(), Metric::Kind::RATE);

        EXPECT_FLOAT_EQ(subject.proxy_value(1.5), 1.5);

        (*target) = 0; // Baseline the clock

        (*target) = 1000;
        EXPECT_FLOAT_EQ(float(subject), 200);
        EXPECT_EQ(std::string(subject), "200.00");

        (*target) = 1500;
        EXPECT_FLOAT_EQ(float(subject), 200);
        EXPECT_EQ(std::string(subject), "200.00");

        (*target) = 1512;
        EXPECT_FLOAT_EQ(float(subject), 4.8f);
        EXPECT_EQ(std::string(subject), "4.80");

        (*target) = 1518;
        EXPECT_FLOAT_EQ(float(subject), 12);
        EXPECT_EQ(std::string(subject), "12.00");
    }

    TEST(RateMetric, rate_limiter)
    {
        StubTimeFunction tgt_time_f({500, 500, 500, 500, 500, 500, 500});
        StubTimeFunction sub_time_f({500, 500, 500, 500, 500, 500, 500});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", tgt_time_f, 0,
                std::chrono::milliseconds(1000));
        RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(target, [](float val){return val*2;}, "test_rate", "test_unit", "test desc", sub_time_f);
        EXPECT_EQ(subject.kind(), Metric::Kind::RATE);

        EXPECT_FLOAT_EQ(subject.proxy_value(1.5), 3.0);

        (*target) = 0; // Baseline the clock
        (*target) = 0; // Baseline the clock
        EXPECT_FLOAT_EQ(float(subject), 0);
        EXPECT_EQ(std::string(subject), "0.00");

        (*target) = 100; // Rate limiter will prevent hook from being called
        EXPECT_FLOAT_EQ(float(subject), 0);
        EXPECT_EQ(std::string(subject), "0.00");

        (*target) = 700;
        EXPECT_FLOAT_EQ(float(subject), 1400);
        EXPECT_EQ(std::string(subject), "1400.00");

        (*target) = 850; // Rate limiter will prevent hook from being called
        EXPECT_FLOAT_EQ(float(subject), 1400);
        EXPECT_EQ(std::string(subject), "1400.00");

        (*target) = 852;
        EXPECT_FLOAT_EQ(float(subject), 304);
        EXPECT_EQ(std::string(subject), "304.00");
    }

}

