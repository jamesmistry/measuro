#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "measuro.hpp"

using namespace std::chrono_literals;

int main(int argc, char * argv[])
{
    measuro::Registry reg;

    // Create a metric and store its handle in handle
    auto num_handle = reg.create_metric(measuro::UINT::KIND, "example_metric",
            "units", "An example number metric");
    auto rate_handle = reg.create_metric(measuro::RATE::KIND, 
            measuro::UINT::KIND, num_handle, "example_rate", "units/sec", 
            "An example rate metric");

    measuro::PlainRenderer renderer(std::cout);
    reg.render_schedule(renderer, std::chrono::seconds(1));

    std::this_thread::sleep_for(1000ms);
    *num_handle = 0;

    std::this_thread::sleep_for(1000ms);
    *num_handle = 100;

    std::this_thread::sleep_for(1000ms);
    *num_handle = 600;

    std::this_thread::sleep_for(1000ms);
    *num_handle = 0;

    std::this_thread::sleep_for(1000ms);
    *num_handle = 1000;

    std::this_thread::sleep_for(1000ms);
    *num_handle = 0;
}
