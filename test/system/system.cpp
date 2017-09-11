#include <string>
#include <sstream>
#include <thread>
#include <cstddef>
#include <cassert>
#include <exception>
#include <functional>
#include <memory>
#include <gtest/gtest.h>
#include <atomic>

#include "Measuro.hpp"

using namespace measuro;

const std::size_t NUM_CREATED_METRICS = 1000;
const std::size_t NUM_THREADS = 2;
std::atomic<unsigned int> barrier_count;

struct Metrics
{
    Registry reg;
    IntHandle test_num_1;
    IntHandle test_num_2;
    UintHandle test_num_3;
    RateOfIntHandle test_rate;
    StringHandle test_str;
    BoolHandle test_bool;
    FloatHandle test_float;
    SumOfIntHandle test_sum;

    Metrics()
    : test_num_1(reg.create_metric(INT::KIND, "TestNum1", "integer(s)",
            "Test number metric 1", 0, std::chrono::milliseconds::zero())),
      test_num_2(reg.create_metric(INT::KIND, "TestNum2", "integer(s)",
                "Test number metric 2", 0, std::chrono::milliseconds::zero())),
      test_num_3(reg.create_metric(UINT::KIND, "TestNum3", "integer(s)",
                "Test number metric 3", 0, std::chrono::milliseconds::zero())),
      test_rate(reg.create_metric(RATE::KIND, INT::KIND, test_num_1, "TestNumRate",
              "integers", "Rate of test number 1", std::chrono::milliseconds::zero())),
      test_str(reg.create_metric(STR::KIND, "TestStr",
              "Test string metric", "val", std::chrono::milliseconds::zero())),
      test_bool(reg.create_metric(BOOL::KIND, "TestBool", "Test bool metric", false,
              "TRUE", "FALSE", std::chrono::milliseconds::zero())),
      test_float(reg.create_metric(FLOAT::KIND, "TestFloat", "Test float metric", "floats",
              0.0f, std::chrono::milliseconds::zero())),
      test_sum(reg.create_metric(SUM::KIND, INT::KIND, "TestSum", "numbers", "Test sum metric", {test_num_1, test_num_2}, std::chrono::milliseconds::zero()))
    {
    }
};

void work_thread(Metrics & m, std::size_t thread_index)
{
    std::stringstream thread_str;
    thread_str << "thread" << thread_index;

    ++barrier_count;
    while(barrier_count < NUM_THREADS);

    for (std::size_t i=0;i<NUM_CREATED_METRICS;++i)
    {
        /*
         * Create some metrics.
         */

        std::stringstream name_prefix, description;
        name_prefix << "TestMetric" << i << '_' << thread_index << '_';
        description << "Metric " << i << ',' << thread_index;

        m.reg.create_metric(UINT::KIND, name_prefix.str() + "uint", "unsigned integer(s)", description.str(), 0, std::chrono::milliseconds::zero());
        auto int_metric = m.reg.create_metric(INT::KIND, name_prefix.str() + "int", "integer(s)", description.str(), 0, std::chrono::milliseconds::zero());
        auto float_metric = m.reg.create_metric(FLOAT::KIND, name_prefix.str() + "float", "float(s)", description.str(), 0, std::chrono::milliseconds::zero());

        auto sum_metric = m.reg.create_metric(SUM::KIND, INT::KIND, name_prefix.str() + "sum_int", "sum", description.str(), {int_metric, m.test_num_1}, std::chrono::milliseconds::zero());
        m.reg.create_metric(RATE::KIND, SUM::KIND, INT::KIND, sum_metric, name_prefix.str() + "rate_sum_int", "integers", "Rate of test number 1", std::chrono::milliseconds::zero());

        /*
         * Perform some lookups.
         */
        EXPECT_NO_THROW(EXPECT_EQ(m.reg(UINT::KIND, "TestNum3"), m.test_num_3));
        EXPECT_NO_THROW(EXPECT_EQ(m.reg(INT::KIND, "TestNum1"), m.test_num_1));
        EXPECT_NO_THROW(EXPECT_EQ(m.reg(STR::KIND, "TestStr"), m.test_str));
        EXPECT_NO_THROW(EXPECT_EQ(m.reg(FLOAT::KIND, name_prefix.str() + "float"), float_metric));
        EXPECT_NO_THROW(EXPECT_EQ(m.reg(BOOL::KIND, "TestBool"), m.test_bool));
    }

    for (std::size_t i=0;i<1000000;++i)
    {
        auto test_num_1 = m.reg(INT::KIND, "TestNum1");
        ++(*test_num_1);

        auto test_str = m.reg(STR::KIND, "TestStr");
        (*test_str) = thread_str.str();

        auto test_float = m.reg(FLOAT::KIND, "TestFloat");
        (*test_float) = i;
    }

    if (thread_index == 0)
    {
        (*m.test_str) = thread_str.str();
    }

}

int main(int argc, char * argv[])
{
    Metrics m;

    std::vector<std::shared_ptr<std::thread> > threads;

    barrier_count = 0;

    for (std::size_t i=0;i<NUM_THREADS;++i)
    {
        auto t = std::make_shared<std::thread>(&work_thread, std::ref(m), i);
        threads.push_back(t);
    }

    for (auto thread : threads)
    {
        thread->join();
    }

    EXPECT_EQ(std::int64_t(*m.reg(INT::KIND, "TestNum1")), NUM_THREADS * 1000000);
    EXPECT_EQ(float(*m.reg(FLOAT::KIND, "TestFloat")), 999999);

    return 0;
}
