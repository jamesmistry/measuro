#include <iostream>
#include <string>
#include <memory>

#include "measuro.hpp"

class ExampleCustomRenderer : public measuro::Renderer
{
public:
    ExampleCustomRenderer(std::ostream & destination)
        : m_destination(destination)
    {
    }

    virtual void before() override final
    {
        m_destination << "--begin metrics--\n";
    }

    virtual void after() override final
    {
        m_destination << "--end metrics--" << std::endl;
    }

    virtual void render(const std::shared_ptr<measuro::Metric> & metric) override final
    {
        m_destination << metric->name() << '=' << std::string(*metric) << '\n';
    }

private:
    std::ostream & m_destination;

};

int main(int argc, char * argv[])
{
    measuro::Registry reg;

    auto count_metric = reg.create_metric(measuro::INT::KIND, "example_count",
        "item(s)", "An example count metric", 1234);
    auto str_metric = reg.create_metric(measuro::STR::KIND, "example_str",
        "An example string metric", "str value");

    ExampleCustomRenderer renderer(std::cout);

    reg.render(renderer);
}
