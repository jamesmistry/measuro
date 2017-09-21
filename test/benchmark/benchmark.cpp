/** @file benchmark.cpp
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
 *
 *
 * The purpose of this test is to produce a simple score for use in measuring
 * the effect of Measuro code changes on performance. The score produced is
 * between 0 and 1, representing how much work was done with Measuro enabled as a
 * proportion of the work done with Measuro disabled.
 *
 * Note that this test represents worst-case performance, with no attempt even at
 * trivial optimisation (such as use of Throttle objects or deferred update of
 * metrics).
 *
 * Scores should only be compared when calculated on the same
 * hardware/OS/environment.
 */

#include <cstddef>
#include <chrono>
#include <string>
#include <vector>
#include <iostream>

#include "measuro.hpp"

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

std::int64_t hard_work(bool use_metrics, bool use_throttle, Metrics & m)
{
    (*m.test_count) = 0;
    (*m.prime_count) = 0;

    auto test_throttle = m.reg.create_throttle(m.test_count, std::chrono::milliseconds(1000), 1000);

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
            if (use_throttle)
            {
                test_throttle = candidate + 1;
            }
            else
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
        no_metrics_test_count += hard_work(false, false, m);
    }
    no_metrics_test_count /= RUN_COUNT;

    float metrics_test_count = 0;
    for (auto run=0;run<RUN_COUNT;++run)
    {
        metrics_test_count += hard_work(true, false, m);
    }
    metrics_test_count /= RUN_COUNT;

    float throttle_test_count = 0;
    for (auto run=0;run<RUN_COUNT;++run)
    {
        throttle_test_count += hard_work(true, true, m);
    }
    throttle_test_count /= RUN_COUNT;

    float score = 0.0f;
    float throttle_score = 0.0f;
    if ((no_metrics_test_count > 0) && (metrics_test_count > 0))
    {
        score = 1 - (float(float(no_metrics_test_count) / float(metrics_test_count)) - 1);
        if (score < 0)
        {
            score = 0.0f;
        }
        else if (score > 1)
        {
            score = 1.0f;
        }

        throttle_score = 1 - (float(float(no_metrics_test_count) / float(throttle_test_count)) - 1);
        if (throttle_score < 0)
        {
            throttle_score = 0.0f;
        }
        else if (throttle_score > 1)
        {
            throttle_score = 1.0f;
        }
    }

    std::cout << "Work items, without metrics = " << no_metrics_test_count << "\n";
    std::cout << "Work items, with metrics    = " << metrics_test_count << "\n";
    std::cout << "Score without throttle      = " << score << "\n";
    std::cout << "                              ^ (closer to 1.0 is better)" << std::endl;
    std::cout << "Score with throttle         = " << throttle_score << "\n";
    std::cout << "                              ^ (closer to 1.0 is better)" << std::endl;

    return 0;
}

