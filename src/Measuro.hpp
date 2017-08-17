#ifndef MEASURO_HPP
#define MEASURO_HPP

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
        MeasuroError(std::string description) : std::runtime_error(description.c_str())
        {
        }
    };

    class MetricNameError : public MeasuroError
    {
    public:
        MetricNameError(std::string description) : MeasuroError(description)
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

    using DeadlineUnit = std::chrono::milliseconds;

    enum class UINT { KIND };
    enum class INT { KIND };
    enum class FLOAT { KIND };
    enum class RATE { KIND };
    enum class STR { KIND };
    enum class MOV_AVG { KIND };
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
        enum class Kind { UINT = 0, INT = 1, FLOAT = 2, RATE = 3, STR = 4, MOV_AVG = 5, BOOL = 6, SUM = 7 };

        Metric(Kind kind, std::string name, std::string unit, std::string description,
                std::function<std::chrono::steady_clock::time_point ()> time_function, DeadlineUnit rate_limit_1_per = DeadlineUnit::zero()) noexcept
        : m_kind(kind), m_name(name), m_unit(unit), m_description(description),
          m_last_updated(time_function()), m_time_function(time_function), m_rate_limit(rate_limit_1_per)
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
            switch(m_kind)
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
            case Kind::MOV_AVG:
                return "MOV_AVG";
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
            std::lock_guard<std::mutex> lock(m_metric_mutex);

            m_hooks.push_back(registrant);
            m_has_hooks = true;
        }

    protected:

        void hook_handler(std::chrono::steady_clock::time_point update_time)
        {
            (void)(update_time);
            return;
        }

        void update(std::function<void()> update_logic) noexcept(false)
        {
            auto now = m_time_function();

            update_logic();

            if ((m_has_hooks) && ((m_cascade_limit == DeadlineUnit::zero()) ||
                    (now - m_last_hook_update.load()) >= m_cascade_limit))
            {
                std::lock_guard<std::mutex> lock(m_metric_mutex);

                for (auto hook : m_hooks)
                {
                    hook(m_last_hook_update);
                }

                m_last_hook_update = now;
            }
        }

        std::mutex m_metric_mutex;

    private:
        Kind m_kind;
        std::string m_name;
        std::string m_unit;
        std::string m_description;
        std::atomic<std::chrono::steady_clock::time_point> m_last_hook_update;
        std::function<std::chrono::steady_clock::time_point ()> m_time_function;
        DeadlineUnit m_cascade_limit;
        std::vector<std::function<void (std::chrono::steady_clock::time_point update_time)> > m_hooks;
        std::atomic<bool> m_has_hooks;

    };

    template<Metric::Kind K, typename T>
    class NumberMetric : public Metric
    {
    public:
        NumberMetric(std::string name, std::string unit, std::string description, std::function<std::chrono::steady_clock::time_point ()> time_function,
                T initial_value = 0, DeadlineUnit cascade_rate_limit = DeadlineUnit::zero()) noexcept
        : Metric(K, name, unit, description, time_function, cascade_rate_limit), m_value(initial_value)
        {
        }

        NumberMetric(const NumberMetric &) = delete;
        NumberMetric(NumberMetric &&) = delete;
        NumberMetric & operator=(const NumberMetric &) = delete;
        NumberMetric & operator=(NumberMetric &&) = delete;

        operator std::string() const noexcept(false)
        {
            std::stringstream formatter;

            // TODO: Replace with "static if" on migration to C++17
            switch(kind())
            {
            case Metric::Kind::FLOAT:
                formatter << std::fixed << std::setprecision(2) << m_value;
                break;
            default:
                formatter << m_value;
                break;
            }

            return formatter.str();
        }

        operator T() const noexcept
        {
            return m_value;
        }

        void operator=(T rhs) noexcept(false)
        {
            update([this, rhs]()
            {
                this->m_value = rhs;
            });
        }

        T operator++() noexcept
        {
            T new_val = this->m_value;

            update([this, & new_val]()
            {
                new_val = ++this->m_value;
            });

            return new_val;
        }

        T operator++(int) noexcept
        {
            T old_val = this->m_value;

            update([this, & old_val]()
            {
                old_val = this->m_value++;
            });

            return old_val;
        }

        T operator--() noexcept
        {
            T new_val = this->m_value;

            update([this, & new_val]()
            {
                new_val = --this->m_value;
            });

            return new_val;
        }

        T operator--(int) noexcept
        {
            T old_val = this->m_value;

            update([this, & old_val]()
            {
                old_val = this->m_value--;
            });

            return old_val;
        }

        T operator+=(const T & rhs) noexcept
        {
            T new_val = this->m_value;

            update([this, rhs, & new_val]()
            {
                new_val = this->m_value += rhs;
            });

            return new_val;
        }

        T operator-=(const T & rhs) noexcept
        {
            T new_val = this->m_value;

            update([this, rhs, & new_val]()
            {
                new_val = this->m_value -= rhs;
            });

            return new_val;
        }

    private:
        std::atomic<std::uint64_t> m_value;

    };

    class Renderer
    {
    public:
        Renderer()
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

    };

    class TextRenderer : Renderer
    {
    public:
        TextRenderer(std::ostream & destination) : m_destination(destination)
        {
        }

        virtual ~TextRenderer()
        {
        }

        void after()
        {
            m_destination << std::endl;
        }

        void render(const std::shared_ptr<Metric> & metric)
        {
            m_destination << metric->name() << " = " << metric << "\n";
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

        std::shared_ptr<UintMetric> create_metric(UINT, std::string name, std::string unit, std::string description,
                std::uint64_t initial_value = 0, DeadlineUnit rate_limit_1_per = DeadlineUnit::zero()) noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto metric = std::shared_ptr<UintMetric>(new UintMetric(name, unit, description, m_time_function, initial_value, rate_limit_1_per));
            register_metric<UintMetric>(name, metric, m_uint_metrics);
            return metric;
        }

        std::shared_ptr<UintMetric> operator()(UINT, std::string name) noexcept(false)
        {
            // TODO: Replace with std::scoped_lock on migration to C++17
            std::lock_guard<std::mutex> lock(m_registry_mutex);

            auto entry = lookup(name);
            return m_uint_metrics[entry->second.second];
        }

        void render(Renderer & renderer, std::string name_prefix = "") noexcept(false)
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
                m_renderer.before();
            }

            ~RendererContext()
            {
                m_renderer.after();
            }

        private:
            Renderer & m_renderer;
        };

        std::map<std::string, std::pair<std::shared_ptr<Metric>, std::uint64_t> >::const_iterator
        lookup(const std::string & name) const noexcept(false)
        {
            auto entry = m_metrics.find(name);
            if (entry == m_metrics.end())
            {
                throw MetricNameError("No metric exists with the name \"" + name + "\"");
            }

            return entry;
        }

        template<typename T>
        void register_metric(std::string & metric_name, std::shared_ptr<T> & metric, std::vector<std::shared_ptr<T> > & metric_registry) noexcept(false)
        {
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

        std::mutex m_registry_mutex;
        std::function<std::chrono::steady_clock::time_point ()> m_time_function;
        std::map<std::string, std::pair<std::shared_ptr<Metric>, std::uint64_t> > m_metrics;

        std::vector<std::shared_ptr<UintMetric> > m_uint_metrics;

    };
}

#endif
