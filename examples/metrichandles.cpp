#include <iostream>
#include <string>

#include "measuro.hpp"

int main(int argc, char * argv[])
{
    measuro::Registry reg;

    // Create a metric and store its handle in handle
    auto handle = reg.create_metric(measuro::INT::KIND, "example_metric",
            "file(s)", "An example metric");

    // Find the metric's handle by specifying its kind and name. Note that if
    // the metric can't be found, an exception of type measuro::MetricTypeError
    // or measuro::MetricNameError will be thrown
    auto found_handle = reg(measuro::INT::KIND, "example_metric");

    if (handle == found_handle)
    {
        std::cout << "Found the handle for " << found_handle->name() << "!\n";
    }
}
