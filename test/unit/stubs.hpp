#ifndef STUBS_HPP_
#define STUBS_HPP_

#include <cstdint>
#include <list>
#include <chrono>
#include <iostream>
#include <functional>

#include "Measuro.hpp"

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
        : Metric(Metric::Kind::UINT, name, unit, description, time_function, cascade_rate_limit), m_target(target), m_value(initial_value)
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

        std::chrono::steady_clock::time_point hook_time() const
        {
            return m_hook_time;
        }

    private:
        void hook_handler(std::chrono::steady_clock::time_point update_time)
        {
            m_value = std::uint64_t(m_target);
            m_hook_time = update_time;
        }

        NumberMetric<Metric::Kind::UINT, std::uint64_t> & m_target;
        std::uint64_t m_value;
        std::chrono::steady_clock::time_point m_hook_time;

    };

}

#endif /* STUBS_HPP_ */
