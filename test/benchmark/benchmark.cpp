#include <cstddef>
#include <chrono>
#include <string>
#include <vector>
#include <iostream>

#include "Measuro.hpp"

using namespace measuro;

const auto TEST_DURATION = std::chrono::seconds(5);
const auto RUN_COUNT = 3;
const std::vector<std::string> prime_keys = {"KEY_A", "KEY_B", "KEY_C", "KEY_D", "KEY_E", "KEY_F"};

struct Metrics
{
    Registry reg;
    IntHandle test_count;
    IntHandle prime_count;
    RateOfIntHandle prime_rate;
    StringHandle last_prime_key;

    Metrics()
    : test_count(reg.create_metric(INT::KIND, "TestCount", "integer(s)",
            "The number of integers tested for primality", 0, std::chrono::milliseconds(1000))),
      prime_count(reg.create_metric(INT::KIND, "PrimeCount", "integer(s)",
              "The number of integers found to be primes", 0, std::chrono::milliseconds(1000))),
      prime_rate(reg.create_metric(RATE::KIND, INT::KIND, test_count, "TestRate",
              "integers/s", "The number of integers tested for primality per second", std::chrono::milliseconds(1000))),
      last_prime_key(reg.create_metric(STR::KIND, "LastPrimeKey",
              "A string key arbitrarily associated with the last found prime", "val", std::chrono::milliseconds(1000)))
    {
    }
};

/*
 * Ref: https://en.wikipedia.org/wiki/Primality_test
 */
bool is_prime(std::size_t n)
{
    if (n <= 1)
    {
        return false;
    }
    else if (n <= 3)
    {
        return true;
    }
    else if ((n % 2 == 0) || (n % 3 == 0))
    {
        return false;
    }

    std::size_t i = 5;
    while ((i * i) <= n)
    {
        if ((n % i == 0) || (n % (i + 2) == 0))
        {
            return false;
        }
        i += 6;
    }
    return true;
}

std::int64_t hard_work(bool use_metrics, Metrics & m)
{
    (*m.test_count) = 0;
    (*m.prime_count) = 0;

    auto test_end = std::chrono::steady_clock::now() + TEST_DURATION;
    std::size_t candidate = 0;

    while(std::chrono::steady_clock::now() < test_end)
    {
        if (is_prime(candidate))
        {
            if (use_metrics)
            {
                ++(*m.prime_count);

                (*m.last_prime_key) = prime_keys[candidate % prime_keys.size()];
            }
        }

        if (use_metrics)
        {
            if (candidate % 10000 == 0)
            {
                (*m.test_count) = candidate + 1;
            }
        }

        ++candidate;
    }

    return candidate - 1;
}

int main(int argc, char * argv[])
{
    Metrics m;

    float no_metrics_test_count = 0;
    for (auto run=0;run<RUN_COUNT;++run)
    {
        no_metrics_test_count += hard_work(false, m);
    }
    no_metrics_test_count /= RUN_COUNT;

    float metrics_test_count = 0;
    for (auto run=0;run<RUN_COUNT;++run)
    {
        metrics_test_count += hard_work(true, m);
    }
    metrics_test_count /= RUN_COUNT;

    float score = 0.0f;
    if ((no_metrics_test_count > 0) && (metrics_test_count > 0))
    {
        score = float(float(no_metrics_test_count) / float(metrics_test_count));
    }

    std::cout << "Work items, without metrics = " << no_metrics_test_count << "\n";
    std::cout << "Work items, with metrics = " << metrics_test_count << "\n";
    std::cout << "Score = " << score << "\n";
    std::cout << "        (closer to 1.0 is better)" << std::endl;

    return 0;
}

