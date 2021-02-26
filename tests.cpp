//
// Created by Stas on 25.02.2021.
//

#include <filesystem>
#include <fstream>

#include "gtest/gtest.h"

#include "Tree.h"

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(Tree, Build) {

    std::string currentPath = std::filesystem::current_path().string();

    std::filesystem::create_directory(currentPath + "/testtree");
    std::ofstream os;
    os.open(currentPath + "/testtree/textfile.txt", std::ofstream::out | std::ofstream::trunc);
    os << "test value";
    os.close();

    std::filesystem::create_directory(currentPath + "/testtree/folder");
    os.open(currentPath + "/testtree/folder/anotherfile.txt", std::ofstream::out | std::ofstream::trunc);
    os << "test value 2";
    os.close();

    Tree t(currentPath + "/testtree");
    t.build();

    auto n = t.getRoot()->get("textfile.txt");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->getValue(), "test value");

    n = t.getRoot()->get("/folder/anotherfile.txt");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->getValue(), "test value 2");

    std::filesystem::remove_all(currentPath + "/testtree");

    t.build();

    n = t.getRoot()->get("textfile.txt");
    EXPECT_EQ(n, nullptr);

};

TEST(Tree, BuildFromJson) {

    std::string currentPath = std::filesystem::current_path().string();

    std::filesystem::create_directory(currentPath + "/testtree");
    std::ofstream os;
    os.open(currentPath + "/testtree/testjson.json", std::ofstream::out | std::ofstream::trunc);
    os << R"(
{
    "key": "value",
    "object": {
        "a": "b",
        "foo" : [
            "bar",
            123.45
        ]
    }
})";
    os.close();

    Tree t(currentPath + "/testtree");
    t.build();

    auto n = t.getRoot()->get("/testjson.json/key");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->getValue(), "value");

    n = t.getRoot()->get("/testjson.json/object/foo/0");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->getValue(), "bar");

    std::filesystem::remove_all(currentPath + "/testtree");

}