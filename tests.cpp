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

    auto n = t.getRoot()->getFirst("textfile.txt");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->getValue(), "test value");

    n = t.getRoot()->getFirst("/folder/anotherfile.txt");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->getValue(), "test value 2");

    std::filesystem::remove_all(currentPath + "/testtree");

    t.build();

    n = t.getRoot()->getFirst("textfile.txt");
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

    auto n = t.getRoot()->getFirst("/testjson.json/key");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->getValue(), "value");

    n = t.getRoot()->getFirst("/testjson.json/object/foo/0");
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

    auto n = t.getRoot()->getFirst("testjson/key");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->getValue(), "value");
    EXPECT_EQ(n->getPath(), "/testjson.template.json/key");

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

<p>Welcome to the homepage</p>

<!-- template(category post) -->

</body>
</html>
)";
    os.close();

    // Create category template

    os.open(currentPath + "/testtree/category.html", std::ofstream::out | std::ofstream::trunc);
    os << R"(<div class="category"><!-- print(params/name) --></div>
)";
    os.close();

    // Create post template

    os.open(currentPath + "/testtree/post.html", std::ofstream::out | std::ofstream::trunc);
    os << R"(<div class="post"><!-- print(@params/title) --></div>
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
    "sitename": "Test website",
    "showgreetings": true
}
)";
    os.close();

    Tree t(currentPath + "/testtree");
    t.build();

    auto n = t.getRoot()->getFirst("/blog/post");

    EXPECT_EQ(Generator::Generate(n, t.getRoot(), "home", nullptr, "/"),

            R"(
<!DOCTYPE html>
<html>
<head>
<title>Test website - This is a post title</title>
</head>
<body>

<h1>This is a post title</h1>

<p>Welcome to the homepage</p>

<div class="category">This is a category name</div>
<div class="post">This is a post title</div>


</body>
</html>
)");

    std::filesystem::remove_all(currentPath + "/testtree");

}

TEST(Generator, PrintMultipleNodes) {

    std::string currentPath = std::filesystem::current_path().string();

    std::filesystem::create_directory(currentPath + "/testtree");

    // Create home template

    std::ofstream os;
    os.open(currentPath + "/testtree/home.html", std::ofstream::out | std::ofstream::trunc);
    os << R"(Items:
<!-- print(items/.* item) -->)";
    os.close();

    // Create item template

    os.open(currentPath + "/testtree/item.html", std::ofstream::out | std::ofstream::trunc);
    os << R"(Item number <!-- print(number) -->
)";
    os.close();

    // Create items

    std::filesystem::create_directory(currentPath + "/testtree/items");

    os.open(currentPath + "/testtree/items/1.json", std::ofstream::out | std::ofstream::trunc);
    os << R"({"number":1})";
    os.close();
    os.open(currentPath + "/testtree/items/2.json", std::ofstream::out | std::ofstream::trunc);
    os << R"({"number":2})";
    os.close();
    os.open(currentPath + "/testtree/items/3.json", std::ofstream::out | std::ofstream::trunc);
    os << R"({"number":3})";
    os.close();
    os.open(currentPath + "/testtree/items/4.json", std::ofstream::out | std::ofstream::trunc);
    os << R"({"number":4})";
    os.close();
    os.open(currentPath + "/testtree/items/5.json", std::ofstream::out | std::ofstream::trunc);
    os << R"({"number":5})";
    os.close();

    Tree t(currentPath + "/testtree");
    t.build();

    auto n = t.getRoot()->getFirst("/");

    ASSERT_NE(n, nullptr);
    EXPECT_EQ(Generator::Generate(n, n, "home", nullptr, "/"),
              R"(Items:
Item number 1
Item number 2
Item number 3
Item number 4
Item number 5
)");

    std::filesystem::remove_all(currentPath + "/testtree");

}

TEST(Generator, If) {

    std::string currentPath = std::filesystem::current_path().string();

    std::filesystem::create_directory(currentPath + "/testtree");

    // Create home template

    std::ofstream os;
    os.open(currentPath + "/testtree/home.html", std::ofstream::out | std::ofstream::trunc);
    os << R"(Conditional output: <!-- if(/params/variable ^hello$) -->Condition is true, <!-- print(/params/variable) --><!-- endif() -->)";

    os.close();

    // Create item template

    os.open(currentPath + "/testtree/params.json", std::ofstream::out | std::ofstream::trunc);
    os << R"({"variable": "hello"})";
    os.close();

    Tree t(currentPath + "/testtree");
    t.build();

    EXPECT_EQ(Generator::Generate(t.getRoot(), t.getRoot(), "home", nullptr, "/"),
              R"(Conditional output: Condition is true, hello)");

    os.open(currentPath + "/testtree/params.json", std::ofstream::out | std::ofstream::trunc);
    os << R"({"variable": "world"})";
    os.close();

    t.build();

    EXPECT_EQ(Generator::Generate(t.getRoot(), t.getRoot(), "home", nullptr, "/"),
              R"(Conditional output: )");

    std::filesystem::remove_all(currentPath + "/testtree");
}

TEST(Generator, PathVariable) {

    std::string currentPath = std::filesystem::current_path().string();

    std::filesystem::create_directory(currentPath + "/testtree");

    // Create home template

    std::ofstream os;
    os.open(currentPath + "/testtree/home.html", std::ofstream::out | std::ofstream::trunc);
    os << R"(<!-- print(^obj\d+.*$ object) -->)";
    os.close();

    // Create object template

    os.open(currentPath + "/testtree/object.html", std::ofstream::out | std::ofstream::trunc);
    os << R"(<!-- print($FULLPATH) -->)";
    os.close();

    // Create object

    os.open(currentPath + "/testtree/obj1.object.json", std::ofstream::out | std::ofstream::trunc);
    os << R"({"variable": "hello"})";
    os.close();

    os.open(currentPath + "/testtree/obj2.object.json", std::ofstream::out | std::ofstream::trunc);
    os << R"({"variable": "hello"})";
    os.close();

    os.open(currentPath + "/testtree/obj3.object.json", std::ofstream::out | std::ofstream::trunc);
    os << R"({"variable": "hello"})";
    os.close();

    Tree t(currentPath + "/testtree");
    t.build();

    auto n = t.getRoot();

    EXPECT_EQ(Generator::Generate(n, n, "home", nullptr, "/"),
              R"(/obj3/obj2/obj1)");

    std::filesystem::remove_all(currentPath + "/testtree");

}