/*!
 * @file Measuro.hpp
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

#ifndef MEASURO_HPP
#define MEASURO_HPP

#include <cstddef>
#include <string>
#include <sstream>
#include <cstdint>
#include <chrono>
#include <atomic>
#include <functional>
#include <map>
#include <vector>
#include <utility>
#include <memory>
#include <exception>
#include <ostream>
#include <mutex>
#include <type_traits>
#include <cmath>
#include <iostream>

namespace measuro
{

    static_assert(std::is_trivially_copyable<std::chrono::steady_clock::time_point>::value, "time_point must be trivially copyable (for use with std::atomic)");

    class MeasuroError : public std::runtime_error
    {
    public:
        MeasuroError(const std::string description) : std::runtime_error(description.c_str())
        {
        }

        virtual ~MeasuroError()
        {
        }
    };

    class MetricNameError : public MeasuroError
    {
    public:
        MetricNameError(const std::string description) : MeasuroError(description)
        {
        }
    };

    class MetricTypeError : public MeasuroError
    {
    public:
        MetricTypeError(const std::string description) : MeasuroError(description)
        {
        }
    };

    /*!
    *
    *
    * @param major
    * @param minor
    * @param release
    */
    inline static void version(unsigned int & major, unsigned int & minor, unsigned int & release) noexcept
    {
        major = @LIB_VERSION_MAJOR@;
        minor = @LIB_VERSION_MINOR@;
        release = @LIB_VERSION_REL@;
    }

    /*!
    *
    *
    * @param version_str
    */
    inline static void version_text(std::string & version_str) noexcept(false)
    {
        version_str = "@LIB_VERSION_MAJOR@.@LIB_VERSION_MINOR@-@LIB_VERSION_REL@";
    }

    inline static void copyright_text(std::string & copyright_str) noexcept(false)
    {
        copyright_str = "Measuro version @LIB_VERSION_MAJOR@.@LIB_VERSION_MINOR@-@LIB_VERSION_REL@\n\nCopyright (c) 2017, James Mistry. Released under the MIT licence - for details see https://github.com/jamesmistry/measuro";
    }

    enum class UINT { KIND };
    enum class INT { KIND };
    enum class FLOAT { KIND };
    enum class RATE { KIND };
    enum class STR { KIND };
    enum class BOOL { KIND };
    enum class SUM { KIND };

    /*!
     * @class Metric
     *
     *
     */
    class Metric
    {
    public:
        enum class Kind { UINT = 0, INT = 1, FLOAT = 2, RATE = 3, STR = 4, BOOL = 5, SUM = 6 };

        Metric(const Kind kind, const std::string name, const std::string unit, const std::string description,
                std::function<std::chrono::steady_clock::time_point ()> time_function,
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds::zero()) noexcept
        : m_kind(kind), m_name(name), m_unit(unit), m_description(description),
          m_last_hook_update(time_function()), m_time_function(time_function), m_cascade_limit(cascade_rate_limit), m_has_hooks(false)
        {
        }

        virtual ~Metric()
        {
        }

        std::string name() const noexcept(false)
        {
            return m_name;
        }

        std::string unit() const noexcept(false)
        {
            return m_unit;
        }

        std::string description() const noexcept(false)
        {
            return m_description;
        }

        Kind kind() const noexcept
        {
            return m_kind;
        }

        std::string kind_name() const noexcept(false)
        {
            return kind_name(m_kind);
        }

        std::string kind_name(Kind kind) const noexcept(false)
        {
            switch(kind)
            {
            case Kind::UINT:
                return "UINT";
            case Kind::INT:
                return "INT";
            case Kind::FLOAT:
                return "FLOAT";
            case Kind::RATE:
                return "RATE";
            case Kind::STR:
                return "STR";
            case Kind::BOOL:
                return "BOOL";
            case Kind::SUM:
                return "SUM";
            default:
                return "";
            }
        }

        virtual operator std::string() const = 0;

        void register_hook(std::function<void (std::chrono::steady_clock::time_point update_time)> registrant)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_metric_mutex);

            m_hooks.push_back(registrant);
            m_has_hooks = true;
        }

    protected:

        virtual void hook_handler(const std::chrono::steady_clock::time_point update_time)
        {
            (void)(update_time);
            return;
        }

        void update(std::function<void()> update_logic) noexcept(false)
        {
            auto now = m_time_function();

            /*
             * update_logic() must always be called outside an m_metric_mutex critical
             * section so that the hook logic is free to acquire the mutex if it needs
             * to.
             */
            update_logic();

            if ((m_has_hooks) && ((m_cascade_limit == std::chrono::milliseconds::zero()) ||
                    (now - m_last_hook_update.load()) >= m_cascade_limit))
            {
                // TODO: Replace with std::scoped_lock on migration to C++17
                std::lock_guard<std::mutex> lock(m_metric_mutex);

                for (auto hook : m_hooks)
                {
                    hook(now);
                }

                m_last_hook_update = now;
            }
        }

        mutable std::mutex m_metric_mutex;

    private:
        Kind m_kind;
        std::string m_name;
        std::string m_unit;
        std::string m_description;
        std::atomic<std::chrono::steady_clock::time_point> m_last_hook_update;
        std::function<std::chrono::steady_clock::time_point ()> m_time_function;
        std::chrono::milliseconds m_cascade_limit;
        std::vector<std::function<void (std::chrono::steady_clock::time_point update_time)> > m_hooks;
        std::atomic<bool> m_has_hooks;

    };

    template<typename T>
    class DiscoverableNativeType
    {
    public:
        typedef T NativeType;
    };

    template<Metric::Kind K, typename T>
    class NumberMetric : public Metric, public DiscoverableNativeType<T>
    {
    public:
        NumberMetric(const std::string name, const std::string unit, const std::string description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const T initial_value = 0, const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds::zero()) noexcept
        : Metric(K, name, unit, description, time_function, cascade_rate_limit), m_value(initial_value)
        {
        }

        NumberMetric(const NumberMetric &) = delete;
        NumberMetric(NumberMetric &&) = delete;
        NumberMetric & operator=(const NumberMetric &) = delete;
        NumberMetric & operator=(NumberMetric &&) = delete;

        operator std::string() const noexcept(false) override final
        {
            std::stringstream formatter;

            formatter << std::fixed << std::setprecision(2) << m_value;

            return formatter.str();
        }

        explicit operator T() const noexcept
        {
            return m_value;
        }

        void operator=(T rhs) noexcept(false)
        {
            update([this, rhs]()
            {
                m_value = rhs;
            });
        }

        T operator++() noexcept
        {
            T new_val = m_value;

            update([this, & new_val]()
            {
                new_val = ++m_value;
            });

            return new_val;
        }

        T operator++(int) noexcept
        {
            T old_val = m_value;

            update([this, & old_val]()
            {
                old_val = m_value++;
            });

            return old_val;
        }

        T operator--() noexcept
        {
            T new_val = m_value;

            update([this, & new_val]()
            {
                new_val = --m_value;
            });

            return new_val;
        }

        T operator--(int) noexcept
        {
            T old_val = m_value;

            update([this, & old_val]()
            {
                old_val = m_value--;
            });

            return old_val;
        }

        T operator+=(const T & rhs) noexcept
        {
            T new_val = m_value;

            update([this, rhs, & new_val]()
            {
                new_val = m_value += rhs;
            });

            return new_val;
        }

        T operator-=(const T & rhs) noexcept
        {
            T new_val = m_value;

            update([this, rhs, & new_val]()
            {
                new_val = m_value -= rhs;
            });

            return new_val;
        }

    private:
        std::atomic<T> m_value;

    };

    template<typename D>
    class RateMetric : public Metric, public DiscoverableNativeType<float>
    {
    public:
        RateMetric(std::shared_ptr<D> & distance, const float multiplier, const std::string name, const std::string unit, const std::string description,
                std::function<std::chrono::steady_clock::time_point ()> time_function, const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds::zero()) noexcept
        : Metric(Metric::Kind::RATE, name, unit, description, time_function, cascade_rate_limit), m_distance(distance), m_multiplier(multiplier), m_last_distance(0),
          m_value(0.0f)
        {
            m_distance->register_hook(std::bind(&RateMetric::hook_handler, this, std::placeholders::_1));
        }

        RateMetric(const RateMetric &) = delete;
        RateMetric(RateMetric &&) = delete;
        RateMetric & operator=(const RateMetric &) = delete;
        RateMetric & operator=(RateMetric &&) = delete;

        operator std::string() const noexcept(false) override final
        {
            std::stringstream formatter;
            formatter << std::fixed << std::setprecision(2) << m_value;
            return formatter.str();
        }

        explicit operator float() const noexcept
        {
            return m_value;
        }

    private:
        void hook_handler(const std::chrono::steady_clock::time_point update_time) override final
        {
            update([this, & update_time]()
            {
                auto time_elapsed = float(std::chrono::duration_cast<std::chrono::milliseconds>(update_time - m_last_hook_time).count()) / 1000;
                auto distance_travelled = float((typename D::NativeType)(*m_distance));

                if (time_elapsed != 0.0) // Avoid div by zero
                {
                    m_value = ((distance_travelled - m_last_distance) / time_elapsed) * m_multiplier;
                }

                m_last_distance = distance_travelled;
                m_last_hook_time = update_time;
            });
        }

        std::shared_ptr<D> m_distance;
        float m_multiplier;
        float m_last_distance;
        std::atomic<float> m_value;
        std::chrono::steady_clock::time_point m_last_hook_time;

    };

    template<typename D>
    class SumMetric : public Metric, public DiscoverableNativeType<typename D::NativeType>
    {
    public:
        SumMetric(std::initializer_list<std::shared_ptr<D> > targets, const std::string name, const std::string unit, const std::string description,
                std::function<std::chrono::steady_clock::time_point ()> time_function, const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds::zero())
        : Metric(Metric::Kind::SUM, name, unit, description, time_function, cascade_rate_limit)
        {
            for (auto target : targets)
            {
                m_targets.push_back(target);
            }
        }

        SumMetric(const std::string name, const std::string unit, const std::string description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds::zero())
        : Metric(Metric::Kind::SUM, name, unit, description, time_function, cascade_rate_limit)
        {
        }

        SumMetric(const SumMetric &) = delete;
        SumMetric(SumMetric &&) = delete;
        SumMetric & operator=(const SumMetric &) = delete;
        SumMetric & operator=(SumMetric &&) = delete;

        void add_target(std::shared_ptr<D> & target)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_metric_mutex);

            m_targets.push_back(target);
        }

        explicit operator typename D::NativeType() const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_metric_mutex);

            typename D::NativeType total = 0;

            for (std::size_t index=0;index<m_targets.size();++index)
            {
                total += (const typename D::NativeType)(*m_targets[index]);
            }

            return total;
        }

        operator std::string() const noexcept(false) override final
        {
            auto total = (operator typename D::NativeType)();

            std::stringstream formatter;
            formatter << std::fixed << std::setprecision(2) << total;

            return formatter.str();
        }

    private:
        std::vector<std::shared_ptr<D> > m_targets;

    };

    class StringMetric : public Metric, public DiscoverableNativeType<std::string>
    {
    public:
        StringMetric(const std::string name, const std::string unit, const std::string description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const std::string initial_value, const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds::zero())
        : Metric(Metric::Kind::STR, name, unit, description, time_function, cascade_rate_limit), m_value(initial_value)
        {
        }

        StringMetric(const StringMetric &) = delete;
        StringMetric(StringMetric &&) = delete;
        StringMetric & operator=(const StringMetric &) = delete;
        StringMetric & operator=(StringMetric &&) = delete;

        operator std::string() const noexcept(false) override final
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_metric_mutex);

            return m_value;
        }

        void operator=(std::string rhs) noexcept(false)
        {
            update([this, rhs]()
            {
                // TODO: Replace with std::scoped_lock on migration to C++17
                std::lock_guard<std::mutex> lock(m_metric_mutex);

                m_value = rhs;
            });
        }

    private:
        std::string m_value;

    };

    class BoolMetric : public Metric, public DiscoverableNativeType<bool>
    {
    public:
        BoolMetric(const std::string name, const std::string unit, const std::string description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const bool initial_value, const std::string true_rep = "TRUE", const std::string false_rep = "FALSE", const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds::zero())
        : Metric(Metric::Kind::STR, name, unit, description, time_function, cascade_rate_limit), m_value(initial_value), m_true_rep(true_rep),
          m_false_rep(false_rep)
        {
        }

        BoolMetric(const BoolMetric &) = delete;
        BoolMetric(BoolMetric &&) = delete;
        BoolMetric & operator=(const BoolMetric &) = delete;
        BoolMetric & operator=(BoolMetric &&) = delete;

        operator std::string() const noexcept override final
        {
            if (m_value)
            {
                return m_true_rep;
            }
            else
            {
                return m_false_rep;
            }
        }

        explicit operator bool() const noexcept
        {
            return m_value;
        }

        void operator=(bool rhs) noexcept(false)
        {
            update([this, rhs]()
            {
                m_value = rhs;
            });
        }

        bool operator!() noexcept(false)
        {
            return !m_value;
        }

    private:
        std::atomic<bool> m_value;
        const std::string m_true_rep;
        const std::string m_false_rep;

    };

    class Renderer
    {
    public:
        Renderer() : m_suppressed_exception(false)
        {
        }

        virtual ~Renderer()
        {
        }

        virtual void before()
        {
            return;
        }

        virtual void after()
        {
            return;
        }

        virtual void render(const std::shared_ptr<Metric> & metric)
        {
            (void)(metric);
            return;
        }

        void suppressed_exception(bool state) noexcept
        {
            m_suppressed_exception = state;
        }

        bool suppressed_exception() const noexcept
        {
            return m_suppressed_exception;
        }

    private:
        bool m_suppressed_exception;

    };

    class PlainRenderer : Renderer
    {
    public:
        PlainRenderer(std::ostream & destination) : m_destination(destination)
        {
        }

        virtual ~PlainRenderer()
        {
        }

        virtual void after() noexcept(false) override
        {
            m_destination << std::endl;
        }

        void render(const std::shared_ptr<Metric> & metric) noexcept(false) override final
        {
            m_destination << metric->name() << " = " << metric << "\n";
        }

    private:
        std::ostream & m_destination;

    };

    class JsonRenderer : Renderer
    {
    public:
        JsonRenderer(std::ostream & destination) : m_destination(destination)
        {
        }

        virtual ~JsonRenderer()
        {
        }

        virtual void after() noexcept(false) override
        {
            m_destination.flush();
        }

        void render(const std::shared_ptr<Metric> & metric) noexcept(false) override final
        {

        }

    private:
        std::ostream & m_destination;

    };

    class Registry
    {
    public:
        Registry(std::function<std::chrono::steady_clock::time_point ()> time_function = []{return std::chrono::steady_clock::now();}) noexcept
        : m_time_function(time_function)
        {
        }

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > create_metric(const UINT, const std::string name, const std::string unit, const std::string description,
                const std::uint64_t initial_value = 0, const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >(name, unit, description, m_time_function, initial_value, cascade_rate_limit);
            register_metric<NumberMetric<Metric::Kind::UINT, std::uint64_t> >(name, metric, m_uint_metrics);
            return metric;
        }

        std::shared_ptr<NumberMetric<Metric::Kind::INT, std::int64_t> > create_metric(const INT, const std::string name, const std::string unit, const std::string description,
                const std::uint64_t initial_value = 0, const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<NumberMetric<Metric::Kind::INT, std::int64_t> >(name, unit, description, m_time_function, initial_value, cascade_rate_limit);
            register_metric<NumberMetric<Metric::Kind::INT, std::int64_t> >(name, metric, m_int_metrics);
            return metric;
        }

        std::shared_ptr<NumberMetric<Metric::Kind::FLOAT, float> > create_metric(const FLOAT, const std::string name, const std::string unit, const std::string description,
                const float initial_value = 0, const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<NumberMetric<Metric::Kind::FLOAT, float> >(name, unit, description, m_time_function, initial_value, cascade_rate_limit);
            register_metric<NumberMetric<Metric::Kind::FLOAT, float> >(name, metric, m_float_metrics);
            return metric;
        }

        std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > create_metric(const RATE, const UINT,
                std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > & distance, const float multiplier, const std::string name, const std::string unit, const std::string description,
                std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(distance, multiplier, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(name, metric, m_uint_rate_metrics);
            return metric;
        }

        std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > create_metric(const RATE, const INT,
                std::shared_ptr<NumberMetric<Metric::Kind::INT, std::int64_t> > & distance, const float multiplier, const std::string name, const std::string unit, const std::string description,
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > >(distance, multiplier, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > >(name, metric, m_int_rate_metrics);
            return metric;
        }

        std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > > create_metric(const RATE, const FLOAT,
                std::shared_ptr<NumberMetric<Metric::Kind::FLOAT, float> > & distance, const float multiplier, const std::string name, const std::string unit, const std::string description,
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > >(distance, multiplier, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > >(name, metric, m_float_rate_metrics);
            return metric;
        }

        std::shared_ptr<RateMetric<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > > create_metric(const RATE, const SUM, const UINT,
                std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > & distance, const float multiplier, const std::string name, const std::string unit, const std::string description,
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<RateMetric<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > >(distance, multiplier, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<RateMetric<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > >(name, metric, m_sum_uint_rate_metrics);
            return metric;
        }

        std::shared_ptr<RateMetric<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > > create_metric(const RATE, const SUM, const INT,
                std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > & distance, const float multiplier, const std::string name, const std::string unit, const std::string description,
                std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<RateMetric<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > >(distance, multiplier, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<RateMetric<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > >(name, metric, m_sum_int_rate_metrics);
            return metric;
        }

        std::shared_ptr<RateMetric<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > > > create_metric(const RATE, const SUM, const FLOAT,
                std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > > & distance, const float multiplier, const std::string name, const std::string unit, const std::string description,
                std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<RateMetric<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > > >(distance, multiplier, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<RateMetric<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > > >(name, metric, m_sum_float_rate_metrics);
            return metric;
        }

        std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > create_metric(const SUM, const UINT,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > targets = {},
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(targets, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(name, metric, m_uint_sum_metrics);
            return metric;
        }

        std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > create_metric(const SUM, const INT,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<std::shared_ptr<NumberMetric<Metric::Kind::INT, std::int64_t> > > targets = {},
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > >(targets, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > >(name, metric, m_int_sum_metrics);
            return metric;
        }

        std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > > create_metric(const SUM, const FLOAT,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<std::shared_ptr<NumberMetric<Metric::Kind::FLOAT, float> > > targets = {},
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > >(targets, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > >(name, metric, m_float_sum_metrics);
            return metric;
        }

        std::shared_ptr<SumMetric<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > > create_metric(const SUM, const RATE, const UINT,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > > targets = {},
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<SumMetric<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > >(targets, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<SumMetric<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > >(name, metric, m_rate_uint_sum_metrics);
            return metric;
        }

        std::shared_ptr<SumMetric<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > > create_metric(const SUM, const RATE, const INT,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > > targets = {},
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<SumMetric<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > >(targets, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<SumMetric<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > >(name, metric, m_rate_int_sum_metrics);
            return metric;
        }

        std::shared_ptr<SumMetric<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > > > create_metric(const SUM, const RATE, const FLOAT,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > > > targets = {},
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<SumMetric<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > > >(targets, name, unit, description, m_time_function, cascade_rate_limit);
            register_metric<SumMetric<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > > >(name, metric, m_rate_float_sum_metrics);
            return metric;
        }

        std::shared_ptr<StringMetric> create_metric(const STR, const std::string name, const std::string unit, const std::string description,
                const std::string initial_value = "", const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<StringMetric>(name, unit, description, m_time_function, initial_value, cascade_rate_limit);
            register_metric<StringMetric>(name, metric, m_str_metrics);
            return metric;
        }

        std::shared_ptr<BoolMetric> create_metric(const BOOL, const std::string name, const std::string unit, const std::string description,
                const bool initial_value = false, const std::string true_rep = "TRUE", const std::string false_rep = "FALSE",
                const std::chrono::milliseconds cascade_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            auto metric = std::make_shared<BoolMetric>(name, unit, description, m_time_function, initial_value, true_rep, false_rep, cascade_rate_limit);
            register_metric<BoolMetric>(name, metric, m_bool_metrics);
            return metric;
        }

        std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > operator()(const UINT, const std::string name) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name, Metric::Kind::UINT);
            return m_uint_metrics[entry->second.second];
        }

        std::shared_ptr<NumberMetric<Metric::Kind::INT, std::int64_t> > operator()(const INT, const std::string name) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name, Metric::Kind::INT);
            return m_int_metrics[entry->second.second];
        }

        std::shared_ptr<NumberMetric<Metric::Kind::FLOAT, float> > operator()(const FLOAT, const std::string name) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name, Metric::Kind::FLOAT);
            return m_float_metrics[entry->second.second];
        }

        std::shared_ptr<StringMetric> operator()(const STR, const std::string name) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name, Metric::Kind::STR);
            return m_str_metrics[entry->second.second];
        }

        std::shared_ptr<BoolMetric> operator()(const BOOL, const std::string name) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name, Metric::Kind::BOOL);
            return m_bool_metrics[entry->second.second];
        }

        void render(Renderer & renderer, const std::string name_prefix = "") const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            RendererContext render_ctx(renderer);

            for (auto metric : m_metrics)
            {
                if ((name_prefix.length() == 0) || (metric.first.find(name_prefix) == 0))
                {
                    renderer.render(metric.second.first);
                }
            }
        }

    private:
        class RendererContext
        {
        public:
            RendererContext(Renderer & renderer) : m_renderer(renderer)
            {
                m_renderer.suppressed_exception(false);
                m_renderer.before();
            }

            ~RendererContext()
            {
                // Prevent exceptions in Renderer::after() from leaving the destructor
                try
                {
                    m_renderer.after();
                }
                catch (...)
                {
                    m_renderer.suppressed_exception(true);
                    return;
                }
            }

        private:
            Renderer & m_renderer;
        };

        std::map<std::string, std::pair<std::shared_ptr<Metric>, std::uint64_t> >::const_iterator
        lookup(const std::string & name, const Metric::Kind expected_kind) const noexcept(false)
        {
            auto entry = m_metrics.find(name);
            if (entry == m_metrics.end())
            {
                throw MetricNameError("No metric exists called \"" + name + "\"");
            }
            else if (entry->second.first->kind() != expected_kind)
            {
                throw MetricTypeError("The metric called \"" + name + "\" is of an unexpected kind: actual kind is " +
                        entry->second.first->kind_name() + "; expected kind is " + entry->second.first->kind_name(expected_kind));
            }

            return entry;
        }

        template<typename T>
        void register_metric(const std::string & metric_name, std::shared_ptr<T > & metric, std::vector<std::shared_ptr<T > > & metric_registry) noexcept(false)
        {
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            if (metric_name.length() == 0)
            {
                // Anonymous metric - not associated with a name
                metric_registry.push_back(metric);
            }
            else
            {
                if (m_metrics.find(metric_name) != m_metrics.end())
                {
                    throw MetricNameError("A metric already exists with the name \"" + metric_name + "\"");
                }

                metric_registry.push_back(metric);
                m_metrics[metric_name] = std::pair<std::shared_ptr<Metric>, std::uint64_t>(metric, metric_registry.size() - 1);
            }
        }

        mutable std::mutex m_registry_mutex;
        std::function<std::chrono::steady_clock::time_point ()> m_time_function;
        std::map<std::string, std::pair<std::shared_ptr<Metric>, std::uint64_t> > m_metrics;

        std::vector<std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > m_uint_metrics;
        std::vector<std::shared_ptr<NumberMetric<Metric::Kind::INT, std::int64_t> > > m_int_metrics;
        std::vector<std::shared_ptr<NumberMetric<Metric::Kind::FLOAT, float> > > m_float_metrics;

        std::vector<std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > > m_uint_rate_metrics;
        std::vector<std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > > m_int_rate_metrics;
        std::vector<std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > > > m_float_rate_metrics;
        std::vector<std::shared_ptr<RateMetric<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > > > m_sum_uint_rate_metrics;
        std::vector<std::shared_ptr<RateMetric<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > > > m_sum_int_rate_metrics;
        std::vector<std::shared_ptr<RateMetric<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > > > > m_sum_float_rate_metrics;

        std::vector<std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > > m_uint_sum_metrics;
        std::vector<std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > > m_int_sum_metrics;
        std::vector<std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > > > m_float_sum_metrics;
        std::vector<std::shared_ptr<SumMetric<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > > > m_rate_uint_sum_metrics;
        std::vector<std::shared_ptr<SumMetric<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > > > m_rate_int_sum_metrics;
        std::vector<std::shared_ptr<SumMetric<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > > > > m_rate_float_sum_metrics;

        std::vector<std::shared_ptr<StringMetric> > m_str_metrics;
        std::vector<std::shared_ptr<BoolMetric> > m_bool_metrics;

    };
}

#endif
