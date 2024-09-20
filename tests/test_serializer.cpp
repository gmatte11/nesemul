#include <catch2/catch_all.hpp>

#include <list>
#include <string>
#include <variant>

#include "settings/serializer.h"

using namespace std::literals;

TEST_CASE("Serializer Integer Types", "[serializer]")
{
    Serializer out;

    {
        int8_t i8 = 42;
        out.process("value", i8);
        CHECK(out.to_string() == "{\"value\":42}");

        auto in = Serializer::from_string("{\"value\":16}");
        CHECK(in);
        in.value().process("value", i8);
        CHECK(i8 == 16);
    }

    {
        uint8_t u8 = 42;
        out.process("value", u8);
        CHECK(out.to_string() == "{\"value\":42}");

        auto in = Serializer::from_string("{\"value\":16}");
        CHECK(in);
        in.value().process("value", u8);
        CHECK(u8 == 16);
    }

    {
        int16_t i16 = 42;
        out.process("value", i16);
        CHECK(out.to_string() == "{\"value\":42}");

        auto in = Serializer::from_string("{\"value\":16}");
        CHECK(in);
        in.value().process("value", i16);
        CHECK(i16 == 16);
    }

    {
        uint16_t u16 = 42;
        out.process("value", u16);
        CHECK(out.to_string() == "{\"value\":42}");

        auto in = Serializer::from_string("{\"value\":16}");
        CHECK(in);
        in.value().process("value", u16);
        CHECK(u16 == 16);
    }

    {
        int32_t i32 = 42;
        out.process("value", i32);
        CHECK(out.to_string() == "{\"value\":42}");

        auto in = Serializer::from_string("{\"value\":16}");
        CHECK(in);
        in.value().process("value", i32);
        CHECK(i32 == 16);
    }

    {
        uint32_t u32 = 42;
        out.process("value", u32);
        CHECK(out.to_string() == "{\"value\":42}");

        auto in = Serializer::from_string("{\"value\":16}");
        CHECK(in);
        in.value().process("value", u32);
        CHECK(u32 == 16);
    }

    {
        int64_t i64 = 42;
        out.process("value", i64);
        CHECK(out.to_string() == "{\"value\":42}");

        auto in = Serializer::from_string("{\"value\":16}");
        CHECK(in);
        in.value().process("value", i64);
        CHECK(i64 == 16);
    }

    {
        uint64_t u64 = 42;
        out.process("value", u64);
        CHECK(out.to_string() == "{\"value\":42}");

        auto in = Serializer::from_string("{\"value\":16}");
        CHECK(in);
        in.value().process("value", u64);
        CHECK(u64 == 16);
    }
}

TEST_CASE("Serializer String Type", "[serializer]")
{
    Serializer out;

    std::string s = "Long John Silver";
    out.process("name", s);
    CHECK(out.to_string() == "{\"name\":\"Long John Silver\"}");
    CHECK(out.to_string(true) == "{\n    \"name\": \"Long John Silver\"\n}");

    auto in = Serializer::from_string("{\"surname\":\"Silver\"}");
    in.value().process("surname", s);
    CHECK(s == "Silver");
}

TEST_CASE("Serializer Array Type", "[serializer]")
{
    using namespace Catch::Matchers;
    Serializer out;

    SECTION("Array of Integers")
    {
        std::vector<int> vi = {1, 1, 2, 3, 5, 8, 13};
        out.process("ints", vi);
        CHECK(out.to_string() == "{\"ints\":[1,1,2,3,5,8,13]}");

        auto in = Serializer::from_string("{\"in_ints\":[1,2,3,4,5]}");
        in.value().process("in_ints", vi);
        CHECK_THAT(vi, Equals(std::vector<int>{1,2,3,4,5}));
    }

    SECTION ("Array of strings")
    {
        std::vector<std::string> vs = {"once", "upon", "a", "time"};
        out.process("strs", vs);
        CHECK(out.to_string() == "{\"strs\":[\"once\",\"upon\",\"a\",\"time\"]}");

        auto in = Serializer::from_string("{\"in_strs\":[\"one\",\"two\",\"three\"]}");
        in.value().process("in_strs", vs);
        CHECK_THAT(vs, Equals(std::vector<std::string>{"one"s, "two"s, "three"s}));
    }
}

TEST_CASE("Serializer Errors", "[serializer]")
{
    auto opt = Serializer::from_string("{\"value1\": 42, \"value2\": \"A String\"}");

    Serializer& in = opt.value();

    int i;
    std::string s;

    CHECK_THROWS(in.process("value", i));
    CHECK_THROWS(in.process("value1", s));
    CHECK_THROWS(in.process("value2", i));
}