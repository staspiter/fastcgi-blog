//
// Created by Stas Piter on 25.02.2021.
//

#include <filesystem>
#include <fstream>

#include "gtest/gtest.h"

#include "Tree.h"
#include "Generator.h"

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

TEST(Tree, GetPages) {

    std::string currentPath = std::filesystem::current_path().string();

    std::filesystem::create_directory(currentPath + "/testtree");
    std::ofstream os;
    os.open(currentPath + "/testtree/testjson.template.json", std::ofstream::out | std::ofstream::trunc);
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

    std::vector<std::string> pages;

    auto n = t.getRoot()->get("testjson/key", &pages);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->getValue(), "value");
    EXPECT_EQ(pages[0], "testjson.template.json");

    std::filesystem::remove_all(currentPath + "/testtree");

}

TEST(Generator, PrintAndPage) {

    std::string currentPath = std::filesystem::current_path().string();

    std::filesystem::create_directory(currentPath + "/testtree");

    // Create home template

    std::ofstream os;
    os.open(currentPath + "/testtree/home.html", std::ofstream::out | std::ofstream::trunc);
    os << R"(
<!DOCTYPE html>
<html>
<head>
<title><!-- print(/params/sitename) --> - <!-- print(@params/title) --></title>
</head>
<body>

<h1><!-- print(@params/title) --></h1>

<!-- if(path "^/$") -->
<p>Welcome to the homepage</p>
<!-- endif -->

<!-- template(category post) -->

</body>
</html>
)";
    os.close();

    // Create category template

    os.open(currentPath + "/testtree/category.html", std::ofstream::out | std::ofstream::trunc);
    os << R"(
<div class="category"><!-- print(params/name) --></div>
)";
    os.close();

    // Create post template

    os.open(currentPath + "/testtree/post.html", std::ofstream::out | std::ofstream::trunc);
    os << R"(
<div class="post"><!-- print(@params/title) --></div>
)";
    os.close();

    // Create category file

    std::filesystem::create_directory(currentPath + "/testtree/blog.category/");
    os.open(currentPath + "/testtree/blog.category/params.json", std::ofstream::out | std::ofstream::trunc);
    os << R"(
{
    "name": "This is a category name"
}
)";
    os.close();

    // Create post file

    std::filesystem::create_directory(currentPath + "/testtree/blog.category/post.post");
    os.open(currentPath + "/testtree/blog.category/post.post/params.json", std::ofstream::out | std::ofstream::trunc);
    os << R"(
{
    "title": "This is a post title"
}
)";
    os.close();

    // Create params file

    os.open(currentPath + "/testtree/params.json", std::ofstream::out | std::ofstream::trunc);
    os << R"(
{
    "sitename": "Test website"
}
)";
    os.close();

    Tree t(currentPath + "/testtree");
    t.build();

    std::vector<std::string> pages;
    auto n = t.getRoot()->get("/blog/post", &pages);

    EXPECT_EQ(Generator::Generate(n, t.getRoot(), "home", &pages),

            R"(
<!DOCTYPE html>
<html>
<head>
<title>Test website - This is a post title</title>
</head>
<body>

<h1>This is a post title</h1>

<!-- if(path "^/$") -->
<p>Welcome to the homepage</p>
<!-- endif -->


<div class="category">This is a category name</div>

<div class="post">This is a post title</div>


</body>
</html>
)");

    std::filesystem::remove_all(currentPath + "/testtree");

}