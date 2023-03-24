#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>
#include <sstream>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{
    TEST(PrometheusRenderer, basic_output_structure)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric_1 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("example_count1", "ItEm(s)", "An example count metric", [&dummy_clock]{return dummy_clock;});
        *metric_1 = 100;

        auto metric_2 = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("example_count2", "ItEm(s)", "An example count metric", [&dummy_clock]{return dummy_clock;});
        *metric_2 = 200;

        std::stringstream test_output;
        PrometheusRenderer subject(test_output, [](){return 1234567;}, "testapp");
        subject.before();
        subject.render(metric_1);
        subject.render(metric_2);
        subject.after();

        EXPECT_EQ(test_output.str(), "# HELP testapp::example_count1_items An example count metric\ntestapp::example_count1_items 100 1234567\n# HELP testapp::example_count2_items An example count metric\ntestapp::example_count2_items 200 1234567\n");
    }

    TEST(PrometheusRenderer, dirty_help_text)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("example_count", "ItEm(s)", "Line 1\nLine2\nLine \\3\n", [&dummy_clock]{return dummy_clock;});
        *metric = 100;

        std::stringstream test_output;
        PrometheusRenderer subject(test_output, [](){return 1234567;}, "testapp");
        subject.before();
        subject.render(metric);
        subject.after();

        EXPECT_EQ(test_output.str(), "# HELP testapp::example_count_items Line 1\\nLine2\\nLine \\\\3\\n\ntestapp::example_count_items 100 1234567\n");
    }

    TEST(PrometheusRenderer, dirty_name_valid)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("example_count$", "ItEm(s)", "An example count metric", [&dummy_clock]{return dummy_clock;});
        *metric = 100;

        std::stringstream test_output;
        PrometheusRenderer subject(test_output, [](){return 1234567;}, "testapp");
        subject.before();
        subject.render(metric);
        subject.after();

        EXPECT_EQ(test_output.str(), "# HELP testapp::example_count_items An example count metric\ntestapp::example_count_items 100 1234567\n");
    }

    TEST(PrometheusRenderer, dirty_name_invalid)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("1example_count", "ItEm(s)", "An example count metric", [&dummy_clock]{return dummy_clock;});
        *metric = 100;

        std::stringstream test_output;
        PrometheusRenderer subject(test_output, [](){return 1234567;}, "testapp");
        subject.before();
        subject.render(metric);
        subject.after();

        EXPECT_EQ(test_output.str(), "\n");
    }

    TEST(PrometheusRenderer, exclude_str)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        std::shared_ptr<StringMetric> metric = std::make_shared<StringMetric>("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, "init");

        std::stringstream test_output;
        PrometheusRenderer subject(test_output, [](){return 1234567;}, "testapp");
        subject.before();
        subject.render(metric);
        subject.after();

        EXPECT_EQ(test_output.str(), "\n");
    }

    TEST(PrometheusRenderer, render_bool)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        std::shared_ptr<BoolMetric> metric_1 = std::make_shared<BoolMetric>("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, true);
        std::shared_ptr<BoolMetric> metric_2 = std::make_shared<BoolMetric>("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, false);

        {
            std::stringstream test_output;
            PrometheusRenderer subject(test_output, [](){return 1234567;}, "testapp");
            subject.before();
            subject.render(metric_1);
            subject.after();

            EXPECT_EQ(test_output.str(), "# HELP testapp::test_name test desc 1\ntestapp::test_name 1 1234567\n");
        }

        {
            std::stringstream test_output;
            PrometheusRenderer subject(test_output, [](){return 1234567;}, "testapp");
            subject.before();
            subject.render(metric_2);
            subject.after();

            EXPECT_EQ(test_output.str(), "# HELP testapp::test_name test desc 1\ntestapp::test_name 0 1234567\n");
        }
    }

    TEST(PrometheusRenderer, render_float)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::FLOAT, float> >("example_count", "", "An example float metric", [&dummy_clock]{return dummy_clock;});
        *metric = 1.5;

        std::stringstream test_output;
        PrometheusRenderer subject(test_output, [](){return 1234567;}, "testapp");
        subject.before();
        subject.render(metric);
        subject.after();

        EXPECT_EQ(test_output.str(), "# HELP testapp::example_count An example float metric\ntestapp::example_count 1.50 1234567\n");
    }

    TEST(PrometheusRenderer, render_rate)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        StubTimeFunction time_f({0, 5000, 5000});

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > target = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", time_f, 1);
        std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > metric = std::make_shared<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(target, [](float val){return val*2;}, "test_rate", "test_unit", "test desc", time_f);

        *target = 0; // Baseline the clock
        metric->calculate();

        *target = 1000;
        metric->calculate();

        EXPECT_FLOAT_EQ(float((*metric)), 400);
        EXPECT_EQ(std::string((*metric)), "400.00");

        std::stringstream test_output;
        PrometheusRenderer subject(test_output, [](){return 1234567;}, "testapp");
        subject.before();
        subject.render(metric);
        subject.after();

        EXPECT_EQ(test_output.str(), "# HELP testapp::test_rate_testunit test desc\ntestapp::test_rate_testunit 400.00 1234567\n");
    }

    TEST(PrometheusRenderer, render_sum)
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

        PrometheusRenderer subject(output, [](){return 1234567;}, "testapp");
        subject.before();
        subject.render(metric);
        subject.after();

        EXPECT_EQ(output.str(), "# HELP testapp::test_name_bps test desc\ntestapp::test_name_bps 145.75 1234567\n");
    }

    TEST(PrometheusRenderer, no_unit)
    {
        std::chrono::steady_clock::time_point dummy_clock;

        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("example_count", "", "An example count metric", [&dummy_clock]{return dummy_clock;});
        *metric = 100;

        std::stringstream test_output;
        PrometheusRenderer subject(test_output, [](){return 1234567;}, "testapp");
        subject.before();
        subject.render(metric);
        subject.after();

        EXPECT_EQ(test_output.str(), "# HELP testapp::example_count An example count metric\ntestapp::example_count 100 1234567\n");
    }

    TEST(PrometheusRenderer, invalid_app_name)
    {
        std::stringstream test_output;
        EXPECT_THROW(PrometheusRenderer subject = PrometheusRenderer(test_output, [](){return 1234567;}, "invalid¬name"), RenderError);
    }
}
