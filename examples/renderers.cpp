#include <string>
#include <iostream>

#include "measuro.hpp"

int main(int argc, char * argv[])
{
    measuro::Registry reg;

    auto count_metric = reg.create_metric(measuro::INT::KIND, "example_count",
            "item(s)", "An example count metric", 0);
    auto str_metric = reg.create_metric(measuro::STR::KIND, "example_str",
            "An example string metric");

    *str_metric = "Example text";

    for (auto i=0;i<100;++i)
    {
        ++*count_metric;
    }

    measuro::PlainRenderer pl_renderer(std::cout);
    measuro::JsonRenderer js_renderer(std::cout);

    std::cout << "Plain text output:\n\n";
    reg.render(pl_renderer);
    std::cout << "----------\n\nJSON output:\n\n";
    reg.render(js_renderer);

    std::cout << std::endl; // The JSON renderer doesn't terminate with a newline!

}
