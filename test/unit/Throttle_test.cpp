#include <gtest/gtest.h>
#include <memory>
#include <cstdint>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(Throttle, throttle_str_eq)
    {
        StubTimeFunction time_f({0, 500, 500, 500, 500, 500});
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<StringMetric>("test_name", "test desc", [&dummy_clock]{return dummy_clock;}, "a");
        Throttle<StringMetric> subject(metric, std::chrono::milliseconds(1000), 1, time_f);

        EXPECT_EQ(std::string((*metric)), "a");
        subject = "b";
        EXPECT_EQ(std::string((*metric)), "a");
        subject = "c";
        EXPECT_EQ(std::string((*metric)), "c");
        subject = "d";
        EXPECT_EQ(std::string((*metric)), "c");
        subject = "e";
        EXPECT_EQ(std::string((*metric)), "e");
        subject = "f";
        EXPECT_EQ(std::string((*metric)), "e");
    }

    TEST(Throttle, str_number_no_limit)
    {
        StubTimeFunction time_f({0, 500, 500, 500, 500, 500});
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<StringMetric>("test_name", "test desc", [&dummy_clock]{return dummy_clock;}, "a");
        Throttle<StringMetric> subject(metric, std::chrono::milliseconds::zero(), 1, time_f);

        EXPECT_EQ(std::string((*metric)), "a");
        subject = "b";
        EXPECT_EQ(std::string((*metric)), "b");
        subject = "c";
        EXPECT_EQ(std::string((*metric)), "c");
        subject = "d";
        EXPECT_EQ(std::string((*metric)), "d");
        subject = "e";
        EXPECT_EQ(std::string((*metric)), "e");
        subject = "f";
        EXPECT_EQ(std::string((*metric)), "f");
    }

    TEST(Throttle, throttle_bool_eq)
    {
        StubTimeFunction time_f({0, 500, 500, 500, 500, 500});
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<BoolMetric>("test_name", "test desc", [&dummy_clock]{return dummy_clock;}, false);
        Throttle<BoolMetric> subject(metric, std::chrono::milliseconds(1000), 1, time_f);

        EXPECT_EQ(bool((*metric)), false);
        subject = true;
        EXPECT_EQ(bool((*metric)), false);
        subject = true;
        EXPECT_EQ(bool((*metric)), true);
        subject = false;
        EXPECT_EQ(bool((*metric)), true);
        subject = false;
        EXPECT_EQ(bool((*metric)), false);
        subject = true;
        EXPECT_EQ(bool((*metric)), false);
    }

    TEST(Throttle, throttle_number_eq)
    {
        StubTimeFunction time_f({0, 500, 500, 500, 500, 500});
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        Throttle<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(metric, std::chrono::milliseconds(1000), 1, time_f);

        EXPECT_EQ(std::uint64_t((*metric)), 0);
        subject = 100;
        EXPECT_EQ(std::uint64_t((*metric)), 0);
        subject = 200;
        EXPECT_EQ(std::uint64_t((*metric)), 200);
        subject = 300;
        EXPECT_EQ(std::uint64_t((*metric)), 200);
        subject = 400;
        EXPECT_EQ(std::uint64_t((*metric)), 400);
        subject = 500;
        EXPECT_EQ(std::uint64_t((*metric)), 400);
        subject.commit();
        EXPECT_EQ(std::uint64_t((*metric)), 400);
    }

    TEST(Throttle, throttle_number_no_limit)
    {
        StubTimeFunction time_f({0, 500, 500, 500, 500, 500});
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        Throttle<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(metric, std::chrono::milliseconds::zero(), 1, time_f);

        EXPECT_EQ(std::uint64_t((*metric)), 0);
        subject = 100;
        EXPECT_EQ(std::uint64_t((*metric)), 100);
        subject = 200;
        EXPECT_EQ(std::uint64_t((*metric)), 200);
        subject = 300;
        EXPECT_EQ(std::uint64_t((*metric)), 300);
        subject = 400;
        EXPECT_EQ(std::uint64_t((*metric)), 400);
        subject = 500;
        EXPECT_EQ(std::uint64_t((*metric)), 500);
    }

    TEST(Throttle, throttle_number_plus_eq)
    {
        StubTimeFunction time_f({0, 500, 500, 500, 500, 500});
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        Throttle<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(metric, std::chrono::milliseconds(1000), 1, time_f);

        EXPECT_EQ(std::uint64_t((*metric)), 0);
        EXPECT_FALSE(subject += 100);
        EXPECT_EQ(std::uint64_t((*metric)), 0);
        EXPECT_TRUE(subject += 200);
        EXPECT_EQ(std::uint64_t((*metric)), 300);
        EXPECT_FALSE(subject += 300);
        EXPECT_EQ(std::uint64_t((*metric)), 300);
        EXPECT_TRUE(subject += 400);
        EXPECT_EQ(std::uint64_t((*metric)), 1000);
        EXPECT_FALSE(subject += 500);
        EXPECT_EQ(std::uint64_t((*metric)), 1000);
        subject.commit();
        EXPECT_EQ(std::uint64_t((*metric)), 1500);
        subject.commit();
        EXPECT_EQ(std::uint64_t((*metric)), 1500);
    }

    TEST(Throttle, throttle_number_plus_plus)
    {
        StubTimeFunction time_f({0, 500, 500, 500, 500, 500});
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        Throttle<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(metric, std::chrono::milliseconds(1000), 1, time_f);

        EXPECT_EQ(std::uint64_t((*metric)), 0);
        EXPECT_FALSE(++subject);
        EXPECT_EQ(std::uint64_t((*metric)), 0);
        EXPECT_TRUE(++subject);
        EXPECT_EQ(std::uint64_t((*metric)), 2);
        EXPECT_FALSE(++subject);
        EXPECT_EQ(std::uint64_t((*metric)), 2);
        EXPECT_TRUE(++subject);
        EXPECT_EQ(std::uint64_t((*metric)), 4);
        EXPECT_FALSE(++subject);
        EXPECT_EQ(std::uint64_t((*metric)), 4);
        subject.commit();
        EXPECT_EQ(std::uint64_t((*metric)), 5);
        subject.commit();
        EXPECT_EQ(std::uint64_t((*metric)), 5);
    }

    TEST(Throttle, throttle_number_deref_op)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        Throttle<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(metric, std::chrono::milliseconds::zero(), 1);

        EXPECT_EQ(std::uint64_t((*metric)), 0);
        (*subject) = 100;
        EXPECT_EQ(std::uint64_t((*metric)), 100);
        ++(*subject);
        EXPECT_EQ(std::uint64_t((*metric)), 101);
    }

    TEST(Throttle, throttle_nonnum_deref_op)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<StringMetric>("test_name", "test desc", [&dummy_clock]{return dummy_clock;}, "a");
        Throttle<StringMetric> subject(metric, std::chrono::milliseconds::zero(), 1);

        EXPECT_EQ(std::string((*metric)), "a");
        (*subject) = "b";
        EXPECT_EQ(std::string((*metric)), "b");
    }

    TEST(Throttle, throttle_num_op_limit_zero)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        Throttle<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(metric, std::chrono::milliseconds::zero(), 0);

        EXPECT_EQ(std::uint64_t((*metric)), 0);
        (*subject) = 100;
        EXPECT_EQ(std::uint64_t((*metric)), 100);
        ++(*subject);
        EXPECT_EQ(std::uint64_t((*metric)), 101);
    }

    TEST(Throttle, throttle_nonnum_op_limit_zero)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<StringMetric>("test_name", "test desc", [&dummy_clock]{return dummy_clock;}, "a");
        Throttle<StringMetric> subject(metric, std::chrono::milliseconds::zero(), 0);

        EXPECT_EQ(std::string((*metric)), "a");
        (*subject) = "b";
        EXPECT_EQ(std::string((*metric)), "b");
    }

    TEST(Throttle, throttle_num_op_limit_nonzero)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        Throttle<NumberMetric<Metric::Kind::UINT, std::uint64_t> > subject(metric, std::chrono::milliseconds::zero(), 5);

        EXPECT_EQ(std::uint64_t((*metric)), 0);
        (*subject) = 100;
        EXPECT_EQ(std::uint64_t((*metric)), 0);
        (*subject) = 200;
        EXPECT_EQ(std::uint64_t((*metric)), 0);
        (*subject) = 300;
        EXPECT_EQ(std::uint64_t((*metric)), 0);
        (*subject) = 400;
        EXPECT_EQ(std::uint64_t((*metric)), 0);
        (*subject) = 500;
        EXPECT_EQ(std::uint64_t((*metric)), 500);
    }

    TEST(Throttle, throttle_nonnum_op_limit_nonzero)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<StringMetric>("test_name", "test desc", [&dummy_clock]{return dummy_clock;}, "a");
        Throttle<StringMetric> subject(metric, std::chrono::milliseconds::zero(), 5);

        EXPECT_EQ(std::string((*metric)), "a");
        (*subject) = "b";
        EXPECT_EQ(std::string((*metric)), "a");
        (*subject) = "c";
        EXPECT_EQ(std::string((*metric)), "a");
        (*subject) = "d";
        EXPECT_EQ(std::string((*metric)), "a");
        (*subject) = "e";
        EXPECT_EQ(std::string((*metric)), "a");
        (*subject) = "f";
        EXPECT_EQ(std::string((*metric)), "f");
    }

}
