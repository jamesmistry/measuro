/*!
 * @file system.cpp
 *
 * Copyright (c) 2017 James Mistry
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string>
#include <sstream>
#include <thread>
#include <cstddef>
#include <cassert>
#include <exception>
#include <functional>
#include <memory>
#include <atomic>
#include <vector>
#include <iostream>

#include "measuro.hpp"

using namespace measuro;

const std::size_t NUM_CREATED_METRICS = 1000;
const std::size_t NUM_THREADS = 2;
std::atomic<unsigned int> barrier_count;

class Outputter : public JsonRenderer
{
public:
    Outputter(std::stringstream & destination)
    : JsonRenderer(destination), m_destination(destination)
    {
    }

    virtual void after() noexcept(false) override
    {
        JsonRenderer::after();
        m_records.push_back(m_destination.str());
        m_destination.str("");
    }

    std::vector<std::string> m_records;

private:
    std::stringstream & m_destination;

};

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
    UintThrottle test_num_3_throt;
    std::stringstream render_out;
    Outputter renderer;

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
      test_sum(reg.create_metric(SUM::KIND, INT::KIND, "TestSum", "numbers", "Test sum metric", {test_num_1, test_num_2}, std::chrono::milliseconds::zero())),
      test_num_3_throt(reg.create_throttle(test_num_3, std::chrono::milliseconds(1000), 1000)),
      renderer(render_out)

    {
        reg.render_schedule(renderer, std::chrono::seconds(1));
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
         * Create metrics.
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
         * Perform lookups.
         */
        assert(m.reg(UINT::KIND, "TestNum3") == m.test_num_3);
        assert(m.reg(INT::KIND, "TestNum1") == m.test_num_1);
        assert(m.reg(STR::KIND, "TestStr") == m.test_str);
        assert(m.reg(FLOAT::KIND, name_prefix.str() + "float") == float_metric);
        assert(m.reg(BOOL::KIND, "TestBool") == m.test_bool);
    }

    std::this_thread::sleep_for (std::chrono::seconds(3 ));

    for (std::size_t i=0;i<1000000;++i)
    {
        auto test_num_1 = m.reg(INT::KIND, "TestNum1");
        ++(*test_num_1);

        auto test_str = m.reg(STR::KIND, "TestStr");
        (*test_str) = thread_str.str();

        auto test_float = m.reg(FLOAT::KIND, "TestFloat");
        (*test_float) = i;

        ++m.test_num_3_throt;
    }

    assert(std::uint64_t(*m.test_num_3) < NUM_THREADS * 999999);

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

    assert(std::int64_t(*m.reg(INT::KIND, "TestNum1")) == NUM_THREADS * 1000000);
    assert(float(*m.reg(FLOAT::KIND, "TestFloat")) == 999999);

    for (auto record : m.renderer.m_records)
    {
        std::cout << record << '\n';
    }

    return 0;
}
