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

    Tree t(currentPath + "/testtree");
    t.build();

    auto n = t.getRoot()->get("textfile.txt");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->getValue(), "test value");

    std::filesystem::remove_all(currentPath + "/testtree");

};