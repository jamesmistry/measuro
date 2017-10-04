#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>
#include <sstream>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(PlainRenderer, render_single_nounit)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        std::shared_ptr<StringMetric> metric = std::make_shared<StringMetric>("test_name", "test desc 1", [&dummy_clock]{return dummy_clock;}, "init");

        std::stringstream output;

        PlainRenderer subject(output);
        subject.render(metric);
        subject.after();

        std::string expected = "test_name = init\n\n";

        EXPECT_EQ(output.str(), expected);
    }

    TEST(PlainRenderer, render_single_unit)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >("test_name", "bps", "test desc", [&dummy_clock]{return dummy_clock;});
        *metric = 1001;

        std::stringstream output;

        PlainRenderer subject(output);
        subject.render(metric);
        subject.after();

        std::string expected = "test_name = 1001 bps\n\n";

        EXPECT_EQ(output.str(), expected);
    }

    TEST(PlainRenderer, render_multi)
    {
        std::chrono::steady_clock::time_point dummy_clock;
        std::shared_ptr<StringMetric> metric1 = std::make_shared<StringMetric>("test_name1", "test desc 1", [&dummy_clock]{return dummy_clock;}, "val1");
        std::shared_ptr<StringMetric> metric2 = std::make_shared<StringMetric>("test_name2", "test desc 1", [&dummy_clock]{return dummy_clock;}, "val2");

        std::stringstream output;

        PlainRenderer subject(output);
        subject.render(metric1);
        subject.render(metric2);
        subject.after();

        std::string expected = "test_name1 = val1\ntest_name2 = val2\n\n";

        EXPECT_EQ(output.str(), expected);
    }

}
