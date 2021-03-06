#ifndef STUBS_HPP_
#define STUBS_HPP_

#include <cstdint>
#include <list>
#include <chrono>
#include <iostream>
#include <functional>
#include <string>
#include <exception>
#include <iostream>

#include "measuro.hpp"

namespace measuro
{

    struct StubTimeFunction
    {
        StubTimeFunction(std::list<std::uint64_t> offsets)
        : m_offsets(offsets), m_relative_to(std::chrono::steady_clock::now())
        {
        }

        std::chrono::steady_clock::time_point operator()()
        {
            if (m_offsets.size() > 0)
            {
                auto return_val = m_relative_to += std::chrono::milliseconds(m_offsets.front());
                m_offsets.pop_front();
                return return_val;
            }
            else
            {
                return m_relative_to;
            }
        }

    private:
        std::list<std::uint64_t> m_offsets;
        std::chrono::steady_clock::time_point m_relative_to;
    };

    class StubHookMetric : public Metric
    {

    public:
        StubHookMetric(NumberMetric<Metric::Kind::UINT, std::uint64_t> & target, std::string name, std::string unit, std::string description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                std::uint64_t initial_value = 0, std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds::zero()) noexcept
        : Metric(Metric::Kind::UINT, name, unit, description, time_function, cascade_rate_limit), m_target(target), m_value(initial_value), m_hook_metric(nullptr)
        {
            target.register_hook(std::bind(&StubHookMetric::hook_handler, this, std::placeholders::_1));
        }

        operator std::string() const noexcept(false)
        {
            return std::to_string(m_value);
        }

        operator std::uint64_t() const noexcept
        {
            return m_value;
        }

        const Metric * hook_metric() const
        {
            return m_hook_metric;
        }

    private:
        void hook_handler(const Metric & metric)
        {
            m_value = std::uint64_t(metric);
            m_hook_metric = &metric;
        }

        NumberMetric<Metric::Kind::UINT, std::uint64_t> & m_target;
        std::uint64_t m_value;
        const Metric * m_hook_metric;

    };

    class StubRenderer : public Renderer
    {
    public:
        StubRenderer(bool exception_after = false)
        : Renderer(), m_exception_after(exception_after), m_render_count(0)
        {
        }

        virtual ~StubRenderer()
        {
        }

        void exception_after(bool value)
        {
            m_exception_after = value;
        }

        virtual void before() override final
        {
            m_render_count = 0;
            m_op_log.clear();
            m_op_log.push_back("before()");
        }

        virtual void after() override final
        {
            m_op_log.push_back("after()");
            if (m_exception_after)
            {
                throw std::runtime_error("after()");
            }
        }

        virtual void render(const std::shared_ptr<Metric> & metric) override final
        {
            m_op_log.push_back("render(" + metric->name() + ")");
            ++m_render_count;
        }

        bool check_log(const std::initializer_list<std::string> expected) const
        {
            bool found_expected = true;

            auto log_iter = m_op_log.cbegin();
            for (auto val : expected)
            {
                if (log_iter != m_op_log.end())
                {
                    if ((*log_iter) != val)
                    {
                        std::cout << "Expected " << val << ", got " << (*log_iter);
                        found_expected = false;
                        break;
                    }
                }
                else
                {
                    std::cout << "Expected " << val << ", got end-of-log";
                    found_expected = false;
                    break;
                }
                ++log_iter;
            }

            return found_expected;
        }

        std::size_t render_count() const
        {
            return m_render_count;
        }

        void render_count(std::size_t count)
        {
            m_render_count = count;
        }

    private:
        bool m_exception_after;
        std::size_t m_render_count;
        std::list<std::string> m_op_log;

    };

}

#endif /* STUBS_HPP_ */
