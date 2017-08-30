#include <gtest/gtest.h>

#include "Measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(RateMetric, rate_calc)
    {
        StubTimeFunction tgt_time_f({0, 5000, 5000, 2500, 2500, 500});
        StubTimeFunction sub_time_f({0, 5000, 5000, 2500, 2500, 500});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", tgt_time_f, 1);
        RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(target, 2, "test_rate", "test_unit", "test desc", sub_time_f);

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

    TEST(RateMetric, rate_limiter)
    {
        StubTimeFunction tgt_time_f({500, 500, 500, 500, 500, 500, 500});
        StubTimeFunction sub_time_f({500, 500, 500, 500, 500, 500, 500});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", tgt_time_f, 0,
                std::chrono::milliseconds(1000));
        RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(target, 2, "test_rate", "test_unit", "test desc", sub_time_f);

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

