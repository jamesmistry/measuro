#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(Registry, create_uint)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(UINT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::UINT);
        EXPECT_EQ(metric->name(), "test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(std::uint64_t((*metric)), 100);
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(test_name)", "after()"}));
    }

    TEST(Registry, create_int)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(INT::KIND, "test_name", "test_unit", "test_description", -100, std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::INT);
        EXPECT_EQ(metric->name(), "test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(std::int64_t((*metric)), -100);
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(test_name)", "after()"}));
    }

    TEST(Registry, create_float)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(FLOAT::KIND, "test_name", "test_unit", "test_description", 10.75, std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::FLOAT);
        EXPECT_EQ(metric->name(), "test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_FLOAT_EQ(float((*metric)), 10.75);
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(test_name)", "after()"}));
    }

    TEST(Registry, create_rate_uint)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target = subject.create_metric(UINT::KIND, "a_test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto metric = subject.create_metric(RATE::KIND, UINT::KIND, target, "b_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});

        EXPECT_EQ(metric->kind(), Metric::Kind::RATE);
        EXPECT_EQ(metric->name(), "b_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_FLOAT_EQ(metric->proxy_value(1.0), 2.0);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "after()"}));
    }

    TEST(Registry, create_rate_int)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target = subject.create_metric(INT::KIND, "a_test_name", "test_unit", "test_description", -100, std::chrono::milliseconds(2000));
        auto metric = subject.create_metric(RATE::KIND, INT::KIND, target, "b_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});

        EXPECT_EQ(metric->kind(), Metric::Kind::RATE);
        EXPECT_EQ(metric->name(), "b_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_FLOAT_EQ(metric->proxy_value(1.0), 2.0);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "after()"}));
    }

    TEST(Registry, create_rate_float)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target = subject.create_metric(FLOAT::KIND, "a_test_name", "test_unit", "test_description", 55.75, std::chrono::milliseconds(2000));
        auto metric = subject.create_metric(RATE::KIND, FLOAT::KIND, target, "b_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});

        EXPECT_EQ(metric->kind(), Metric::Kind::RATE);
        EXPECT_EQ(metric->name(), "b_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_FLOAT_EQ(metric->proxy_value(1.0), 2.0);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "after()"}));
    }

    TEST(Registry, create_rate_sum_uint)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target1 = subject.create_metric(UINT::KIND, "a_test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto target2 = subject.create_metric(UINT::KIND, "b_test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto target3 = subject.create_metric(SUM::KIND, UINT::KIND, "c_test_name", "test_unit", "test_description", {target1, target2}, std::chrono::milliseconds(2000));
        auto metric = subject.create_metric(RATE::KIND, SUM::KIND, UINT::KIND, target3, "d_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});

        EXPECT_EQ(metric->kind(), Metric::Kind::RATE);
        EXPECT_EQ(metric->name(), "d_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_FLOAT_EQ(metric->proxy_value(1.0), 2.0);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "render(c_test_name)", "render(d_test_name)", "after()"}));
    }

    TEST(Registry, create_rate_sum_int)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target1 = subject.create_metric(INT::KIND, "a_test_name", "test_unit", "test_description", -100, std::chrono::milliseconds(2000));
        auto target2 = subject.create_metric(INT::KIND, "b_test_name", "test_unit", "test_description", -100, std::chrono::milliseconds(2000));
        auto target3 = subject.create_metric(SUM::KIND, INT::KIND, "c_test_name", "test_unit", "test_description", {target1, target2}, std::chrono::milliseconds(2000));
        auto metric = subject.create_metric(RATE::KIND, SUM::KIND, INT::KIND, target3, "d_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});

        EXPECT_EQ(metric->kind(), Metric::Kind::RATE);
        EXPECT_EQ(metric->name(), "d_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_FLOAT_EQ(metric->proxy_value(1.0), 2.0);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "render(c_test_name)", "render(d_test_name)", "after()"}));
    }

    TEST(Registry, create_rate_sum_float)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target1 = subject.create_metric(FLOAT::KIND, "a_test_name", "test_unit", "test_description", 100.75, std::chrono::milliseconds(2000));
        auto target2 = subject.create_metric(FLOAT::KIND, "b_test_name", "test_unit", "test_description", 100.75, std::chrono::milliseconds(2000));
        auto target3 = subject.create_metric(SUM::KIND, FLOAT::KIND, "c_test_name", "test_unit", "test_description", {target1, target2}, std::chrono::milliseconds(2000));
        auto metric = subject.create_metric(RATE::KIND, SUM::KIND, FLOAT::KIND, target3, "d_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});

        EXPECT_EQ(metric->kind(), Metric::Kind::RATE);
        EXPECT_EQ(metric->name(), "d_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_FLOAT_EQ(metric->proxy_value(1.0), 2.0);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "render(c_test_name)", "render(d_test_name)", "after()"}));
    }

    TEST(Registry, create_sum_uint)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target1 = subject.create_metric(UINT::KIND, "a_test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto target2 = subject.create_metric(UINT::KIND, "b_test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto metric = subject.create_metric(SUM::KIND, UINT::KIND, "c_test_name", "test_unit", "test_description", {target1, target2}, std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::SUM);
        EXPECT_EQ(metric->name(), "c_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_EQ(std::uint64_t((*metric)), 200);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "render(c_test_name)", "after()"}));
    }

    TEST(Registry, create_sum_int)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target1 = subject.create_metric(INT::KIND, "a_test_name", "test_unit", "test_description", -100, std::chrono::milliseconds(2000));
        auto target2 = subject.create_metric(INT::KIND, "b_test_name", "test_unit", "test_description", 300, std::chrono::milliseconds(2000));
        auto metric = subject.create_metric(SUM::KIND, INT::KIND, "c_test_name", "test_unit", "test_description", {target1, target2}, std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::SUM);
        EXPECT_EQ(metric->name(), "c_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_EQ(std::int64_t((*metric)), 200);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "render(c_test_name)", "after()"}));
    }

    TEST(Registry, create_sum_float)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target1 = subject.create_metric(FLOAT::KIND, "a_test_name", "test_unit", "test_description", 1.5, std::chrono::milliseconds(2000));
        auto target2 = subject.create_metric(FLOAT::KIND, "b_test_name", "test_unit", "test_description", 2.5, std::chrono::milliseconds(2000));
        auto metric = subject.create_metric(SUM::KIND, FLOAT::KIND, "c_test_name", "test_unit", "test_description", {target1, target2}, std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::SUM);
        EXPECT_EQ(metric->name(), "c_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_EQ(float((*metric)), 4.0);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "render(c_test_name)", "after()"}));
    }

    TEST(Registry, create_sum_rate_uint)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target1 = subject.create_metric(UINT::KIND, "a_test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto target2 = subject.create_metric(UINT::KIND, "b_test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto target3 = subject.create_metric(RATE::KIND, UINT::KIND, target1, "c_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});
        auto target4 = subject.create_metric(RATE::KIND, UINT::KIND, target2, "d_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});
        auto metric = subject.create_metric(SUM::KIND, RATE::KIND, UINT::KIND, "e_test_name", "test_unit", "test_description", {target3, target4}, std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::SUM);
        EXPECT_EQ(metric->name(), "e_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_EQ(metric->target_count(), 2);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "render(c_test_name)", "render(d_test_name)", "render(e_test_name)", "after()"}));
    }

    TEST(Registry, create_sum_rate_int)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target1 = subject.create_metric(INT::KIND, "a_test_name", "test_unit", "test_description", -100, std::chrono::milliseconds(2000));
        auto target2 = subject.create_metric(INT::KIND, "b_test_name", "test_unit", "test_description", 300, std::chrono::milliseconds(2000));
        auto target3 = subject.create_metric(RATE::KIND, INT::KIND, target1, "c_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});
        auto target4 = subject.create_metric(RATE::KIND, INT::KIND, target2, "d_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});
        auto metric = subject.create_metric(SUM::KIND, RATE::KIND, INT::KIND, "e_test_name", "test_unit", "test_description", {target3, target4}, std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::SUM);
        EXPECT_EQ(metric->name(), "e_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_EQ(metric->target_count(), 2);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "render(c_test_name)", "render(d_test_name)", "render(e_test_name)", "after()"}));
    }

    TEST(Registry, create_sum_rate_float)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto target1 = subject.create_metric(FLOAT::KIND, "a_test_name", "test_unit", "test_description", 1.75, std::chrono::milliseconds(2000));
        auto target2 = subject.create_metric(FLOAT::KIND, "b_test_name", "test_unit", "test_description", 2.5, std::chrono::milliseconds(2000));
        auto target3 = subject.create_metric(RATE::KIND, FLOAT::KIND, target1, "c_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});
        auto target4 = subject.create_metric(RATE::KIND, FLOAT::KIND, target2, "d_test_name", "test_unit", "test_description", std::chrono::milliseconds(2000), [](float val){return val*2;});
        auto metric = subject.create_metric(SUM::KIND, RATE::KIND, FLOAT::KIND, "e_test_name", "test_unit", "test_description", {target3, target4}, std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::SUM);
        EXPECT_EQ(metric->name(), "e_test_name");
        EXPECT_EQ(metric->unit(), "test_unit");
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));
        EXPECT_EQ(metric->target_count(), 2);

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(a_test_name)", "render(b_test_name)", "render(c_test_name)", "render(d_test_name)", "render(e_test_name)", "after()"}));
    }

    TEST(Registry, create_str)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(STR::KIND, "test_name", "test_description", "val", std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::STR);
        EXPECT_EQ(metric->name(), "test_name");
        EXPECT_EQ(metric->unit().size(), 0);
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(std::string((*metric)), "val");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(test_name)", "after()"}));
    }

    TEST(Registry, create_bool)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(BOOL::KIND, "test_name", "test_description", true, "yes", "no", std::chrono::milliseconds(2000));

        EXPECT_EQ(metric->kind(), Metric::Kind::BOOL);
        EXPECT_EQ(metric->name(), "test_name");
        EXPECT_EQ(metric->unit().size(), 0);
        EXPECT_EQ(metric->description(), "test_description");
        EXPECT_EQ(bool((*metric)), true);
        EXPECT_EQ(std::string((*metric)), "yes");
        (*metric) = false;
        EXPECT_EQ(std::string((*metric)), "no");
        EXPECT_EQ(metric->cascade_rate_limit(), std::chrono::milliseconds(2000));

        StubRenderer rndr;
        subject.render(rndr);
        EXPECT_TRUE(rndr.check_log({"before()", "render(test_name)", "after()"}));
    }

    TEST(Registry, duplicate_metric)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric1 = subject.create_metric(UINT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        EXPECT_THROW(auto metric2 = subject.create_metric(UINT::KIND, "test_name", "test_unit", "test_description", 200, std::chrono::milliseconds(2000)), MetricNameError);
    }

    TEST(Registry, lookup_uint)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric1 = subject.create_metric(UINT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto metric2 = subject.create_metric(UINT::KIND, "test_name2", "test_unit", "test_description", 200, std::chrono::milliseconds(2000));

        EXPECT_EQ(subject(UINT::KIND, "test_name"), metric1);
        EXPECT_EQ(subject(UINT::KIND, "test_name2"), metric2);
    }

    TEST(Registry, lookup_int)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric1 = subject.create_metric(INT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto metric2 = subject.create_metric(INT::KIND, "test_name2", "test_unit", "test_description", 200, std::chrono::milliseconds(2000));

        EXPECT_EQ(subject(INT::KIND, "test_name"), metric1);
        EXPECT_EQ(subject(INT::KIND, "test_name2"), metric2);
    }

    TEST(Registry, lookup_float)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric1 = subject.create_metric(FLOAT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto metric2 = subject.create_metric(FLOAT::KIND, "test_name2", "test_unit", "test_description", 200, std::chrono::milliseconds(2000));

        EXPECT_EQ(subject(FLOAT::KIND, "test_name"), metric1);
        EXPECT_EQ(subject(FLOAT::KIND, "test_name2"), metric2);
    }

    TEST(Registry, lookup_str)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric1 = subject.create_metric(STR::KIND, "test_name", "test_description", "t1", std::chrono::milliseconds(2000));
        auto metric2 = subject.create_metric(STR::KIND, "test_name2", "test_description", "t2", std::chrono::milliseconds(2000));

        EXPECT_EQ(subject(STR::KIND, "test_name"), metric1);
        EXPECT_EQ(subject(STR::KIND, "test_name2"), metric2);
    }

    TEST(Registry, lookup_bool)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric1 = subject.create_metric(BOOL::KIND, "test_name", "test_description", true);
        auto metric2 = subject.create_metric(BOOL::KIND, "test_name2", "test_description", false);

        EXPECT_EQ(subject(BOOL::KIND, "test_name"), metric1);
        EXPECT_EQ(subject(BOOL::KIND, "test_name2"), metric2);
    }

    TEST(Registry, lookup_bad_type)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(UINT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));

        EXPECT_THROW(subject(INT::KIND, "test_name"), MetricTypeError);
    }

    TEST(Registry, render_exception)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(UINT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));

        StubRenderer rndr(true);

        subject.render(rndr);
        EXPECT_TRUE(rndr.suppressed_exception());
        EXPECT_TRUE(rndr.check_log({"before()", "render(test_name)", "after()"}));

        rndr.exception_after(false);
        subject.render(rndr);
        EXPECT_FALSE(rndr.suppressed_exception());
        EXPECT_TRUE(rndr.check_log({"before()", "render(test_name)", "after()"}));
    }

    TEST(Registry, render_prefix)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric1 = subject.create_metric(UINT::KIND, "module1.test_a", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto metric2 = subject.create_metric(UINT::KIND, "module1.test_b", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        auto metric3 = subject.create_metric(UINT::KIND, "module2.test_c", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));

        StubRenderer rndr;

        subject.render(rndr, "module1");
        EXPECT_FALSE(rndr.suppressed_exception());
        EXPECT_TRUE(rndr.check_log({"before()", "render(module1.test_a)", "render(module1.test_b)", "after()"}));
    }

    TEST(Registry, schedule_render)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(UINT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));

        StubRenderer rndr;
        subject.render_schedule(rndr, std::chrono::seconds(1));
        std::this_thread::sleep_for(std::chrono::seconds(2));

        EXPECT_GT(rndr.render_count(), 0);

        subject.cancel_render_schedule();
        rndr.render_count(0);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        EXPECT_EQ(rndr.render_count(), 0);
    }

    TEST(Registry, create_uint_throttle)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(UINT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        UintThrottle result = subject.create_throttle(metric, std::chrono::milliseconds(1234), 101);

        EXPECT_EQ(result.time_limit(), std::chrono::milliseconds(1234));
        EXPECT_EQ(result.op_limit(), 101);
    }

    TEST(Registry, create_int_throttle)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(INT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        IntThrottle result = subject.create_throttle(metric, std::chrono::milliseconds(1234), 101);

        EXPECT_EQ(result.time_limit(), std::chrono::milliseconds(1234));
        EXPECT_EQ(result.op_limit(), 101);
    }

    TEST(Registry, create_float_throttle)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(FLOAT::KIND, "test_name", "test_unit", "test_description", 100, std::chrono::milliseconds(2000));
        FloatThrottle result = subject.create_throttle(metric, std::chrono::milliseconds(1234), 101);

        EXPECT_EQ(result.time_limit(), std::chrono::milliseconds(1234));
        EXPECT_EQ(result.op_limit(), 101);
    }

    TEST(Registry, create_str_throttle)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(STR::KIND, "test_name", "test_description", "val", std::chrono::milliseconds(2000));
        StringThrottle result = subject.create_throttle(metric, std::chrono::milliseconds(1234), 101);

        EXPECT_EQ(result.time_limit(), std::chrono::milliseconds(1234));
        EXPECT_EQ(result.op_limit(), 101);
    }

    TEST(Registry, create_bool_throttle)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        Registry subject([&dummy_clock]{return dummy_clock;});
        auto metric = subject.create_metric(BOOL::KIND, "test_name", "test_description", true, "yes", "no", std::chrono::milliseconds(2000));
        BoolThrottle result = subject.create_throttle(metric, std::chrono::milliseconds(1234), 101);

        EXPECT_EQ(result.time_limit(), std::chrono::milliseconds(1234));
        EXPECT_EQ(result.op_limit(), 101);
    }
}
