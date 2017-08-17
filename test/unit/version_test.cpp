#include <gtest/gtest.h>

#include "Measuro.hpp"

namespace measuro
{

    TEST(Metric, version)
    {
        unsigned int maj = 0, min = 0, rel = 0;
        measuro::version(maj, min, rel);

        EXPECT_EQ(maj, 0);
        EXPECT_EQ(min, 1);
        EXPECT_EQ(rel, 1);
    }

	TEST(Metric, version_text)
	{
		std::string ver;
		measuro::version_text(ver);

		EXPECT_EQ(ver, "0.1-1");
	}

	TEST(Metric, copyright_text)
    {
        std::string result;
        measuro::copyright_text(result);

        EXPECT_EQ(result, "Measuro version 0.1-1\n\nCopyright (c) 2017, James Mistry. Released under the MIT licence - for details see https://github.com/jamesmistry/measuro");
    }

}


