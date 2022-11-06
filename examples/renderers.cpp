#include <string>
#include <iostream>
#include <chrono>

#include "measuro.hpp"

int main(int argc, char * argv[])
{
    measuro::Registry reg;

    auto count_metric_1 = reg.create_metric(measuro::INT::KIND, "example_count",
            "item(s)", "An example count metric", 0);
    auto count_metric_2 = reg.create_metric(measuro::INT::KIND, "example_count2",
            "item(s)", "An example count metric", 0);
    auto str_metric = reg.create_metric(measuro::STR::KIND, "example_str",
            "An example string metric");

    *str_metric = "Example text";

    for (auto i=0;i<100;++i)
    {
        ++*count_metric_1;
    }

    *count_metric_2 = 1234;

    measuro::PlainRenderer pl_renderer(std::cout);
    measuro::JsonRenderer js_renderer(std::cout);
    measuro::PrometheusRenderer pr_renderer(std::cout, []()
            { 
                /* Assumes C++20 */ 
                return std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count(); 
            }, "example_app");

    std::cout << "Plain text output:\n\n";
    reg.render(pl_renderer);

    std::cout << "----------\n\nJSON output:\n\n";
    reg.render(js_renderer);
    std::cout << "\n\n"; // The JSON renderer doesn't terminate with a newline!

    std::cout << "----------\n\nPrometheus output:\n\n";
    reg.render(pr_renderer); // Prometheus renderer always ends with a newline!
}
