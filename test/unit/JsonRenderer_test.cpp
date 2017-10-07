#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>
#include <sstream>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(JsonRenderer, render_str)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        std::shared_ptr<StringMetric> metric = std::make_shared<StringMetric>("test_name", "test desc", [&dummy_clock]{return dummy_clock;}, "init");

        std::stringstream output;

        JsonRenderer subject(output);
        subject.before();
        subject.render(metric);
        subject.after();

        std::string expected = "{\"test_name\":{\"value\":\"init\",\"unit\":\"\",\"kind\":\"STR\",\"description\":\"test desc\"}}";

        EXPECT_EQ(output.str(), expected);
    }

    TEST(JsonRenderer, render_bool)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        std::shared_ptr<BoolMetric> metric = std::make_shared<BoolMetric>("test_name", "test desc", [&dummy_clock]{return dummy_clock;}, true, "yes", "no");

        std::stringstream output;

        JsonRenderer subject(output);
        subject.before();
        subject.render(metric);
        subject.after();

        std::string expected = "{\"test_name\":{\"value\":true,\"unit\":\"\",\"kind\":\"BOOL\",\"description\":\"test desc\"}}";

        EXPECT_EQ(output.str(), expected);
    }

    TEST(JsonRenderer, render_uint)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 1024);

        std::stringstream output;

        JsonRenderer subject(output);
        subject.before();
        subject.render(metric);
        subject.after();

        std::string expected = "{\"test_name\":{\"value\":1024,\"unit\":\"bps\",\"kind\":\"UINT\",\"description\":\"test desc\"}}";

        EXPECT_EQ(output.str(), expected);
    }

    TEST(JsonRenderer, render_int)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        std::shared_ptr<NumberMetric<Metric::Kind::INT, std::int64_t> > metric = std::make_shared<NumberMetric<Metric::Kind::INT, std::int64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, -1024);

        std::stringstream output;

        JsonRenderer subject(output);
        subject.before();
        subject.render(metric);
        subject.after();

        std::string expected = "{\"test_name\":{\"value\":-1024,\"unit\":\"bps\",\"kind\":\"INT\",\"description\":\"test desc\"}}";

        EXPECT_EQ(output.str(), expected);
    }

    TEST(JsonRenderer, render_float)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        std::shared_ptr<NumberMetric<Metric::Kind::FLOAT, float> > metric = std::make_shared<NumberMetric<Metric::Kind::FLOAT, float> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;}, 100.75);

        std::stringstream output;

        JsonRenderer subject(output);
        subject.before();
        subject.render(metric);
        subject.after();

        std::string expected = "{\"test_name\":{\"value\":100.75,\"unit\":\"bps\",\"kind\":\"FLOAT\",\"description\":\"test desc\"}}";

        EXPECT_EQ(output.str(), expected);
    }

    TEST(JsonRenderer, render_rate)
    {
        StubTimeFunction time_f({0, 5000, 5000});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", time_f, 1);
        std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > metric = std::make_shared<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(target, [](float val){return val*2;}, "test_rate", "test_unit", "test desc", time_f);

        *target = 0; // Baseline the clock
        metric->calculate();

        *target = 1000;
        metric->calculate();

        EXPECT_FLOAT_EQ(float((*metric)), 400);
        EXPECT_EQ(std::string((*metric)), "400.00");

        std::stringstream output;

        JsonRenderer subject(output);
        subject.before();
        subject.render(metric);
        subject.after();

        std::string expected = "{\"test_rate\":{\"value\":400.00,\"unit\":\"test_unit\",\"kind\":\"RATE\",\"description\":\"test desc\"}}";

        EXPECT_EQ(output.str(), expected);
    }

    TEST(JsonRenderer, render_sum)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        auto target1 = std::make_shared<NumberMetric<Metric::Kind::FLOAT, float> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 10.25);
        auto target2 = std::make_shared<NumberMetric<Metric::Kind::FLOAT, float> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 35.25);
        auto target3 = std::make_shared<NumberMetric<Metric::Kind::FLOAT, float> >("test_tgt", "tst", "test desc 2", [&dummy_clock]{return dummy_clock;}, 100.25);
        auto metric = std::make_shared<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});

        metric->add_target(target1);
        metric->add_target(target2);
        metric->add_target(target3);

        EXPECT_EQ(std::string((*metric)), "145.75");
        EXPECT_FLOAT_EQ(float((*metric)), 145.75);

        std::stringstream output;

        JsonRenderer subject(output);
        subject.before();
        subject.render(metric);
        subject.after();

        std::string expected = "{\"test_name\":{\"value\":145.75,\"unit\":\"bps\",\"kind\":\"SUM\",\"description\":\"test desc\"}}";

        EXPECT_EQ(output.str(), expected);
    }

    TEST(JsonRenderer, render_multi)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        std::shared_ptr<StringMetric> metric1 = std::make_shared<StringMetric>("test_name1", "test desc 1", [&dummy_clock]{return dummy_clock;}, "val1");
        std::shared_ptr<StringMetric> metric2 = std::make_shared<StringMetric>("test_name2", "test desc 2", [&dummy_clock]{return dummy_clock;}, "val2");

        std::stringstream output;

        JsonRenderer subject(output);
        subject.before();
        subject.render(metric1);
        subject.render(metric2);
        subject.after();

        std::string expected = "{\"test_name1\":{\"value\":\"val1\",\"unit\":\"\",\"kind\":\"STR\",\"description\":\"test desc 1\"},\"test_name2\":{\"value\":\"val2\",\"unit\":\"\",\"kind\":\"STR\",\"description\":\"test desc 2\"}}";

        EXPECT_EQ(output.str(), expected);
    }

}
