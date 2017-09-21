#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>

#include "measuro.hpp"
#include "stubs.hpp"

namespace measuro
{

    TEST(JsonStringLiteral, jsonify_from_str)
    {
        std::string data("x\"\\/\u0008\u0009\u000a\u000c\u000dabc");
        data.push_back('\0');
        data.push_back('\x1f');
        JsonStringLiteral subject(data);
        std::string result(subject);

        EXPECT_EQ("\"x\\\"\\\\\\/\\b\\t\\n\\f\\rabc\\u0000\\u001F\"", result);
    }

    TEST(JsonStringLiteral, jsonify_from_cstr)
    {
        std::string data("x\"\\/\u0008\u0009\u000a\u000c\u000dabc");
        data.push_back('\x1f');
        JsonStringLiteral subject(data.c_str());
        std::string result(subject);

        EXPECT_EQ("\"x\\\"\\\\\\/\\b\\t\\n\\f\\rabc\\u001F\"", result);
    }

    TEST(JsonStringLiteral, move_ctor)
    {
        std::string data("test data");

        JsonStringLiteral src(data.c_str());
        JsonStringLiteral dest(std::move(src));
        std::string result(dest);

        EXPECT_EQ(result, "\"test data\"");
        EXPECT_EQ(std::string(src), "");
    }

    TEST(JsonStringLiteral, move_ass)
    {
        std::string data("test data");

        JsonStringLiteral src(data.c_str());
        JsonStringLiteral dest("original");
        dest = std::move(src);
        std::string result(dest);

        EXPECT_EQ(result, "\"test data\"");
        EXPECT_EQ(std::string(src), "");
    }

}
