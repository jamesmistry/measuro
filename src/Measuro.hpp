#ifndef MEASURO_HPP
#define MEASURO_HPP

#include <string>
#include <cstdint>
#include <chrono>
#include <atomic>
#include <functional>

namespace measuro
{
    /*!
    *
    *
    * @param major
    * @param minor
    * @param release
    */
    static void version(unsigned int & major, unsigned int & minor, unsigned int & release) noexcept
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
    static void version_text(std::string & version_str) noexcept(false)
    {
        version_str = "@LIB_VERSION_MAJOR@.@LIB_VERSION_MINOR@-@LIB_VERSION_REL@";
    }

    /*!
     *
     */
    enum class MetricKind { UNSIGNED_INT, SIGNED_INT, FLOAT, RATE, STRING, ROLLING_AVG, BOOL, SUM };

    using DeadlineUnit = std::chrono::milliseconds;

    /*!
     * @class Metric
     *
     *
     */
    class Metric
    {
    public:
        enum class Kind { UNSIGNED_INT, SIGNED_INT, FLOAT, RATE, STRING, MOVING_AVG, BOOL, SUM };

        Metric(Kind kind, const char * name, const char * unit = nullptr, DeadlineUnit rate_limit_1_per = DeadlineUnit::zero()) noexcept
        : m_kind(kind), m_name(name), m_unit(((unit == nullptr) ? "" : unit)), m_last_updated(std::chrono::steady_clock::now()), m_rate_limit(rate_limit_1_per)
        {
        }

        virtual ~Metric()
        {
        }

        const std::string & name() const noexcept
        {
            return m_name;
        }

        const std::string & unit() const noexcept
        {
            return m_unit;
        }

        Kind kind() const noexcept
        {
            return m_kind;
        }

        void kind_name(std::string & name) const noexcept
        {
            switch(m_kind)
            {
            case Kind::UNSIGNED_INT:
                name = "UNSIGNED_INT";
                break;
            case Kind::SIGNED_INT:
                name = "SIGNED_INT";
                break;
            case Kind::FLOAT:
                name = "FLOAT";
                break;
            case Kind::RATE:
                name = "RATE";
                break;
            case Kind::STRING:
                name = "STRING";
                break;
            case Kind::MOVING_AVG:
                name = "MOVING_AVG";
                break;
            case Kind::BOOL:
                name = "BOOL";
                break;
            case Kind::SUM:
                name = "SUM";
                break;
            }
        }

        virtual operator std::string() = 0;

    protected:
        void update(std::function<void()> update_logic) noexcept(false)
        {
            if ((m_rate_limit == DeadlineUnit::zero()) ||
                    (std::chrono::steady_clock::now() - m_last_updated) >= m_rate_limit)
            {
                update_logic();
                m_last_updated = std::chrono::steady_clock::now();
            }
        }

    private:
        std::string m_name;
        Kind m_kind;
        std::string m_unit;
        std::chrono::steady_clock::time_point m_last_updated;
        DeadlineUnit m_rate_limit;

    };

    class UnsignedMetric : public Metric
    {
    public:
        UnsignedMetric(const char * name, const char * unit = nullptr, std::uint64_t initial_value = 0, DeadlineUnit rate_limit_1_per = DeadlineUnit::zero()) noexcept
        : m_value(initial_value), Metric(Metric::Kind::UNSIGNED_INT, name, unit, rate_limit_1_per)
        {
        }

        UnsignedMetric(const UnsignedMetric &) = delete;
        UnsignedMetric(UnsignedMetric &&) = delete;
        UnsignedMetric & operator=(const UnsignedMetric &) = delete;
        UnsignedMetric & operator=(UnsignedMetric &&) = delete;

        operator std::string() const noexcept(false)
        {
            return std::to_string(m_value);
        }

        operator std::uint64_t() const noexcept
        {
            return m_value;
        }

        void operator=(std::uint64_t rhs) noexcept(false)
        {
            update([this, rhs]()
            {
                this->m_value = rhs;
            });
        }

        std::uint64_t operator++() noexcept
        {
            update([this]()
            {
                this->m_value++;
            });

            return m_value;
        }

        std::uint64_t operator++(int) noexcept
        {
            std::uint64_t old = m_value;

            update([this]()
            {
                this->m_value++;
            });

            return old;
        }

        std::uint64_t operator--() noexcept
        {
            update([this]()
            {
                this->m_value--;
            });

            return m_value;
        }

        std::uint64_t operator--(int) noexcept
        {
            std::uint64_t old = m_value;

            update([this]()
            {
                this->m_value--;
            });

            return old;
        }

        std::uint64_t operator+=(const std::uint64_t & rhs) noexcept
        {
            update([this, rhs]()
            {
                this->m_value += rhs;
            });

            return m_value;
        }

        std::uint64_t operator-=(const std::uint64_t & rhs) noexcept
        {
            update([this, rhs]()
            {
                this->m_value -= rhs;
            });

            return m_value;
        }

    private:
        std::atomic<std::uint64_t> m_value;

    };
}

#endif
