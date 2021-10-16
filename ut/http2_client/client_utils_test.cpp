#include "client_utils.hpp"

#include <gtest/gtest.h>

#include "script.hpp"

namespace http2_client
{
TEST(client_utils_test, BuildUri)
{
    std::string host("host"), port("port"), path("/path/");
    ASSERT_EQ("http://" + host + ":" + port + "/" + path, build_uri(host, port, path));
}

void check_pre_built_headers(const size_t s, const header_map& built)
{
    auto element = built.find(CONTENT_TYPE);
    ASSERT_NE(element, built.end());
    ASSERT_EQ(element->second.value, APP_JSON);
    ASSERT_FALSE(element->second.sensitive);

    element = built.find(CONTENT_LENGTH);
    ASSERT_NE(element, built.end());
    ASSERT_EQ(element->second.value, std::to_string(8));
    ASSERT_FALSE(element->second.sensitive);
}

TEST(client_utils_test, BuildHeadersEmptyHeaders)
{
    traffic::msg_headers h;
    size_t s{8};
    const auto built = build_headers(s, h);
    check_pre_built_headers(s, built);
    ASSERT_EQ(built.size(), 2);
}

TEST(client_utils_test, BuildHeadersAddingHeaders)
{
    traffic::msg_headers h{{"first_k", "first_v"}, {"second_k", "second_v"}};
    size_t s{8};
    const auto built = build_headers(s, h);
    check_pre_built_headers(s, built);
    ASSERT_EQ(built.size(), 4);

    auto element = built.find("first_k");
    ASSERT_NE(element, built.end());
    ASSERT_EQ(element->second.value, "first_v");
    ASSERT_FALSE(element->second.sensitive);

    element = built.find("second_k");
    ASSERT_NE(element, built.end());
    ASSERT_EQ(element->second.value, "second_v");
    ASSERT_FALSE(element->second.sensitive);
}

}  // namespace http2_client