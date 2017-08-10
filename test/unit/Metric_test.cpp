#include <gtest/gtest.h>

#include "Measuro.hpp"

namespace locj
{

	TEST(Metric, test_test)
	{
		std::string ver;
		measuro::version_text(ver);

		EXPECT_EQ(ver, "0.1-1");
	}

}


