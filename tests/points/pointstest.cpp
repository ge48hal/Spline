
#include <vector>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "points/points.h"
class PointsTest : public ::testing::Test {
protected:
    Points points1;   // default constructor
    Points points2;   // constructed with size_t
    Points points3;   // constructed with two vectors

    std::string testfile = "../tests/test_cases/points.txt";
    std::shared_ptr<spdlog::logger> test_logger = spdlog::get("test_logger");

    void SetUp() override {
        spdlog::set_level(spdlog::level::info);
        points2 = Points(10);  // reserve(10)
        points3 = Points(
            std::vector<double>{1.0, 2.0, 3.0},
            std::vector<double>{0.5, 1.5, 2.5}
        );

        test_logger->info("PointsTest setup complete");
    }

    void TearDown() override {
        test_logger->info("PointsTest teardown complete\n\n");
    }
};

TEST_F(PointsTest, DefaultConstructor) {
    test_logger->info("Points - Default constructor test");
    ASSERT_EQ(points1.size(), 0u) << "Default constructor size test failed";
    ASSERT_TRUE(points1.get_epsilon().empty()) << "Default constructor epsilon vector empty test failed";
    ASSERT_TRUE(points1.get_sigma().empty()) << "Default constructor sigma vector empty test failed";
    test_logger->info("Points - Default constructor test passed");
}

TEST_F(PointsTest, VectorConstructor) {
    test_logger->info("Points - vector constructor test");
    ASSERT_EQ(points3.size(), 3u) << "Vector constructor size test failed";
    test_logger->info("Points - vector constructor test passed");
}

TEST_F(PointsTest, Pushback_Test1) {
    test_logger->info("Points - push_back test1");

    EXPECT_EQ(points1.size(), 0u);
    for(size_t i = 0; i < 10 ; ++ i){
        points1.push_back(static_cast<double>(i), static_cast<double>(i) * 2.0);
    }

    const std::vector<double>& eps = points1.get_epsilon();
    const std::vector<double>& sig = points1.get_sigma();

    for(size_t i = 0 ; i < 10 ; ++ i){
        EXPECT_DOUBLE_EQ(eps[i], static_cast<double>(i));
        EXPECT_DOUBLE_EQ(sig[i], static_cast<double>(i) * 2.0);
    }

    EXPECT_EQ(points1.get_epsilon().size(), points1.get_sigma().size());

    if(::testing::Test::HasFailure()) {
        test_logger->info("Points - push_back test1 failed");
    } else {
        test_logger->info("Points - push_back test1 passed");
    }
}

TEST_F(PointsTest, Pushback_Test2) {
    test_logger->info("Points - push_back test2");

    EXPECT_EQ(points2.size(), 0u);
    for(size_t i = 0; i < 10 ; ++ i){
        points2.push_back(static_cast<double>(i), static_cast<double>(i) * 2.0);
    }

    const std::vector<double>& eps = points2.get_epsilon();
    const std::vector<double>& sig = points2.get_sigma();


    for(size_t i = 0 ; i < 10 ; ++ i){
        EXPECT_DOUBLE_EQ(eps[i], static_cast<double>(i));
        EXPECT_DOUBLE_EQ(sig[i], static_cast<double>(i) * 2.0);
    }

    EXPECT_EQ(points2.get_epsilon().size(), points2.get_sigma().size());

    if(::testing::Test::HasFailure()) {
        test_logger->info("Points - push_back test2 failed");
    } else {
        test_logger->info("Points - push_back test2 passed");
    }
}

TEST_F(PointsTest, Pushback_Test3){
    test_logger->info("Points - push_back test3");

    EXPECT_EQ(points3.size(), 3u);

    for(size_t i = 0; i < 8 ; ++ i){
        points3.push_back(static_cast<double>(i), static_cast<double>(i) * 2.0);
    }

    const std::vector<double>& eps = points3.get_epsilon();
    const std::vector<double>& sig = points3.get_sigma();

    EXPECT_DOUBLE_EQ(eps[0], 1.0);
    EXPECT_DOUBLE_EQ(eps[1], 2.0);
    EXPECT_DOUBLE_EQ(eps[2], 3.0);

    EXPECT_DOUBLE_EQ(sig[0], 0.5);
    EXPECT_DOUBLE_EQ(sig[1], 1.5);
    EXPECT_DOUBLE_EQ(sig[2], 2.5);

    for(size_t i = 3 ; i < 11 ; ++ i){
        EXPECT_DOUBLE_EQ(eps[i], static_cast<double>(i-3));
        EXPECT_DOUBLE_EQ(sig[i], static_cast<double>(i-3) * 2.0);
    }
    EXPECT_EQ(points3.get_epsilon().size(), points3.get_sigma().size());

    if(::testing::Test::HasFailure()) {
        test_logger->info("Points - push_back test3 failed");
    } else {
        test_logger->info("Points - push_back test passed");
    }
}

TEST_F(PointsTest, ClearTest1) {
    test_logger->info("Points - clear() test1");

    points1.push_back(1.0, 2.0);
    points1.push_back(3.0, 4.0);

    points1.clear();

    ASSERT_EQ(points1.size(), 0u)<< "Clear method size test1 failed";

    test_logger->info("Points - clear() test1 passed");
}

TEST_F(PointsTest, ClearTest2) {
    test_logger->info("Points - clear() test2");

    points3.clear();

    ASSERT_EQ(points3.size(), 0u)<< "Clear method size test2 failed";

    test_logger->info("Points - clear() test2 passed");
}


