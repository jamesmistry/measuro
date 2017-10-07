#include <iostream>
#include <string>
#include <thread>

#include "measuro.hpp"

int main(int argc, char * argv[])
{
    measuro::Registry reg;

    // Create a metric and store its handle in handle
    auto handle = reg.create_metric(measuro::INT::KIND, "example_metric",
            "file(s)", "An example metric", 10001);

    // Create a throttle for use with the metric - limit updates to 1 per second and 1 per 1000 changes
    auto example_throttle = reg.create_throttle(handle, std::chrono::milliseconds(1000), 1000);

    auto committed_updates = 0;
    auto ignored_updates = 0;
    for (auto i=0;i<10000;++i)
    {
        example_throttle = i;
        if (std::int64_t(*handle) == i)
        {
            ++committed_updates;
        }
        else
        {
            ++ignored_updates;
        }

        switch (i)
        {
            case 2500:
            case 5000:
            case 7500:
                std::this_thread::sleep_for(std::chrono::seconds(1));
                break;

            default:
                break;
        }
    }

    std::cout << "Throttle avoided " << ignored_updates << " update operations (" << committed_updates << " updates made)" << std::endl;

}
