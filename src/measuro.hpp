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
#include <limits>
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
#include <iomanip>
#include <thread>
#include <condition_variable>

namespace measuro
{

    static_assert(std::is_trivially_copyable<std::chrono::steady_clock::time_point>::value,
            "time_point must be trivially copyable (for use with std::atomic)");

    /*!
     * @class MeasuroError
     *
     * @brief Measuro exception base class. All Measuro exceptions inherit from this class.
     *
     * @remarks thread-safe
     */
    class MeasuroError : public std::runtime_error
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    description    Description of the error
         */
        MeasuroError(const std::string description) : std::runtime_error(description.c_str())
        {
        }

        virtual ~MeasuroError() noexcept
        {
        }
    };

    /*!
     * @class MetricNameError
     *
     * @brief Describes an error that occurs because of an incompatible metric name.
     *
     * @remarks thread-safe
     */
    class MetricNameError : public MeasuroError
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    description    Description of the error
         */
        MetricNameError(const std::string description) : MeasuroError(description)
        {
        }
    };

    /*!
     * @class MetricTypeError
     *
     * @brief Describes an error that occurs because of an incompatible metric type.
     *
     * @remarks thread-safe
     */
    class MetricTypeError : public MeasuroError
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    description    Description of the error
         */
        MetricTypeError(const std::string description) : MeasuroError(description)
        {
        }
    };

    /*!
     * @class MetricCastError
     *
     * @brief Describes an error due to an attempt to cast a metric to an invalid type.
     *
     * @remarks thread-safe
     */
    class MetricCastError : public MeasuroError
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    description    Description of the error
         */
        MetricCastError(const std::string description) : MeasuroError(description)
        {
        }
    };

    /*!
    * Retrieves the current Measuro version as integers.
    *
    * @param[out]    major     Measuro major version
    * @param[out]    minor     Measuro minor version
    * @param[out]    release   Measuro release number
    *
    * @remarks thread-safe
    */
    inline static void version(unsigned int & major, unsigned int & minor, unsigned int & release) noexcept
    {
        major = @LIB_VERSION_MAJOR@;
        minor = @LIB_VERSION_MINOR@;
        release = @LIB_VERSION_REL@;
    }

    /*!
    * Retrieves the current Measuro version as a string.
    *
    * @param[out]    version_str    Measuro version string
    *
    * @remarks thread-safe
    */
    inline static void version_text(std::string & version_str) noexcept(false)
    {
        version_str = "@LIB_VERSION_MAJOR@.@LIB_VERSION_MINOR@-@LIB_VERSION_REL@";
    }

    /*!
     * Retrieves the current Measuro copyright string. Useful for inclusion in
     * abbreviated attribution text.
     *
     * @param[out]    copyright_str    Measuro copyright string
     *
     * @remarks thread-safe
     */
    inline static void copyright_text(std::string & copyright_str) noexcept(false)
    {
        copyright_str = "Measuro version @LIB_VERSION_MAJOR@.@LIB_VERSION_MINOR@-@LIB_VERSION_REL@\n\nCopyright (c) 2017, James Mistry. Released under the MIT licence - for details see https://github.com/jamesmistry/measuro";
    }

    /*!
     * @enum UINT
     *
     * Enum used to uniquely identify unsigned integer metric types in code.
     * The actual value is UINT::KIND
     */
    enum class UINT { KIND };

    /*!
     * @enum INT
     *
     * Enum used to uniquely identify signed integer metric types in code.
     * The actual value is INT::KIND
     */
    enum class INT { KIND };

    /*!
     * Enum used to uniquely identify floating point metric types in code.
     * The actual value is FLOAT::KIND
     */
    enum class FLOAT { KIND };

    /*!
     * @enum RATE
     *
     * Enum used to uniquely identify rate metric types in code. The actual
     * value is RATE::KIND
     */
    enum class RATE { KIND };

    /*!
     * @enum STR
     *
     * Enum used to uniquely identify string metric types in code. The actual
     * value is STR::KIND
     */
    enum class STR { KIND };

    /*!
     * @enum BOOL
     *
     * Enum used to uniquely identify boolean metric types in code. The actual
     * value is BOOL::KIND
     */
    enum class BOOL { KIND };

    /*!
     * @enum SUM
     *
     * Enum used to uniquely identify sum metric types in code. The actual
     * value is SUM::KIND
     */
    enum class SUM { KIND };

    /*!
     * @class Metric
     *
     * @brief Base class of all metric classes.
     *
     * Specifies an abstract interface for metric classes and provides a rate-
     * limited hook mechanism for other metric objects to be invoked when
     * a metric's underlying value changes.
     *
     * @remarks thread-safe
     */
    class Metric
    {
    public:
        /*!
         * @enum Kind
         *
         * The kind of metric as determined by the inheriting class.
         */
        enum class Kind { UINT = 0, INT = 1, FLOAT = 2, RATE = 3, STR = 4, BOOL = 5, SUM = 6 };

        /*!
         * Constructor.
         *
         * @param[in]    kind                  The kind of the underlying metric
         * @param[in]    name                  The name of the metric, as it will appear in rendered output
         * @param[in]    unit                  The units of the metric's value
         * @param[in]    description           A short description of what the metric measures, as it will appear in rendered output
         * @param[in]    time_function         The function to use for determining the time. Used for simulation in testing
         * @param[in]    hook_rate_limit       The minimum number of milliseconds between invoking hooks registered by other metrics. Used to limit frequency of costly updates. Specify std::chrono::milliseconds::zero() to disable rate limiting
         */
        Metric(const Kind kind, const std::string & name, const std::string & unit, const std::string & description,
                std::function<std::chrono::steady_clock::time_point ()> time_function,
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : m_time_function(time_function), m_kind(kind), m_name(name), m_unit(unit), m_description(description),
          m_last_hook_update(m_time_function()), m_hook_rate_limit(hook_rate_limit), m_has_hooks(false)
        {
        }

        /*!
         * Constructor.
         *
         * @see Metric::Metric(const Kind, const std::string &, const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const std::chrono::milliseconds)
         */
        Metric(const Kind kind, const char * name, const char * unit, const char * description,
                std::function<std::chrono::steady_clock::time_point ()> time_function,
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : m_time_function(time_function), m_kind(kind), m_name(name), m_unit(unit), m_description(description),
          m_last_hook_update(m_time_function()), m_hook_rate_limit(hook_rate_limit), m_has_hooks(false)
        {
        }

        /*!
         * Constructor.
         *
         * @see Metric::Metric(const Kind, const std::string &, const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const std::chrono::milliseconds)
         */
        Metric(const Kind kind, const std::string & name, const std::string & description,
                std::function<std::chrono::steady_clock::time_point ()> time_function,
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : m_time_function(time_function), m_kind(kind), m_name(name), m_description(description),
          m_last_hook_update(m_time_function()), m_hook_rate_limit(hook_rate_limit), m_has_hooks(false)
        {
        }

        /*!
         * Constructor.
         *
         * @see Metric::Metric(const Kind, const std::string &, const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const std::chrono::milliseconds)
         */
        Metric(const Kind kind, const char * name, const char * description,
                std::function<std::chrono::steady_clock::time_point ()> time_function,
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : m_time_function(time_function), m_kind(kind), m_name(name), m_description(description), m_last_hook_update(m_time_function()),
          m_hook_rate_limit(hook_rate_limit), m_has_hooks(false)
        {
        }

        /*!
         * Destructor.
         */
        virtual ~Metric() noexcept
        {
        }

        /*!
         * Get the metric's name.
         *
         * @return name of the metric
         *
         * @remarks thread-safe
         */
        std::string name() const noexcept(false)
        {
            return m_name;
        }

        /*!
         * Get the unit of the metric's underlying value.
         *
         * @return unit of the metric
         *
         * @remarks thread-safe
         */
        std::string unit() const noexcept(false)
        {
            return m_unit;
        }

        /*!
         * Get the metric's description.
         *
         * @return description of the metric
         *
         * @remarks thread-safe
         */
        std::string description() const noexcept(false)
        {
            return m_description;
        }

        /*!
         * Get the kind of the metric.
         *
         * @return metric kind
         *
         * @remarks thread-safe
         */
        Kind kind() const noexcept
        {
            return m_kind;
        }

        /*!
         * Get a string representation of the kind of the metric.
         *
         * @return string representation of the metric kind
         *
         * @remarks thread-safe
         */
        std::string kind_name() const noexcept(false)
        {
            return kind_name(m_kind);
        }

        /*!
         * Given a Metric::Kind value, get the corresponding string
         * representation.
         *
         * @param[in]    kind    The kind value from which the string should be calculated
         *
         * @return string representation of kind
         *
         * @remarks thread-safe
         */
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
            }

            return "";
        }

        /*!
         * Ensures all metrics can be represented as std::string.
         */
        virtual operator std::string() const = 0;

        /*!
         * Cast the metric to an unsigned 64-bit integer. Only call this method
         * if the metric kind is @c Metric::Kind::UINT or a
         * @c Metric::Kind::SUM targeting this metric kind.
         *
         * @throws MetricCastError
         */
        virtual explicit operator std::uint64_t() const
        {
            throw MetricCastError("Metric " + m_name + " of kind " + kind_name() + " cannot be represented as a std::uint64_t");

            return 0;
        }

        /*!
         * Cast the metric to a signed 64-bit integer. Only call this method if
         * the metric kind is @c Metric::Kind::INT or a
         * @c Metric::Kind::SUM targeting this metric kind.
         *
         * @throws MetricCastError
         */
        virtual explicit operator std::int64_t() const
        {
            throw MetricCastError("Metric " + m_name + " of kind " + kind_name() + " cannot be represented as a std::int64_t");

            return 0;
        }

        /*!
         * Cast the metric to a float. Only call this method if the metric kind
         * is @c Metric::Kind::FLOAT, @c Metric::Kind::Rate or a
         * @c Metric::Kind::SUM targeting one of these metric kinds.
         *
         * @throws MetricCastError
         */
        virtual explicit operator float() const
        {
            throw MetricCastError("Metric " + m_name + " of kind " + kind_name() + " cannot be represented as a float");

            return 0.0f;
        }

        /*!
         * Cast the metric to a bool. Only call this method if the metric kind
         * is @c Metric::Kind::BOOL.
         *
         * @throws MetricCastError
         */
        virtual explicit operator bool() const
        {
            throw MetricCastError("Metric " + m_name + " of kind " + kind_name() + " cannot be represented as a bool");

            return false;
        }

        /*!
         * Used by inheriting metric classes to separate the evaluation of the
         * metric value (performed by @c calculate()) from the representation
         * as a native type or string (performed by the relevant @code{.cpp}
         * operator T method @endcode overload).
         */
        virtual void calculate()
        {
            return;
        }

        /*!
         * Registers a "hook" function against the metric that will be called
         * when the metric's value changes, in accordance with the hook rate
         * limit specified.
         *
         * The hook function must take one argument of type
         * @code{.cpp} const measuro::Metric & @endcode
         *
         * @param[in]    registrant    Hook function to call
         *
         * @remarks thread-safe
         */
        void register_hook(std::function<void (const Metric & metric)> registrant) noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_metric_mutex);

            m_hooks.push_back(registrant);
            m_has_hooks = true;
        }

        /*!
         * Gets the hook rate limit value in effect for the metric. The
         * hook rate limit is the minimum number of milliseconds between
         * hook function calls.
         *
         * @return the hook rate limit, in milliseconds
         *
         * @remarks thread-safe
         */
        std::chrono::milliseconds hook_rate_limit() const
        {
            return m_hook_rate_limit;
        }

    protected:
        /*!
         * Used by inheriting metric classes to perform the update of their
         * underlying values in a way that respects hook rate limits.
         *
         * @param[in]    update_logic    The function that performs the underlying value update
         *
         * @remarks thread-safe
         */
        void update(std::function<void()> update_logic) noexcept(false)
        {
            /*
             * update_logic() must always be called outside an m_metric_mutex critical
             * section so that the hook logic is free to acquire the mutex if it needs
             * to.
             */
            update_logic();

            if (m_has_hooks)
            {
                auto now = m_time_function();
                if (((m_hook_rate_limit == std::chrono::milliseconds::zero()) ||
                        (now - m_last_hook_update.load()) >= m_hook_rate_limit))
                {
                    // TODO: Replace with std::scoped_lock on migration to C++17
                    std::lock_guard<std::mutex> lock(m_metric_mutex);

                    for (auto hook : m_hooks)
                    {
                        hook(*this);
                    }

                    m_last_hook_update = now;
                }
            }
        }

        mutable std::mutex m_metric_mutex; //!< Mutex for protecting concurrent access to the metric's members
        std::function<std::chrono::steady_clock::time_point ()> m_time_function; //!< Function used to determine the time

    private:
        Kind m_kind; //!< Metric kind
        std::string m_name; //!< Metric name
        std::string m_unit; //!< Metric unit
        std::string m_description; //!< Metric description
        std::atomic<std::chrono::steady_clock::time_point> m_last_hook_update; //!< Time the last hook functions were called
        std::chrono::milliseconds m_hook_rate_limit; //!< hook rate limit, the minimum number of milliseconds between hook function calls
        std::vector<std::function<void (const Metric & metric)> > m_hooks; //!< Array of registered hook functions
        std::atomic<bool> m_has_hooks; //!< Are there any registered hooks? If not, a shortcut is taken in Metric::update

    };

    /*!
     * @class DiscoverableNativeType
     *
     * @brief An interface for exposing a metric's "native type"
     *
     * A metric's "native type" is the type used to represent the metric's
     * value internally.
     */
    template<typename T>
    class DiscoverableNativeType
    {
    public:
        typedef T NativeType;
    };

    /*!
     * @class NumberMetric
     *
     * @brief A metric representing signed/unsigned integer and floating point numbers
     *
     * A template class for metrics to be represented as numeric values.
     * Various arithmetic and assignment operators are overloaded for
     * manipulating metric objects easily.
     *
     * @par Template arguments
     *
     * @par
     * K - the metric's kind
     *
     * @par
     * T - the metric's native type
     *
     * @remarks thread-safe
     */
    template<Metric::Kind K, typename T>
    class NumberMetric : public Metric, public DiscoverableNativeType<T>
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    name                  @see Metric::Metric
         * @param[in]    unit                  @see Metric::Metric
         * @param[in]    description           @see Metric::Metric
         * @param[in]    time_function         @see Metric::Metric
         * @param[in]    initial_value         Value with which to initialise the metric
         * @param[in]    hook_rate_limit    @see Metric::Metric
         */
        NumberMetric(const std::string & name, const std::string & unit, const std::string & description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const T initial_value = 0, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(K, name, unit, description, time_function, hook_rate_limit), m_value(initial_value)
        {
        }

        /*!
         * Constructor.
         *
         * @see NumberMetric::NumberMetric(const std::string &, const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const T initial_value, const std::chrono::milliseconds)
         */
        NumberMetric(const char * name, const char * unit, const char * description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const T initial_value = 0, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(K, name, unit, description, time_function, hook_rate_limit), m_value(initial_value)
        {
        }

        NumberMetric(const NumberMetric &) = delete;
        NumberMetric(NumberMetric &&) = delete;
        NumberMetric & operator=(const NumberMetric &) = delete;
        NumberMetric & operator=(NumberMetric &&) = delete;

        /*!
         * Get the metric value as a std::string.
         *
         * @remarks thread-safe
         */
        operator std::string() const noexcept(false) override final
        {
            std::stringstream formatter;

            formatter << std::fixed << std::setprecision(2) << m_value;

            return formatter.str();
        }

        /*!
         * Get the metric value as its native type (template argument T).
         *
         * @remarks thread-safe
         */
        explicit operator T() const noexcept override final
        {
            return m_value;
        }

        /*!
         * Assignment operator.
         *
         * @param[in]    rhs    Value to assign to the metric
         *
         * @remarks thread-safe
         */
        void operator=(T rhs) noexcept(false)
        {
            update([this, rhs]()
            {
                m_value = rhs;
            });
        }

        /*!
         * Pre-increment operator.
         *
         * @return incremented value
         *
         * @remarks thread-safe
         */
        T operator++() noexcept
        {
            T new_val = m_value;

            update([this, & new_val]()
            {
                new_val = ++m_value;
            });

            return new_val;
        }

        /*!
         * Post-increment operator.
         *
         * @param ignored
         *
         * @return original value (before increment operation)
         *
         * @remarks thread-safe
         */
        T operator++(int) noexcept
        {
            T old_val = m_value;

            update([this, & old_val]()
            {
                old_val = m_value++;
            });

            return old_val;
        }

        /*!
         * Pre-decrement operator.
         *
         * @return decremented value
         *
         * @remarks thread-safe
         */
        T operator--() noexcept
        {
            T new_val = m_value;

            update([this, & new_val]()
            {
                new_val = --m_value;
            });

            return new_val;
        }

        /*!
         * Post-decrement operator.
         *
         * @param ignored
         *
         * @return original value (before decrement operation)
         *
         * @remarks thread-safe
         */
        T operator--(int) noexcept
        {
            T old_val = m_value;

            update([this, & old_val]()
            {
                old_val = m_value--;
            });

            return old_val;
        }

        /*!
         * Adds the specified value to the metric.
         *
         * @param[in]    rhs    Value to add
         *
         * @return the value of the metric, after the add operation
         *
         * @remarks thread-safe
         */
        T operator+=(const T & rhs) noexcept
        {
            T new_val = m_value;

            update([this, rhs, & new_val]()
            {
                new_val = m_value += rhs;
            });

            return new_val;
        }

        /*!
         * Subtracts the specified value from the metric.
         *
         * @param[in]    rhs    Value to subtract
         *
         * @return the value of the metric, after the subtract operation
         *
         * @remarks thread-safe
         */
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
        std::atomic<T> m_value; //!< Metric's value

    };

    /*!
     * @class NumberMetric<Metric::Kind::FLOAT, float>
     *
     * @brief Specialisation of NumberMetric for float values.
     *
     * @see NumberMetric
     *
     * @remarks thread-safe
     */
    template<>
    class NumberMetric<Metric::Kind::FLOAT, float> : public Metric, public DiscoverableNativeType<float>
    {
    public:
        /*!
         * Constructor.
         *
         * @see NumberMetric
         */
        NumberMetric(const std::string & name, const std::string & unit, const std::string & description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const float initial_value = 0, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::FLOAT, name, unit, description, time_function, hook_rate_limit), m_value(initial_value)
        {
        }

        /*!
         * Constructor.
         *
         * @see NumberMetric
         */
        NumberMetric(const char * name, const char * unit, const char * description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const float initial_value = 0, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::FLOAT, name, unit, description, time_function, hook_rate_limit), m_value(initial_value)
        {
        }

        NumberMetric(const NumberMetric &) = delete;
        NumberMetric(NumberMetric &&) = delete;
        NumberMetric & operator=(const NumberMetric &) = delete;
        NumberMetric & operator=(NumberMetric &&) = delete;

        /*!
         * Get the metric value as a std::string. Always represented to 2
         * decimal places.
         *
         * @remarks thread-safe
         */
        operator std::string() const noexcept(false) override final
        {
            std::stringstream formatter;

            formatter << std::fixed << std::setprecision(2) << m_value;

            return formatter.str();
        }

        /*!
         * Get the metric value as a float.
         *
         * @remarks thread-safe
         */
        explicit operator float() const noexcept override final
        {
            return m_value;
        }

        /*!
         * Assignment operator.
         *
         * @param[in]    rhs    Value to assign to the metric
         *
         * @remarks thread-safe
         */
        void operator=(float rhs) noexcept(false)
        {
            update([this, rhs]()
            {
                m_value = rhs;
            });
        }

    private:
        std::atomic<float> m_value; //!< Metric's value

    };

    /*!
     * @class RateMetric
     *
     * @brief Tracks the rate of change of a "distance" NumberMetric
     *
     * The rate is always expressed per second. The rate is calculated only on
     * calls to ::calculate, and even then at most once per second. The rest
     * of the time, the metric's value is pulled from a cache. A result proxy
     * function can be used to modify the calculated rate before it is stored.
     * This is particularly useful for performing unit conversions.
     *
     * @par Template arguments
     *
     * @par
     * D - the type of the metric object whose rate is to be tracked
     *
     * @remarks thread-safe
     */
    template<typename D>
    class RateMetric : public Metric, public DiscoverableNativeType<float>
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    distance              The metric whose rate is to be tracked
         * @param[in]    result_proxy          A function used to modify a calculated rate before it is committed
         * @param[in]    name                  @see Metric::Metric
         * @param[in]    unit                  @see Metric::Metric
         * @param[in]    description           @see Metric::Metric
         * @param[in]    time_function         @see Metric::Metric
         * @param[in]    hook_rate_limit    @see Metric::Metric
         */
        RateMetric(std::shared_ptr<D> & distance, std::function<float (float)> result_proxy, const std::string & name, const std::string & unit, const std::string & description,
                std::function<std::chrono::steady_clock::time_point ()> time_function, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::RATE, name, unit, description, time_function, hook_rate_limit), m_distance(distance), m_result_proxy(result_proxy),
          m_last_distance(0), m_cache(0.0f)
        {
        }

        /*!
         * @see RateMetric::RateMetric(std::shared_ptr<D> &, std::function<float (float)>, const std::string &, const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const std::chrono::milliseconds)
         */
        RateMetric(std::shared_ptr<D> & distance, std::function<float (float)> result_proxy, const char * name, const char * unit, const char * description,
                std::function<std::chrono::steady_clock::time_point ()> time_function, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::RATE, name, unit, description, time_function, hook_rate_limit), m_distance(distance), m_result_proxy(result_proxy),
          m_last_distance(0), m_cache(0.0f)
        {
        }

        /*!
         * @see RateMetric::RateMetric(std::shared_ptr<D> &, std::function<float (float)>, const std::string &, const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const std::chrono::milliseconds)
         */
        RateMetric(std::shared_ptr<D> & distance, const std::string & name, const std::string & unit, const std::string & description,
                std::function<std::chrono::steady_clock::time_point ()> time_function, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::RATE, name, unit, description, time_function, hook_rate_limit), m_distance(distance), m_last_distance(0), m_cache(0.0f)
        {
        }

        /*!
         * @see RateMetric::RateMetric(std::shared_ptr<D> &, std::function<float (float)>, const std::string &, const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const std::chrono::milliseconds)
         */
        RateMetric(std::shared_ptr<D> & distance, const char * name, const char * unit, const char * description,
                std::function<std::chrono::steady_clock::time_point ()> time_function, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::RATE, name, unit, description, time_function, hook_rate_limit), m_distance(distance), m_last_distance(0), m_cache(0.0f)
        {
        }

        RateMetric(const RateMetric &) = delete;
        RateMetric(RateMetric &&) = delete;
        RateMetric & operator=(const RateMetric &) = delete;
        RateMetric & operator=(RateMetric &&) = delete;

        /*!
         * Get the metric value as a std::string. Always represented
         * to 2 decimal places.
         *
         * @remarks thread-safe
         */
        operator std::string() const noexcept(false) override final
        {
            std::stringstream formatter;
            formatter << std::fixed << std::setprecision(2) << m_cache;
            return formatter.str();
        }

        /*!
         * Get the metric value as a float.
         *
         * @remarks thread-safe
         */
        explicit operator float() const noexcept override final
        {
            return m_cache;
        }

        /*!
         * For a given value @c val apply the specified proxy function and
         * return the result. Alternatively if no proxy function is specified,
         * return @c val
         *
         * @param[in]    val    Value to proxy
         *
         * @return proxied value
         *
         * @remarks thread-safe if proxy function is also thread-safe
         */
        float proxy_value(float val) const
        {
            return ((m_result_proxy) ? m_result_proxy(val) : val);
        }

        /*!
         * Calculates the rate. Internally the calculations are cached.
         * The cache is updated at most once per second, so there is no need to
         * (but little harm in) calling this method more frequently than that.
         *
         * As with all metric kinds this method is called automatically before
         * rendering, thus ensuring the metric's value is up-to-date prior to
         * the render operation.
         *
         * If the metric's value is needed outside of a render operation, it's
         * recommended that rather than call this method applications instead
         * rely on their regular render operation (assuming there is one) to
         * keep the cache up-to-date. Reading the value is then simply a case
         * of casting the metric to a @c float or @c std::string
         */
        void calculate() override final
        {
            auto now_time = m_time_function();

            if (now_time - m_last_calc_time >= std::chrono::milliseconds(1000))
            {
                update([this, & now_time]()
                {
                    float distance = static_cast<typename D::NativeType>(*m_distance);

                    auto time_elapsed_sec = float(std::chrono::duration_cast<std::chrono::milliseconds>(now_time - m_last_calc_time).count()) / 1000;
                    auto distance_travelled = float(distance - m_last_distance);

                    if (time_elapsed_sec != 0.0)
                    {
                        float value = distance_travelled / time_elapsed_sec;
                        m_cache = proxy_value(value);
                    }

                    m_last_distance = distance;
                    m_last_calc_time = now_time;
                });
            }
        }

    private:
        std::shared_ptr<D> m_distance; //!< Distance metric whose rate is to be tracked
        std::function<float (float)> m_result_proxy; //!< Proxy function to use for rate values
        std::atomic<float> m_last_distance; //!< The distance value last read
        std::atomic<float> m_cache; //!< Most recently calculated rate
        std::chrono::steady_clock::time_point m_last_calc_time; //!< Last time at which the cache was updated

    };

    /*!
     * @class SumMetric
     *
     * @brief Sums the values of other metrics
     *
     * Adds the values of multiple other "target" metrics together. Unlike
     * RateMetric, SumMetric doesn't use hooks, but simply defers the summing
     * operation until it is rendered (or cast to a std::string).
     *
     * @par Template arguments
     *
     * @par
     * D - the type of the metric objects to be summed
     *
     * @remarks thread-safe
     */
    template<typename D>
    class SumMetric : public Metric, public DiscoverableNativeType<typename D::NativeType>
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    targets               List of target metrics whose values are to be summed
         * @param[in]    name                  @see Metric::Metric
         * @param[in]    unit                  @see Metric::Metric
         * @param[in]    description           @see Metric::Metric
         * @param[in]    time_function         @see Metric::Metric
         * @param[in]    hook_rate_limit    @see Metric::Metric
         */
        SumMetric(std::initializer_list<std::shared_ptr<D> > targets, const std::string & name, const std::string & unit, const std::string & description,
                std::function<std::chrono::steady_clock::time_point ()> time_function, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::SUM, name, unit, description, time_function, hook_rate_limit), m_cache(0)
        {
            for (auto target : targets)
            {
                add_target(target);
            }
        }

        /*!
         * @see SumMetric::SumMetric(std::initializer_list<std::shared_ptr<D> >, const std::string &, const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const std::chrono::milliseconds)
         */
        SumMetric(const std::string & name, const std::string & unit, const std::string & description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::SUM, name, unit, description, time_function, hook_rate_limit), m_cache(0)
        {
        }

        /*!
         * @see SumMetric::SumMetric(std::initializer_list<std::shared_ptr<D> >, const std::string &, const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const std::chrono::milliseconds)
         */
        SumMetric(std::initializer_list<std::shared_ptr<D> > targets, const char * name, const char * unit, const char * description,
                std::function<std::chrono::steady_clock::time_point ()> time_function, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::SUM, name, unit, description, time_function, hook_rate_limit), m_cache(0)
        {
            for (auto target : targets)
            {
                add_target(target);
            }
        }

        /*!
         * @see SumMetric::SumMetric(std::initializer_list<std::shared_ptr<D> >, const std::string &, const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const std::chrono::milliseconds)
         */
        SumMetric(const char * name, const char * unit, const char * description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::SUM, name, unit, description, time_function, hook_rate_limit), m_cache(0)
        {
        }

        SumMetric(const SumMetric &) = delete;
        SumMetric(SumMetric &&) = delete;
        SumMetric & operator=(const SumMetric &) = delete;
        SumMetric & operator=(SumMetric &&) = delete;

        /*!
         * Adds a target metric to the list of those to be summed.
         *
         * @param[in]    target    The metric to be included in the sum operation
         *
         * @remarks thread-safe
         */
        void add_target(std::shared_ptr<D> & target) noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_metric_mutex);

            m_targets.push_back(target);
            m_cache = m_cache + static_cast<typename D::NativeType>(*target);
        }

        /*!
         * Get the sum as the native type defined by the target metric type.
         *
         * @remarks thread-safe
         */
        explicit operator typename D::NativeType() const noexcept override final
        {
            return m_cache;
        }

        /*!
         * Get the sum as a std::string. Always represented to 2 decimal
         * places for float metrics.
         *
         * @remarks thread-safe
         */
        operator std::string() const noexcept(false) override final
        {
            std::stringstream formatter;
            formatter << std::fixed << std::setprecision(2) << m_cache;

            return formatter.str();
        }

        /*!
         * Get the number of metrics being summed.
         *
         * @return the number of metrics
         *
         * @remarks thread-safe
         */
        std::size_t target_count() const noexcept
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_metric_mutex);

            return m_targets.size();
        }

        /*!
         * Adds the values of the target metrics together and caches the
         * result. This is a relatively expensive operation, so call it as
         * infrequently as possible.
         *
         * As with all metric kinds this method is called automatically before
         * rendering, thus ensuring the metric's value is up-to-date prior to
         * the render operation.
         *
         * If the metric's value is needed outside of a render operation, it's
         * recommended that rather than call this method applications instead
         * rely on their regular render operation (assuming there is one) to
         * keep the metric up-to-date. Reading the value is then simply a case
         * of casting the metric to an integral or @c std::string
         */
        void calculate() override final
        {
            update([this]()
            {
                // TODO: Replace with std::scoped_lock on migration to C++17
                std::lock_guard<std::mutex> lock(m_metric_mutex);

                typename D::NativeType total = 0;
                for (std::size_t index=0;index<m_targets.size();++index)
                {
                    total += static_cast<const typename D::NativeType>(*m_targets[index]);
                }

                m_cache = total;
            });
        }

    private:
        std::vector<std::shared_ptr<D> > m_targets; //!< Array of targets to be summed
        std::atomic<typename D::NativeType> m_cache;

    };

    /*!
     * @class StringMetric
     *
     * @brief A metric that is assigned a string.
     *
     * @remarks thread-safe
     */
    class StringMetric : public Metric, public DiscoverableNativeType<std::string>
    {
    public:
        /*!
         *
         * @param[in]    name                  @see Metric::Metric
         * @param[in]    description           @see Metric::Metric
         * @param[in]    time_function         @see Metric::Metric
         * @param[in]    initial_value         Value with which to initialise the metric
         * @param[in]    hook_rate_limit       @see Metric::Metric
         */
        StringMetric(const std::string & name, const std::string & description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const std::string & initial_value, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::STR, name, description, time_function, hook_rate_limit), m_value(initial_value)
        {
        }

        /*!
         * @see StringMetric::StringMetric(const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const std::string &, const std::chrono::milliseconds)
         */
        StringMetric(const char * name, const char * description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const char * initial_value, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::STR, name, description, time_function, hook_rate_limit), m_value(initial_value)
        {
        }

        StringMetric(const StringMetric &) = delete;
        StringMetric(StringMetric &&) = delete;
        StringMetric & operator=(const StringMetric &) = delete;
        StringMetric & operator=(StringMetric &&) = delete;

        /*!
         * Get the value of the metric as a string.
         */
        operator std::string() const noexcept(false) override final
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_metric_mutex);

            return m_value;
        }

        /*!
         * Assign a new value to the metric.
         *
         * @param[in]    rhs    The new value to assign
         */
        void operator=(const char * rhs) noexcept(false)
        {
            update([this, rhs]()
            {
                // TODO: Replace with std::scoped_lock on migration to C++17
                std::lock_guard<std::mutex> lock(m_metric_mutex);

                m_value = std::string(rhs);
            });
        }

        /*!
         * @see StringMetric::operator=(const char *)
         */
        void operator=(const std::string & rhs) noexcept(false)
        {
            update([this, rhs]()
            {
                // TODO: Replace with std::scoped_lock on migration to C++17
                std::lock_guard<std::mutex> lock(m_metric_mutex);

                m_value = rhs;
            });
        }

    private:
        std::string m_value; //!< Metric's value

    };

    /*!
     * @class BoolMetric
     *
     * @brief A metric that can have 1 of 2 states: @c true or @c false.
     *
     * @remarks thread-safe
     */
    class BoolMetric : public Metric, public DiscoverableNativeType<bool>
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    name                  @see Metric::Metric
         * @param[in]    description           @see Metric::Metric
         * @param[in]    time_function         @see Metric::Metric
         * @param[in]    initial_value         Value with which to initialise the metric
         * @param[in]    true_rep              String representation of the metric when its value is @c true. Default = TRUE
         * @param[in]    false_rep             String representation of the metric when its value is @c false. Default = FALSE
         * @param[in]    hook_rate_limit    @see Metric::Metric
         */
        BoolMetric(const std::string & name, const std::string & description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const bool initial_value, const std::string true_rep = "TRUE", const std::string false_rep = "FALSE",
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::BOOL, name, description, time_function, hook_rate_limit), m_value(initial_value), m_true_rep(true_rep),
          m_false_rep(false_rep)
        {
        }

        /*!
         * @see BoolMetric::BoolMetric(const std::string &, const std::string &, std::function<std::chrono::steady_clock::time_point ()>, const bool, const std::string, const std::string, const std::chrono::milliseconds)
         */
        BoolMetric(const char * name, const char * description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                const bool initial_value, const std::string true_rep = "TRUE", const std::string false_rep = "FALSE",
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds::zero()) noexcept(false)
        : Metric(Metric::Kind::BOOL, name, description, time_function, hook_rate_limit), m_value(initial_value), m_true_rep(true_rep),
          m_false_rep(false_rep)
        {
        }

        BoolMetric(const BoolMetric &) = delete;
        BoolMetric(BoolMetric &&) = delete;
        BoolMetric & operator=(const BoolMetric &) = delete;
        BoolMetric & operator=(BoolMetric &&) = delete;

        /*!
         * Get the appropriate @c true or @c false string representation of the
         * metric based on its value.
         */
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

        /*!
         * Get the value of the metric as a bool.
         */
        explicit operator bool() const noexcept override final
        {
            return m_value;
        }

        /*!
         * Assign a new value to the metric.
         *
         * @param[in]    rhs    The new value to assign
         */
        void operator=(bool rhs) noexcept
        {
            update([this, rhs]()
            {
                m_value = rhs;
            });
        }

        /*!
         * Set the value of the metric to the logical not of its current value,
         * i.e. if it's current value is @c true then set it to @c false and
         * vice versa.
         */
        bool operator!() noexcept
        {
            return !m_value;
        }

    private:
        std::atomic<bool> m_value; //!< Metric's value
        const std::string m_true_rep; //!< @c true string representation
        const std::string m_false_rep; //!< @c false string representation

    };

    /*!
     * @class Throttle
     *
     * @brief Limits the rate at which operations are performed on a metric
     *
     * Performance can suffer dramatically if a metric is updated with high
     * frequency. This class serves as an adapter for limiting the rate at
     * which a metric is updated in 2 ways: time (minimum time interval
     * between updates) and attempted operation count (minimum attempted
     * operations between updates). Instances of this class are not
     * thread-safe. The performance advantages of its use are in part a result
     * of eliminating locks and atomics.
     *
     * @par Template arguments
     *
     * @par
     * T - the type of the metric object to be throttled
     *
     * @remarks thread-hostile
     */
    template<typename T>
    class Throttle
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    metric           The metric to throttle
         * @param[in]    time_limit       The minimum number of milliseconds between updates
         * @param[in]    op_limit         The number of attempted operations between updates
         * @param[in]    time_function    Function used to determine the time. Used for simulation in testing
         */
        Throttle(std::shared_ptr<T> & metric, const std::chrono::milliseconds time_limit, const std::uint64_t op_limit = 1000,
                std::function<std::chrono::steady_clock::time_point ()> time_function = []{return std::chrono::steady_clock::now();})
        : m_metric(metric), m_time_limit(time_limit), m_next_update(time_function() + time_limit), m_time_function(time_function),
          m_op_limit(((op_limit == 0) ? 1 : op_limit)), m_op_count(0)
        {
        }

        ~Throttle()
        {
        }

        /*!
         * Assign a new value to the throttled metric according to the
         * specified throttling rules.
         *
         * @param[in]    rhs    The new value to assign
         *
         * @remarks thread-hostile
         */
        void operator=(const typename T::NativeType rhs) noexcept(false)
        {
            if (check_update())
            {
                (*m_metric) = rhs;
            }
        }

        /*!
         * Allows the de-reference operator to be used with Throttle objects.
         * Helps avoid confusion with the typical use of Metric objects that
         * requires de-referencing due to them being managed by a
         * std::shared_ptr
         *
         * @return reference to the Throttle object
         *
         * @remarks thread-hostile
         */
        Throttle<T> & operator*() noexcept
        {
            return (*this);
        }

        /*!
         * Get the configured time limit (minimum number of milliseconds
         * between updates).
         *
         * @return configured time limit in milliseconds
         *
         * @remarks thread-hostile
         */
        std::chrono::milliseconds time_limit() const noexcept
        {
            return m_time_limit;
        }

        /*!
         * Get the configured attempted operations limit (number of attempted
         * operations between updates).
         *
         * @return configured attempted operations limit
         *
         * @remarks thread-hostile
         */
        std::uint64_t op_limit() const noexcept
        {
            return m_op_limit;
        }

    private:
        /*!
         * Given the configured time and attempted operations limits, determine
         * if an update should succeed.
         *
         * @return @c true if the update should succeed, @c false otherwise
         *
         * @remarks thread-hostile
         */
        inline bool check_update() noexcept
        {
            if (++m_op_count % m_op_limit == 0)
            {
                auto now = m_time_function();
                if (now >= m_next_update)
                {
                    m_next_update = now + m_time_limit;
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        std::shared_ptr<T> m_metric; //!< The metric being throttled
        const std::chrono::milliseconds m_time_limit; //!< Time limit (milliseconds between updates)
        std::chrono::steady_clock::time_point m_next_update; //!< Time at/after which an update may next succeed
        std::function<std::chrono::steady_clock::time_point ()> m_time_function; //!< Function used to determine the time
        const std::uint64_t m_op_limit; //!< Attempted operation limit (attempted operations between updates)
        std::uint64_t m_op_count; //!< Total attempted operations
    };

    /*!
     * @brief Throttle template specialisation for NumberMetric objects
     *
     * A template specialisation of Throttle providing addition operator
     * overloads useful for manipulating NumberMetric objects. These operator
     * overloads remember their right-hand side operand when the addition
     * operation can't be performed on the metric due to throttling. The
     * stored value is then added to the metric when the next addition
     * operation is committed. As with Throttle, this class is not thread-safe.
     *
     * @see Throttle
     *
     * @par Template arguments
     *
     * @par
     * K - the kind of the NumberMetric being throttled
     *
     * @par
     * T - the native type of the NumberMetric being throttled
     *
     * @remarks thread-hostile
     */
    template<Metric::Kind K, typename T>
    class Throttle<NumberMetric<K, T> >
    {
    public:
        /*!
         * @see Throttle
         */
        Throttle(std::shared_ptr<NumberMetric<K, T> > & metric, const std::chrono::milliseconds time_limit, const std::uint64_t op_limit = 1000,
                std::function<std::chrono::steady_clock::time_point ()> time_function = []{return std::chrono::steady_clock::now();})
        : m_metric(metric), m_time_limit(time_limit), m_next_update(time_function() + time_limit), m_time_function(time_function),
          m_op_limit(((op_limit == 0) ? 1 : op_limit)), m_op_count(0), m_pending_val(0)
        {
        }

        ~Throttle()
        {
        }

        /*!
         * @see Throttle
         */
        void operator=(const T & rhs) noexcept
        {
            if (check_update())
            {
                (*m_metric) = rhs;
                m_pending_val = 0;
            }
        }

        /*!
         * Adds the specified value to the metric if allowed by the throttling
         * parameters.
         *
         * @param[in]    rhs    Number to be added to the metric
         *
         * @return @c true if the metric was updated, @c false if it was not due to throttling
         *
         * @remarks thread-hostile
         */
        bool operator+=(const T & rhs) noexcept
        {
            m_pending_val += rhs;

            if (check_update())
            {
                commit();
                return true;
            }
            else
            {
                return false;
            }
        }

        /*!
         * Increments the metric if allowed by the throttling parameters.
         *
         * @return @c true if the metric was updated, @c false if it was not due to throttling
         *
         * @remarks thread-hostile
         */
        bool operator++() noexcept
        {
            ++m_pending_val;

            if (check_update())
            {
                commit();
                return true;
            }
            else
            {
                return false;
            }
        }

        /*!
         * @see Throttle::operator*
         */
        Throttle<NumberMetric<K, T> > & operator*() noexcept
        {
            return (*this);
        }

        /*!
         * Updates the metric with any pending addition operations deferred due
         * to throttling.
         */
        inline void commit() noexcept
        {
            (*m_metric) += m_pending_val;
            m_pending_val = 0;
        }

        /*!
         * @see Throttle::time_limit
         */
        std::chrono::milliseconds time_limit() const noexcept
        {
            return m_time_limit;
        }

        /*!
         * @see Throttle::op_limit
         */
        std::uint64_t op_limit() const noexcept
        {
            return m_op_limit;
        }

    private:
        /*!
         * @see Throttle::check_update
         */
        inline bool check_update() noexcept
        {
            if (++m_op_count % m_op_limit == 0)
            {
                auto now = m_time_function();
                if (now >= m_next_update)
                {
                    m_next_update = now + m_time_limit;
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        std::shared_ptr<NumberMetric<K, T> > m_metric; //!< @see Throttle
        const std::chrono::milliseconds m_time_limit; //!< @see Throttle
        std::chrono::steady_clock::time_point m_next_update; //!< @see Throttle
        std::function<std::chrono::steady_clock::time_point ()> m_time_function; //!< @see Throttle
        const std::uint64_t m_op_limit; //!< @see Throttle
        std::uint64_t m_op_count; //!< @see Throttle
        T m_pending_val; //!< Uncommitted value waiting to be added to the metric
    };

    /*!
     * @class Renderer
     *
     * @brief Interface for metric "renderers" (output objects)
     *
     * @remarks thread-hostile
     */
    class Renderer
    {
    public:
        Renderer() : m_suppressed_exception(false)
        {
        }

        virtual ~Renderer() noexcept
        {
        }

        /*!
         * For a given render operation, called once before all calls to
         * @link Renderer::render @endlink
         *
         * @see Renderer::render
         */
        virtual void before()
        {
            return;
        }

        /*!
         * For a given render operation, called once after all calls to
         * @link Renderer::render @endlink
         *
         * @see Renderer::render
         */
        virtual void after()
        {
            return;
        }

        /*!
         * For a given render operation, called once for each metric to be
         * rendered.
         *
         * @param[in]    metric    The metric to be rendered
         */
        virtual void render(const std::shared_ptr<Metric> & metric)
        {
            (void)(metric);
            return;
        }

        /*!
         * Sets or unsets the "suppressed exception" flag which is used to
         * indicate if an exception thrown in a derived method of
         * after was suppressed. This will happen to prevent exceptions
         * propagating out of the RendererContext destructor.
         *
         * @param[in]    state    The state to which to set the flag
         *
         * @see Renderer::after
         */
        void suppressed_exception(bool state) noexcept
        {
            m_suppressed_exception = state;
        }

        /*!
         * Gets the state of the "suppressed exception" flag.
         *
         * @see Renderer::suppressed_exception(bool)
         *
         * @return @c true if the flag is set, @c false otherwise
         */
        bool suppressed_exception() const noexcept
        {
            return m_suppressed_exception;
        }

    private:
        bool m_suppressed_exception; //!< The flag - @c true if an exception was suppressed, @c false otherwise

    };

    /*!
     * @class PlainRenderer
     *
     * @brief Renders metrics as simple line-delimited key value pairs in the form \<metric name\> = \<metric value\>
     *
     * @remarks thread-hostile
     */
    class PlainRenderer : public Renderer
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    destination    Output stream to which to write the rendered metrics. This stream object @b must persist beyond the life of the renderer
         */
        PlainRenderer(std::ostream & destination) noexcept : m_destination(destination)
        {
        }

        virtual ~PlainRenderer() noexcept
        {
        }

        /*!
         * Adds a newline to the stream and flushes its buffer.
         */
        virtual void after() noexcept(false) override
        {
            m_destination << std::endl;
        }

        /*!
         * Renders a metric as \<metric name\> = \<metric value\> \<metric unit\>\\n
         * and writes it to the output stream.
         *
         * @param[in]    metric    The metric to render
         */
        void render(const std::shared_ptr<Metric> & metric) noexcept(false) override final
        {
            std::string unit_part = metric->unit();
            if (unit_part.size() > 0)
            {
                m_destination << metric->name() << " = " << std::string((*metric)) << ' ' << unit_part << '\n';
            }
            else
            {
                m_destination << metric->name() << " = " << std::string((*metric)) << '\n';
            }
        }

    private:
        std::ostream & m_destination; //!< Output stream to which to write rendered output

    };

    /*!
     * @class JsonStringLiteral
     *
     * @brief A valid, quoted and escaped JSON string literal
     *
     * Converts a data buffer to a JSON string literal for insertion
     * directly in JSON data. The string literal representation contains
     * the source data with leading and trailing double quotes along with
     * appropriately escaped characters according to standards (see
     * https://tools.ietf.org/html/rfc7159).
     */
    class JsonStringLiteral
    {

    public:
        /*!
         * Constructor.
         *
         * @param[in]    data    Contents of the string literal
         */
        explicit JsonStringLiteral(const std::string & data) noexcept(false)
        {
            literalise(data, m_result);
        }

        /*!
         * Constructor.
         *
         * @param[in]    data    Contents of the string literal
         */
        explicit JsonStringLiteral(const char * data) noexcept(false)
        {
            std::string strData(data);

            literalise(strData, m_result);
        }

        JsonStringLiteral(const JsonStringLiteral & rhs) = default;
        JsonStringLiteral(JsonStringLiteral && rhs) noexcept(false) : m_result(std::move(rhs.m_result))
        {
        }

        JsonStringLiteral & operator=(JsonStringLiteral & rhs) = default;
        JsonStringLiteral & operator=(JsonStringLiteral && rhs) noexcept(false)
        {
            if (this != &rhs)
            {
                m_result = std::move(rhs.m_result);
                rhs.m_result.clear();
            }

            return (*this);
        }

        /*!
         * Get a pointer to a null-terminated string containing the JSON string literal,
         * including leading and terminating double quotes, and appropriately escaped
         * source characters.
         */
        operator const char * () const noexcept
        {
            return m_result.c_str();
        }

    private:
        /*!
         * Create a JSON-compatible string literal representation of in, and write it to
         * out.
         *
         * @param[in]     in     The string from which to create a JSON string literal
         * @param[out]    out    The result
         */
        void literalise(const std::string & in, std::string & out) const noexcept(false)
        {
            out.push_back('"');

            for (auto c : in)
            {
                if ((c >= 0) && (c <= 0x1f))
                {
                    switch(c)
                    {
                    case 8: // Backspace
                        out.push_back('\\');
                        out.push_back('b');
                        break;
                    case 9: // Horizontal tab
                        out.push_back('\\');
                        out.push_back('t');
                        break;
                    case 10: // Line feed
                        out.push_back('\\');
                        out.push_back('n');
                        break;
                    case 12: // Form feed
                        out.push_back('\\');
                        out.push_back('f');
                        break;
                    case 13: // Carriage return
                        out.push_back('\\');
                        out.push_back('r');
                        break;
                    default:
                        escape_as_hex(c, out);
                        break;
                    }
                }
                else
                {
                    switch(c)
                    {
                    case '"':
                    case '\\':
                    case '/':
                        // Must be escaped
                        out.push_back('\\');
                        out.push_back(c);
                        break;
                    default:
                        out.push_back(c);
                        break;
                    }
                }
            }

            out.push_back('"');
        }

        /*!
         * Escape c as a JSON hex escape sequence of the form \uXXXX, where XXXX is a
         * 32-bit integer in hexadecimal notation, and append it to out.
         *
         * @param[in]     c      The character to escape
         * @param[out]    out    The string to which the escaped character will be appended
         */
        void escape_as_hex(char c, std::string & out) const noexcept
        {
            std::stringstream formatter;

            formatter << "\\u00" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << static_cast<std::uint16_t>(c);

            out += formatter.str();
        }

        std::string m_result; //!< The normalised JSON string literal

    };

    /*!
     * @class JsonRenderer
     *
     * @brief Renders metrics as a JSON object
     *
     * Rendered metrics consist of a single JSON dictionary whose keys are
     * metric names that each refer to a single JSON dictionary containing
     * information about the metric. Specifically, the metric's @em value,
     * @em unit, @em kind and @em description. Note that rendered JSON is never
     * pretty-printed (but the example below is for convenience).
     *
     * @include json_renderer_example.json
     *
     * @remarks thread-hostile
     */
    class JsonRenderer : public Renderer
    {
    public:
        /*!
         * Constructor.
         *
         * @param[in]    destination    Output stream to which to write the rendered metrics. This stream object @b must persist beyond the life of the renderer
         *
         * @remarks thread-hostile
         */
        JsonRenderer(std::ostream & destination) noexcept : m_destination(destination), m_count(0)
        {
        }

        virtual ~JsonRenderer() noexcept
        {
        }

        /*!
         * Writes the leading brace of the JSON object to the output stream.
         *
         * @remarks thread-hostile
         */
        virtual void before() noexcept(false) override
        {
            m_count = 0;
            m_destination << '{';
        }

        /*!
         * Writes the trailing brace of the JSON object to the output stream,
         * then flushes the stream's buffer.
         *
         * @remarks thread-hostile
         */
        virtual void after() noexcept(false) override
        {
            m_destination << '}';
            m_destination.flush();
        }

        /*!
         * Renders the metric as a JSON object.
         *
         * @example json_renderer_example.json
         *
         * @param[in]    metric    The metric to render
         *
         * @remarks thread-hostile
         */
        void render(const std::shared_ptr<Metric> & metric) noexcept(false) override final
        {
            if (m_count > 0)
            {
                m_destination << ',';
            }

            m_destination << JsonStringLiteral(metric->name()) << ":{";

            m_destination << JsonStringLiteral("value") << ':';
            switch(metric->kind())
            {
            case Metric::Kind::UINT:
            case Metric::Kind::INT:
            case Metric::Kind::FLOAT:
            case Metric::Kind::RATE:
            case Metric::Kind::SUM:
                m_destination << std::string((*metric));
                break;
            case Metric::Kind::STR:
                m_destination << JsonStringLiteral((*metric));
                break;
            case Metric::Kind::BOOL:
                if (bool(*metric))
                {
                    m_destination << "true";
                }
                else
                {
                    m_destination << "false";
                }
                break;
            }
            m_destination << ',';

            m_destination << JsonStringLiteral("unit") << ':' << JsonStringLiteral(metric->unit()) << ',';
            m_destination << JsonStringLiteral("kind") << ':' << JsonStringLiteral(metric->kind_name()) << ',';
            m_destination << JsonStringLiteral("description") << ':' << JsonStringLiteral(metric->description());

            m_destination << '}';

            ++m_count;
        }

    private:
        std::ostream & m_destination; //!< Output stream to which to write rendered output
        std::size_t m_count; //!< Number of metrics written to the stream since the start of the current render operation

    };

    using UintHandle = std::shared_ptr<NumberMetric<Metric::Kind::UINT, std::uint64_t> >; //!< Handle to an unsigned metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using IntHandle = std::shared_ptr<NumberMetric<Metric::Kind::INT, std::int64_t> >; //!< Handle to an signed metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using FloatHandle = std::shared_ptr<NumberMetric<Metric::Kind::FLOAT, float> >; //!< Handle to an float metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using RateOfUintHandle = std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >; //!< Handle to a rate of unsigned metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using RateOfIntHandle = std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > >; //!< Handle to a rate of signed metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using RateOfFloatHandle = std::shared_ptr<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > >; //!< Handle to a rate of float metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using RateOfSumOfUintHandle = std::shared_ptr<RateMetric<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > >; //!< Handle to a rate of sum of unsigned metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using RateOfSumOfIntHandle = std::shared_ptr<RateMetric<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > >; //!< Handle to a rate of sum of signed metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using RateOfSumOfFloatHandle = std::shared_ptr<RateMetric<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > > >; //!< Handle to a rate of sum of float metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using SumOfUintHandle = std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >; //!< Handle to a sum of unsigned metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using SumOfIntHandle = std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > >; //!< Handle to a sum of signed metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using SumOfFloatHandle = std::shared_ptr<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > >; //!< Handle to a sum of float metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using SumOfRateOfUintHandle = std::shared_ptr<SumMetric<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > >; //!< Handle to a sum of rate of unsigned metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using SumOfRateOfIntHandle = std::shared_ptr<SumMetric<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > >; //!< Handle to a sum of rate of signed metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using SumOfRateOfFloatHandle = std::shared_ptr<SumMetric<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > > >; //!< Handle to a sum of rate of float metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using StringHandle = std::shared_ptr<StringMetric>; //!< Handle to a string metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use
    using BoolHandle = std::shared_ptr<BoolMetric>; //!< Handle to a boolean metric. Behind the scenes this is a std::shared_ptr, so must be de-referenced before use

    using UintThrottle = Throttle<NumberMetric<Metric::Kind::UINT, std::uint64_t> >; //!< Throttle object throttling an unsigned metric
    using IntThrottle = Throttle<NumberMetric<Metric::Kind::INT, std::int64_t> >; //!< Throttle object throttling a signed metric
    using FloatThrottle = Throttle<NumberMetric<Metric::Kind::FLOAT, float> >; //!< Throttle object throttling a float metric
    using StringThrottle = Throttle<StringMetric>; //!< Throttle object throttling a string metric
    using BoolThrottle = Throttle<BoolMetric>; //!< Throttle object throttling a boolean metric

    /*!
     * @class Registry
     *
     * @brief Creates, tracks and renders metrics
     *
     * You probably only need a single Registry object for your entire
     * application. It serves as a factory and clearing house for metric
     * objects, providing helper methods to create, look up and render metrics.
     *
     * @remarks thread-safe
     */
    class Registry
    {
    public:
        /*!
         * @class RenderSchedule
         *
         * @brief Schedules a regular render operation to be performed every @em n seconds
         *
         * @remarks thread-safe
         */
        class RenderSchedule
        {
        public:
            /*!
             * Constructor.
             *
             * @param[in]    registry    Registry object containing the metrics to render
             * @param[in]    renderer    The renderer to use in each render operation
             * @param[in]    interval    Number of seconds between scheduled renders
             */
            RenderSchedule(Registry & registry, Renderer & renderer, const std::chrono::seconds interval) noexcept(false)
            : m_registry(registry), m_renderer(renderer), m_stop(false), m_interval(interval), m_executor(std::bind(&RenderSchedule::executor_logic, this))
            {
            }

            ~RenderSchedule() noexcept
            {
                try
                {
                    stop();
                }
                catch(...)
                {
                }
            }

            /*!
             * Stops the scheduled render operation if it's active. Any thread
             * spawned by the object is stopped.
             */
            void stop() noexcept(false)
            {
                m_stop = true;
                m_stop_cond.notify_one();
                m_executor.join();
            }

        private:
            void executor_logic() noexcept(false)
            {
                do
                {
                    std::unique_lock<std::mutex> lock(m_cond_mutex);

                    auto timeout_when = std::chrono::steady_clock::now() + m_interval;
                    m_stop_cond.wait_until(lock, timeout_when, [&timeout_when, this](){return ((std::chrono::steady_clock::now() >= timeout_when) || (m_stop));});

                    if (m_stop)
                    {
                        break;
                    }

                    m_registry.render(m_renderer);
                }
                while(!m_stop);
            }

            Registry & m_registry; //!< The registry managing the metrics to be rendered
            Renderer & m_renderer; //!< The renderer to be used after each interval
            std::atomic<bool> m_stop; //!< Flag indicating whether or not the scheduler should terminate
            const std::chrono::seconds m_interval; //!< Number of seconds between each render
            std::thread m_executor; //!< Thread in which asynchronous render operations should be performed
            mutable std::condition_variable m_stop_cond; //!< Condition variable used to notify the scheduler of a stop command
            mutable std::mutex m_cond_mutex; //!< Condition variable mutex

        };

        /*!
         * Constructor.
         *
         * @param[in]    time_function    Function used to determine the time. Used for testing - in production, use the default value
         */
        Registry(std::function<std::chrono::steady_clock::time_point ()> time_function = []{return std::chrono::steady_clock::now();}) noexcept
        : m_time_function(time_function)
        {
        }

        /*!
         * Creates an unsigned integer metric.
         *
         * @param[in]    k                     Must be UINT::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric
         * @param[in]    description           Description of the metric
         * @param[in]    initial_value         Value with which to initialise the metric
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        UintHandle create_metric(const UINT k, const std::string name, const std::string unit, const std::string description,
                const std::uint64_t initial_value = 0, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k);

            auto metric = std::make_shared<NumberMetric<Metric::Kind::UINT, std::uint64_t> >(name, unit, description, m_time_function, initial_value, hook_rate_limit);
            register_metric<NumberMetric<Metric::Kind::UINT, std::uint64_t> >(name, metric, m_uint_metrics);
            return metric;
        }

        /*!
         * Creates a signed integer metric.
         *
         * @param[in]    k                     Must be INT::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric
         * @param[in]    description           Description of the metric
         * @param[in]    initial_value         Value with which to initialise the metric
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        IntHandle create_metric(const INT k, const std::string name, const std::string unit, const std::string description,
                const std::uint64_t initial_value = 0, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k);

            auto metric = std::make_shared<NumberMetric<Metric::Kind::INT, std::int64_t> >(name, unit, description, m_time_function, initial_value, hook_rate_limit);
            register_metric<NumberMetric<Metric::Kind::INT, std::int64_t> >(name, metric, m_int_metrics);
            return metric;
        }

        /*!
         * Creates a float metric.
         *
         * @param[in]    k                     Must be FLOAT::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric
         * @param[in]    description           Description of the metric
         * @param[in]    initial_value         Value with which to initialise the metric
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        FloatHandle create_metric(const FLOAT k, const std::string name, const std::string unit, const std::string description,
                const float initial_value = 0, const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k);

            auto metric = std::make_shared<NumberMetric<Metric::Kind::FLOAT, float> >(name, unit, description, m_time_function, initial_value, hook_rate_limit);
            register_metric<NumberMetric<Metric::Kind::FLOAT, float> >(name, metric, m_float_metrics);
            return metric;
        }

        /*!
         * Creates a metric that tracks the rate of change of an unsigned metric.
         *
         * @param[in]    k1                    Must be RATE::KIND
         * @param[in]    k2                    Must be UINT::KIND
         * @param[in]    distance              The metric whose rate of change is to be tracked
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric, e.g. bytes/sec
         * @param[in]    description           Description of the metric
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         * @param[in]    result_proxy          A function given the opportunity to modify a calculated rate before being recorded against the metric. Particularly useful for unit conversion, e.g. converting bytes per second to mebibytes per second
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        RateOfUintHandle create_metric(const RATE k1, const UINT k2,
                UintHandle & distance, const std::string name, const std::string unit, const std::string description,
                std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000), std::function<float (float)> result_proxy = nullptr) noexcept(false)
        {
            (void)(k1);
            (void)(k2);

            auto metric = std::make_shared<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(distance, result_proxy, name, unit, description, m_time_function, hook_rate_limit);
            register_metric<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that tracks the rate of change of an signed metric.
         *
         * @param[in]    k1                    Must be RATE::KIND
         * @param[in]    k2                    Must be INT::KIND
         * @param[in]    distance              The metric whose rate of change is to be tracked
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric, e.g. bytes/sec
         * @param[in]    description           Description of the metric
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         * @param[in]    result_proxy          A function given the opportunity to modify a calculated rate before being recorded against the metric. Particularly useful for unit conversion, e.g. converting bytes per second to mebibytes per second
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        RateOfIntHandle create_metric(const RATE k1, const INT k2,
                IntHandle & distance, const std::string name, const std::string unit, const std::string description,
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000), std::function<float (float)> result_proxy = nullptr) noexcept(false)
        {
            (void)(k1);
            (void)(k2);

            auto metric = std::make_shared<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > >(distance, result_proxy, name, unit, description, m_time_function, hook_rate_limit);
            register_metric<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that tracks the rate of change of an float metric.
         *
         * @param[in]    k1                    Must be RATE::KIND
         * @param[in]    k2                    Must be FLOAT::KIND
         * @param[in]    distance              The metric whose rate of change is to be tracked
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric, e.g. bytes/sec
         * @param[in]    description           Description of the metric
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         * @param[in]    result_proxy          A function given the opportunity to modify a calculated rate before being recorded against the metric. Particularly useful for unit conversion, e.g. converting bytes per second to mebibytes per second
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        RateOfFloatHandle create_metric(const RATE k1, const FLOAT k2,
                FloatHandle & distance, const std::string name, const std::string unit, const std::string description,
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000), std::function<float (float)> result_proxy = nullptr) noexcept(false)
        {
            (void)(k1);
            (void)(k2);

            auto metric = std::make_shared<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > >(distance, result_proxy, name, unit, description, m_time_function, hook_rate_limit);
            register_metric<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that tracks the rate of change of a sum of unsigned
         * metrics.
         *
         * @param[in]    k1                    Must be RATE::KIND
         * @param[in]    k2                    Must be SUM::KIND
         * @param[in]    k3                    Must be UINT::KIND
         * @param[in]    distance              The metric whose rate of change is to be tracked
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric, e.g. bytes/sec
         * @param[in]    description           Description of the metric
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         * @param[in]    result_proxy          A function given the opportunity to modify a calculated rate before being recorded against the metric. Particularly useful for unit conversion, e.g. converting bytes per second to mebibytes per second
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        RateOfSumOfUintHandle create_metric(const RATE k1, const SUM k2, const UINT k3,
                SumOfUintHandle & distance, const std::string name, const std::string unit, const std::string description,
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000), std::function<float (float)> result_proxy = nullptr) noexcept(false)
        {
            (void)(k1);
            (void)(k2);
            (void)(k3);

            auto metric = std::make_shared<RateMetric<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > >(distance, result_proxy, name, unit, description, m_time_function, hook_rate_limit);
            register_metric<RateMetric<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that tracks the rate of change of a sum of signed
         * metrics.
         *
         * @param[in]    k1                    Must be RATE::KIND
         * @param[in]    k2                    Must be SUM::KIND
         * @param[in]    k3                    Must be INT::KIND
         * @param[in]    distance              The metric whose rate of change is to be tracked
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric, e.g. bytes/sec
         * @param[in]    description           Description of the metric
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         * @param[in]    result_proxy          A function given the opportunity to modify a calculated rate before being recorded against the metric. Particularly useful for unit conversion, e.g. converting bytes per second to mebibytes per second
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        RateOfSumOfIntHandle create_metric(const RATE k1, const SUM k2, const INT k3,
                SumOfIntHandle & distance, const std::string name, const std::string unit, const std::string description,
                std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000), std::function<float (float)> result_proxy = nullptr) noexcept(false)
        {
            (void)(k1);
            (void)(k2);
            (void)(k3);

            auto metric = std::make_shared<RateMetric<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > >(distance, result_proxy, name, unit, description, m_time_function, hook_rate_limit);
            register_metric<RateMetric<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that tracks the rate of change of a sum of float
         * metrics.
         *
         * @param[in]    k1                    Must be RATE::KIND
         * @param[in]    k2                    Must be SUM::KIND
         * @param[in]    k3                    Must be FLOAT::KIND
         * @param[in]    distance              The metric whose rate of change is to be tracked
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric, e.g. bytes/sec
         * @param[in]    description           Description of the metric
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         * @param[in]    result_proxy          A function given the opportunity to modify a calculated rate before being recorded against the metric. Particularly useful for unit conversion, e.g. converting bytes per second to mebibytes per second
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        RateOfSumOfFloatHandle create_metric(const RATE k1, const SUM k2, const FLOAT k3,
                SumOfFloatHandle & distance, const std::string name, const std::string unit, const std::string description,
                std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000), std::function<float (float)> result_proxy = nullptr) noexcept(false)
        {
            (void)(k1);
            (void)(k2);
            (void)(k3);

            auto metric = std::make_shared<RateMetric<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > > >(distance, result_proxy, name, unit, description, m_time_function, hook_rate_limit);
            register_metric<RateMetric<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that sums unsigned metrics.
         *
         * @param[in]    k1                    Must be SUM::KIND
         * @param[in]    k2                    Must be UINT::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric
         * @param[in]    description           Description of the metric
         * @param[in]    targets               List of metric handles to sum together. More can be added after creation by calling SumMetric::add_target
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        SumOfUintHandle create_metric(const SUM k1, const UINT k2,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<UintHandle > targets = {},
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k1);
            (void)(k2);

            auto metric = std::make_shared<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(name, unit, description, m_time_function, hook_rate_limit);

            initialise_sum_with_targets(metric, targets);

            register_metric<SumMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that sums signed metrics.
         *
         * @param[in]    k1                    Must be SUM::KIND
         * @param[in]    k2                    Must be INT::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric
         * @param[in]    description           Description of the metric
         * @param[in]    targets               List of metric handles to sum together. More can be added after creation by calling SumMetric::add_target
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        SumOfIntHandle create_metric(const SUM k1, const INT k2,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<IntHandle > targets = {},
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k1);
            (void)(k2);

            auto metric = std::make_shared<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > >(name, unit, description, m_time_function, hook_rate_limit);

            initialise_sum_with_targets(metric, targets);

            register_metric<SumMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that sums float metrics.
         *
         * @param[in]    k1                    Must be SUM::KIND
         * @param[in]    k2                    Must be FLOAT::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric
         * @param[in]    description           Description of the metric
         * @param[in]    targets               List of metric handles to sum together. More can be added after creation by calling SumMetric::add_target
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        SumOfFloatHandle create_metric(const SUM k1, const FLOAT k2,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<FloatHandle > targets = {},
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k1);
            (void)(k2);

            auto metric = std::make_shared<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > >(name, unit, description, m_time_function, hook_rate_limit);

            initialise_sum_with_targets(metric, targets);

            register_metric<SumMetric<NumberMetric<Metric::Kind::FLOAT, float> > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that sums rate of unsigned metrics.
         *
         * @param[in]    k1                    Must be SUM::KIND
         * @param[in]    k2                    Must be RATE::KIND
         * @param[in]    k3                    Must be UINT::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric
         * @param[in]    description           Description of the metric
         * @param[in]    targets               List of metric handles to sum together. More can be added after creation by calling SumMetric::add_target
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        SumOfRateOfUintHandle create_metric(const SUM k1, const RATE k2, const UINT k3,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<RateOfUintHandle > targets = {},
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k1);
            (void)(k2);
            (void)(k3);

            auto metric = std::make_shared<SumMetric<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > >(name, unit, description, m_time_function, hook_rate_limit);

            initialise_sum_with_targets(metric, targets);

            register_metric<SumMetric<RateMetric<NumberMetric<Metric::Kind::UINT, std::uint64_t> > > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that sums rate of signed metrics.
         *
         * @param[in]    k1                    Must be SUM::KIND
         * @param[in]    k2                    Must be RATE::KIND
         * @param[in]    k3                    Must be INT::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric
         * @param[in]    description           Description of the metric
         * @param[in]    targets               List of metric handles to sum together. More can be added after creation by calling SumMetric::add_target
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        SumOfRateOfIntHandle create_metric(const SUM k1, const RATE k2, const INT k3,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<RateOfIntHandle > targets = {},
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k1);
            (void)(k2);
            (void)(k3);

            auto metric = std::make_shared<SumMetric<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > >(name, unit, description, m_time_function, hook_rate_limit);

            initialise_sum_with_targets(metric, targets);

            register_metric<SumMetric<RateMetric<NumberMetric<Metric::Kind::INT, std::int64_t> > > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric that sums rate of float metrics.
         *
         * @param[in]    k1                    Must be SUM::KIND
         * @param[in]    k2                    Must be RATE::KIND
         * @param[in]    k3                    Must be FLOAT::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    unit                  Unit string to associate with the metric
         * @param[in]    description           Description of the metric
         * @param[in]    targets               List of metric handles to sum together. More can be added after creation by calling SumMetric::add_target
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        SumOfRateOfFloatHandle create_metric(const SUM k1, const RATE k2, const FLOAT k3,
                const std::string name, const std::string unit, const std::string description,
                const std::initializer_list<RateOfFloatHandle > targets = {},
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k1);
            (void)(k2);
            (void)(k3);

            auto metric = std::make_shared<SumMetric<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > > >(name, unit, description, m_time_function, hook_rate_limit);

            initialise_sum_with_targets(metric, targets);

            register_metric<SumMetric<RateMetric<NumberMetric<Metric::Kind::FLOAT, float> > > >(name, metric);
            return metric;
        }

        /*!
         * Creates a metric whose value is a string.
         *
         * @param[in]    k                     Must be STR::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    description           Description of the metric
         * @param[in]    initial_value         Value with which to initialise the metric
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        StringHandle create_metric(const STR k, const std::string name, const std::string description,
                const std::string initial_value = "", const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k);

            auto metric = std::make_shared<StringMetric>(name, description, m_time_function, initial_value, hook_rate_limit);
            register_metric<StringMetric>(name, metric, m_str_metrics);
            return metric;
        }

        /*!
         * Creates a metric whose value is either @c true or @c false.
         *
         * @param[in]    k                     Must be BOOL::KIND
         * @param[in]    name                  Name of the metric. This must be unique with respect to all metrics in the registry
         * @param[in]    description           Description of the metric
         * @param[in]    initial_value         Value with which to initialise the metric
         * @param[in]    true_rep              String representation of the metric when its value is @c true
         * @param[in]    false_rep             String representation of the metric when its value is @c false
         * @param[in]    hook_rate_limit       Minimum number of milliseconds between hook operations triggered by changes to the metric. Specify std::chrono::milliseconds::zero() to disable rate limiting
         *
         * @return a handle to the created metric, which must be de-referenced before use
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        BoolHandle create_metric(const BOOL k, const std::string name, const std::string description,
                const bool initial_value = false, const std::string true_rep = "TRUE", const std::string false_rep = "FALSE",
                const std::chrono::milliseconds hook_rate_limit = std::chrono::milliseconds(1000)) noexcept(false)
        {
            (void)(k);

            auto metric = std::make_shared<BoolMetric>(name, description, m_time_function, initial_value, true_rep, false_rep, hook_rate_limit);
            register_metric<BoolMetric>(name, metric, m_bool_metrics);
            return metric;
        }

        /*!
         * Creates a Throttle object for use with an unsigned metric. Throttle
         * objects impose limits on the rate of operations performed on a
         * metric.
         *
         * @param[in]    metric        The metric to throttle
         * @param[in]    time_limit    The minimum number of milliseconds between updates of the metric
         * @param[in]    op_limit      The number of attempted update operations between each actual update of the metric
         *
         * @return the created Throttle object
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        UintThrottle create_throttle(UintHandle & metric, const std::chrono::milliseconds time_limit, std::uint64_t op_limit = 1)
        {
            return UintThrottle(metric, time_limit, op_limit, m_time_function);
        }

        /*!
         * Creates a Throttle object for use with an signed metric. Throttle
         * objects impose limits on the rate of operations performed on a
         * metric.
         *
         * @param[in]    metric        The metric to throttle
         * @param[in]    time_limit    The minimum number of milliseconds between updates of the metric
         * @param[in]    op_limit      The number of attempted update operations between each actual update of the metric
         *
         * @return the created Throttle object
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        IntThrottle create_throttle(IntHandle & metric, const std::chrono::milliseconds time_limit, std::uint64_t op_limit = 1)
        {
            return IntThrottle(metric, time_limit, op_limit, m_time_function);
        }

        /*!
         * Creates a Throttle object for use with a float metric. Throttle
         * objects impose limits on the rate of operations performed on a
         * metric.
         *
         * @param[in]    metric        The metric to throttle
         * @param[in]    time_limit    The minimum number of milliseconds between updates of the metric
         * @param[in]    op_limit      The number of attempted update operations between each actual update of the metric
         *
         * @return the created Throttle object
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        FloatThrottle create_throttle(FloatHandle & metric, const std::chrono::milliseconds time_limit, std::uint64_t op_limit = 1)
        {
            return FloatThrottle(metric, time_limit, op_limit, m_time_function);
        }

        /*!
         * Creates a Throttle object for use with an string metric. Throttle
         * objects impose limits on the rate of operations performed on a
         * metric.
         *
         * @param[in]    metric        The metric to throttle
         * @param[in]    time_limit    The minimum number of milliseconds between updates of the metric
         * @param[in]    op_limit      The number of attempted update operations between each actual update of the metric
         *
         * @return the created Throttle object
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        StringThrottle create_throttle(StringHandle & metric, const std::chrono::milliseconds time_limit, std::uint64_t op_limit = 1)
        {
            return StringThrottle(metric, time_limit, op_limit, m_time_function);
        }

        /*!
         * Creates a Throttle object for use with an bool metric. Throttle
         * objects impose limits on the rate of operations performed on a
         * metric.
         *
         * @param[in]    metric        The metric to throttle
         * @param[in]    time_limit    The minimum number of milliseconds between updates of the metric
         * @param[in]    op_limit      The number of attempted update operations between each actual update of the metric
         *
         * @return the created Throttle object
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        BoolThrottle create_throttle(BoolHandle & metric, const std::chrono::milliseconds time_limit, std::uint64_t op_limit = 1)
        {
            return BoolThrottle(metric, time_limit, op_limit, m_time_function);
        }

        /*!
         * Looks up an unsigned metric by name. Avoid performing lookups in
         * performance-critical code. Instead, keep the metric handle returned
         * on creation and manipulate that directly.
         *
         * @param[in]    k       Must be UINT::KIND
         * @param[in]    name    Name of the metric to look up
         *
         * @return a handle to the found metric
         *
         * @throws MetricNameError
         * @throws MetricTypeError
         *
         * @remarks thread-safe
         */
        UintHandle operator()(const UINT k, const std::string name) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name, Metric::Kind::UINT);
            return m_uint_metrics[entry->second.second];
        }

        /*!
         * Looks up an signed metric by name. Avoid performing lookups in
         * performance-critical code. Instead, keep the metric handle returned
         * on creation and manipulate that directly.
         *
         * @param[in]    k       Must be INT::KIND
         * @param[in]    name    Name of the metric to look up
         *
         * @return a handle to the found metric
         *
         * @throws MetricNameError
         * @throws MetricTypeError
         *
         * @remarks thread-safe
         */
        IntHandle operator()(const INT k, const std::string name) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name, Metric::Kind::INT);
            return m_int_metrics[entry->second.second];
        }

        /*!
         * Looks up an float metric by name. Avoid performing lookups in
         * performance-critical code. Instead, keep the metric handle returned
         * on creation and manipulate that directly.
         *
         * @param[in]    k       Must be FLOAT::KIND
         * @param[in]    name    Name of the metric to look up
         *
         * @return a handle to the found metric
         *
         * @throws MetricNameError
         * @throws MetricTypeError
         *
         * @remarks thread-safe
         */
        FloatHandle operator()(const FLOAT k, const std::string name) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name, Metric::Kind::FLOAT);
            return m_float_metrics[entry->second.second];
        }

        /*!
         * Looks up a string metric by name. Avoid performing lookups in
         * performance-critical code. Instead, keep the metric handle returned
         * on creation and manipulate that directly.
         *
         * @param[in]    k       Must be STR::KIND
         * @param[in]    name    Name of the metric to look up
         *
         * @return a handle to the found metric
         *
         * @throws MetricNameError
         * @throws MetricTypeError
         *
         * @remarks thread-safe
         */
        StringHandle operator()(const STR k, const std::string name) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name, Metric::Kind::STR);
            return m_str_metrics[entry->second.second];
        }

        /*!
         * Looks up a bool metric by name. Avoid performing lookups in
         * performance-critical code. Instead, keep the metric handle returned
         * on creation and manipulate that directly.
         *
         * @param[in]    k       Must be BOOL::KIND
         * @param[in]    name    Name of the metric to look up
         *
         * @return a handle to the found metric
         *
         * @throws MetricNameError
         * @throws MetricTypeError
         *
         * @remarks thread-safe
         */
        BoolHandle operator()(const BOOL k, const std::string name) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name, Metric::Kind::BOOL);
            return m_bool_metrics[entry->second.second];
        }

        /*!
         * Renders any metric in the registry whose name begins with
         * @c name_prefix using the provided Renderer object. Metrics'
         * @c calculate() methods are called automatically prior to rendering.
         *
         * @param[in]    renderer       Renderer to use for rendering the metrics
         * @param[in]    name_prefix    Prefix of the metric names to be rendered
         *
         * @remarks thread-safe
         */
        void render(Renderer & renderer, const std::string & name_prefix) const noexcept(false)
        {
            render_with_prefix(renderer, name_prefix);
        }

        /*!
         * @see Registry::render(Renderer &, const std::string &)
         */
        void render(Renderer & renderer, const char * name_prefix) const noexcept(false)
        {
            std::string str_prefix(name_prefix);
            render_with_prefix(renderer, str_prefix);
        }

        /*!
         * Renders all metrics in the registry.
         *
         * @param[in]    renderer    Renderer to use for rendering the metrics
         *
         * @remarks thread-safe
         */
        void render(Renderer & renderer) const noexcept(false)
        {
            std::string dummy_prefix;
            render_with_prefix(renderer, dummy_prefix);
        }

        /*!
         * Schedules a regular render operation. The render operations will be
         * performed in a different thread. Returns immediately.
         *
         * @param[in]    renderer    Renderer to use for rendering the metrics
         * @param[in]    interval    Interval, in seconds, between each render operation
         *
         * @remarks thread-safe
         */
        void render_schedule(Renderer & renderer, const std::chrono::seconds interval) noexcept(false)
        {
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            m_sched = std::make_shared<RenderSchedule>((*this), renderer, interval);
        }

        /*!
         * Cancels a previously-scheduled render operation.
         *
         * @remarks thread-safe
         */
        void cancel_render_schedule() noexcept(false)
        {
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            m_sched = nullptr;
        }

    private:
        /*!
         * @class RendererContext
         *
         * @brief Provides a runtime context for Renderer objects
         *
         * Uses RAII to ensure that a renderer object's @c before() and
         * @c after() methods are called at the appropriate times.
         *
         * @remarks thread-hostile
         */
        class RendererContext
        {
        public:
            /*!
             * Constructor.
             *
             * @param[in]    renderer    The renderer to contextualise
             */
            RendererContext(Renderer & renderer) noexcept(false) : m_renderer(renderer)
            {
                m_renderer.suppressed_exception(false);
                m_renderer.before();
            }

            /*!
             * Destructor. Renderer exceptions are suppressed to prevent them
             * propagating out of the destructor. This can be detected by
             * calling Renderer::suppressed_exception()
             */
            ~RendererContext() noexcept
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
            Renderer & m_renderer; //!< Renderer being contextualised
        };

        /*!
         * Adds a set of target metrics to a given SumMetric.
         *
         * @param[in]    metric     The SumMetric to which @c targets should be added
         * @param[in]    targets    Metrics to be summed by @c metric
         *
         * @par Template arguments
         *
         * @par
         * M - the SumMetric handle type
         *
         * @par
         * T - the target metric handle type
         *
         * @remarks thread-safe
         */
        template<typename M, typename T>
        void initialise_sum_with_targets(M metric, const std::initializer_list<T> targets) const noexcept(false)
        {
            for (auto target : targets)
            {
                metric->add_target(target);
            }
        }

        /*!
         * @see Registry::render(Renderer &, const std::string &)
         */
        void render_with_prefix(Renderer & renderer, const std::string & name_prefix) const noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            RendererContext render_ctx(renderer);

            for (auto metric : m_metrics)
            {
                if ((name_prefix.length() == 0) || (metric.first.find(name_prefix) == 0))
                {
                    metric.second.first->calculate();
                    renderer.render(metric.second.first);
                }
            }
        }

        /*!
         * Looks a metric up by its name and kind.
         *
         * @param[in]    name             Name of the metric to lookup
         * @param[in]    expected_kind    Kind of the metric to lookup
         *
         * @return iterator to a pair describing the found metric and its index in the kind-specific vector
         *
         * @throws MetricNameError
         * @throws MetricTypeError
         *
         * @remarks thread-safe
         */
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
            else if (entry->second.second == std::numeric_limits<std::uint64_t>::max())
            {
                throw MetricTypeError("The metric called \"" + name + "\" is not of a kind that can be looked up");
            }

            return entry;
        }

        /*!
         * Registers a metric that can't be looked up by name with the
         * registry against the specified name.
         *
         * @param[in]    metric_name    Name of the metric
         * @param[in]    metric         Metric object
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        template<typename T>
        void register_metric(const std::string & metric_name, std::shared_ptr<T > & metric) noexcept(false)
        {
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            if (m_metrics.find(metric_name) != m_metrics.end())
            {
                throw MetricNameError("A metric already exists with the name \"" + metric_name + "\"");
            }

            m_metrics[metric_name] = std::pair<std::shared_ptr<Metric>, std::uint64_t>(metric, std::numeric_limits<std::uint64_t>::max());
        }

        /*!
         * Registers a metric that can be looked up by name with the registry
         * against the specified name.
         *
         * @param[in]    metric_name        Name of the metric
         * @param[in]    metric             Metric object
         * @param[in]    metric_registry    Kind-specific registry in which to store the metric
         *
         * @throws MetricNameError
         *
         * @remarks thread-safe
         */
        template<typename T>
        void register_metric(const std::string & metric_name, std::shared_ptr<T > & metric, std::vector<std::shared_ptr<T > > & metric_registry) noexcept(false)
        {
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            if (m_metrics.find(metric_name) != m_metrics.end())
            {
                throw MetricNameError("A metric already exists with the name \"" + metric_name + "\"");
            }

            metric_registry.push_back(metric);
            m_metrics[metric_name] = std::pair<std::shared_ptr<Metric>, std::uint64_t>(metric, metric_registry.size() - 1);
        }

        mutable std::mutex m_registry_mutex; //!< Mutex for the registry
        std::function<std::chrono::steady_clock::time_point ()> m_time_function; //!< Function used to determine the time
        std::map<std::string, std::pair<std::shared_ptr<Metric>, std::uint64_t> > m_metrics; //!< Generic metric store

        std::vector<UintHandle> m_uint_metrics; //!< Unsigned metric store
        std::vector<IntHandle> m_int_metrics; //!< Signed metric store
        std::vector<FloatHandle> m_float_metrics; //!< Float metric store
        std::vector<StringHandle> m_str_metrics; //!< String metric store
        std::vector<BoolHandle> m_bool_metrics; //!< Bool metric store

        std::shared_ptr<RenderSchedule> m_sched; //!< Scheduler for scheduling regular render operations
    };
}

#endif