TEST_F(PointsTest, Insert_Test1){
    test_logger->info("Points - insert_range() test1");

    // source vectors
    std::vector<double> eps_src{0.0, 0.1, 0.2, 0.3};
    std::vector<double> sig_src{1.0, 1.1, 1.2, 1.3};

    EXPECT_EQ(points2.size(), 0u);

    points2.insert_range(eps_src, sig_src, 0, eps_src.size());

    const std::vector<double>& eps = points2.get_epsilon();
    const std::vector<double>& sig = points2.get_sigma();

    // check values
    EXPECT_DOUBLE_EQ(eps[0], 0.0);
    EXPECT_DOUBLE_EQ(sig[0], 1.0);

    EXPECT_DOUBLE_EQ(eps[1], 0.1);
    EXPECT_DOUBLE_EQ(sig[1], 1.1);

    EXPECT_DOUBLE_EQ(eps[2], 0.2);
    EXPECT_DOUBLE_EQ(sig[2], 1.2);

    EXPECT_DOUBLE_EQ(eps[3], 0.3);
    EXPECT_DOUBLE_EQ(sig[3], 1.3);

    if(::testing::Test::HasFailure()) {
        test_logger->info("Points - insert_range() test1 failed");
    } else {
        test_logger->info("Points - insert_range() test1 passed");
    }
}

TEST_F(PointsTest, Insert_Test2){
    test_logger->info("Points - insert_range() test2");

    // source vectors
    std::vector<double> eps_src{0.0, 0.1, 0.2, 0.3, 0.4};
    std::vector<double> sig_src{1.0, 1.1, 1.2, 1.3, 1.4};

    EXPECT_EQ(points3.size(), 3u);

    points3.insert_range(eps_src, sig_src, 0, eps_src.size());

    const std::vector<double>& eps = points3.get_epsilon();
    const std::vector<double>& sig = points3.get_sigma();

    // check original values
    EXPECT_DOUBLE_EQ(eps[0], 1.0);
    EXPECT_DOUBLE_EQ(sig[0], 0.5);

    EXPECT_DOUBLE_EQ(eps[1], 2.0);
    EXPECT_DOUBLE_EQ(sig[1], 1.5);

    EXPECT_DOUBLE_EQ(eps[2], 3.0);
    EXPECT_DOUBLE_EQ(sig[2], 2.5);

    // check inserted values
    for(size_t i = 3 ; i < 8 ; ++ i){
        EXPECT_DOUBLE_EQ(eps[i], eps_src[i - 3]);
        EXPECT_DOUBLE_EQ(sig[i], sig_src[i - 3]);
    }

    if(::testing::Test::HasFailure()) {
        test_logger->info("Points - insert_range() test2 failed");
    } else {
        test_logger->info("Points - insert_range() test2 passed");
    }
}

TEST_F(PointsTest, InsertWrongVectorTest1){
    test_logger->info("Points - insert_range() wrong vector size test1");

    // source vectors with different sizes
    const std::vector<double>& eps_src{0.0, 0.1, 0.2};
    const std::vector<double>& sig_src{1.0, 1.1};

    EXPECT_EQ(points2.size(), 0u);

    points2.insert_range(eps_src, sig_src, 0, eps_src.size());

    // size should remain unchanged
    EXPECT_EQ(points2.size(), 0u) << "Size after insert_range with wrong vector sizes test failed";

    test_logger->info("Points - insert_range() wrong vector size test1 passed");
}

TEST_F(PointsTest, InsertWrongVectorTest2){
    test_logger->info("Points - insert_range() wrong vector size test2");

    // source vectors with different sizes
    const std::vector<double>& eps_src{0.0, 0.1, 0.2, 0.3};
    const std::vector<double>& sig_src{1.0, 1.1};

    EXPECT_EQ(points3.size(), 3u);

    points3.insert_range(eps_src, sig_src, 0, eps_src.size());

    // size should remain unchanged
    EXPECT_EQ(points3.size(), 3u) << "Size after insert_range with wrong vector sizes test failed";

    test_logger->info("Points - insert_range() wrong vector size test2 passed");
}


TEST_F(PointsTest, ChangePointTest) {
    test_logger->info("Points - change_point() test");

    points3.change_point(1, 9.9, 8.8);

    const std::vector<double>& eps = points3.get_epsilon();
    const std::vector<double>& sig = points3.get_sigma();

    EXPECT_DOUBLE_EQ(eps[1], 9.9);
    EXPECT_DOUBLE_EQ(sig[1], 8.8);

    test_logger->info("Points - change_point() test passed");
}

TEST_F(PointsTest, ChangeWrongPointTest1){
    test_logger->info("Points - change_point() wrong index test1");

    points2.change_point(5, 7.7, 6.6); // index 5 is out of bounds

    const std::vector<double>& eps = points2.get_epsilon();
    const std::vector<double>& sig = points2.get_sigma();

    EXPECT_EQ(points2.size(), 0u) << "wrong index test1 failed";

    test_logger->info("Points - change_point() wrong index test1 passed");
}

TEST_F(PointsTest, ChangeWrongPointTest2){
    test_logger->info("Points - change_point() wrong index test2");
    points3.change_point(10, 7.7, 6.6); // index 10 is out of bounds

    const std::vector<double>& eps = points3.get_epsilon();
    const std::vector<double>& sig = points3.get_sigma();

    // original values should remain unchanged
    EXPECT_DOUBLE_EQ(eps[0], 1.0);
    EXPECT_DOUBLE_EQ(sig[0], 0.5);

    EXPECT_DOUBLE_EQ(eps[1], 2.0);
    EXPECT_DOUBLE_EQ(sig[1], 1.5);

    EXPECT_DOUBLE_EQ(eps[2], 3.0);
    EXPECT_DOUBLE_EQ(sig[2], 2.5);

    test_logger->info("Points - change_point() wrong index test2 passed");
}