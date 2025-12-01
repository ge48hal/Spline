#include <vector>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "inputreader/prep.h"

class PrepTest : public ::testing::Test {
protected:

    Points lm1;
    Points lm2;

    std::string testfile = "../tests/test_cases/points.txt";
    std::shared_ptr<spdlog::logger> test_logger = spdlog::get("test_logger");

    void SetUp() override {
        spdlog::set_level(spdlog::level::info);
        lm1 = Points(
            std::vector<double>{0.0, 0.5, 1.0, 1.5, 2.0},
            std::vector<double>{2.0, 1.5, 1.0, 0.5, 0.0}
        );
        lm2 = Points(
            std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0},
            std::vector<double>{-4.0, -3.0, -2.0, -1.0, 0.0}
        );
        test_logger->info("PrepTest setup complete");
    }

    void TearDown() override {
        test_logger->info("PrepTest teardown complete\n\n");
    }

};

TEST_F(PrepTest, PrePolyFunctionTest1) {
    test_logger->info("Prep - _preprocess_polyline function test1");

    double eps_cut = 1.2;

    std::pair<std::size_t,double> interp = preprocess::_preprocess_polyline(eps_cut, lm1);

    const auto& eps = lm1.get_epsilon();
    const auto& sig = lm1.get_sigma();

    EXPECT_EQ(interp.first, 3u);
    double expected_sig = sig[2] + (eps_cut - eps[2]) / (eps[3] - eps[2]) * (sig[3] - sig[2]);
    EXPECT_DOUBLE_EQ(interp.second, expected_sig);

    test_logger->info("Prep - _preprocess_polyline function test1 passed");

}

TEST_F(PrepTest, PrePolyFunctionTest2) {
    test_logger->info("Prep - _preprocess_polyline function test2");

    double eps_cut = 2.5;

    std::pair<std::size_t,double> interp = preprocess::_preprocess_polyline(eps_cut, lm2);

    const auto& eps = lm2.get_epsilon();
    const auto& sig = lm2.get_sigma();

    EXPECT_EQ(interp.first, 3u);
    double expected_sig = sig[2] + (eps_cut - eps[2]) / (eps[3] - eps[2]) * (sig[3] - sig[2]);
    EXPECT_DOUBLE_EQ(interp.second, expected_sig);

    test_logger->info("Prep - _preprocess_polyline function test2 passed");
}

TEST_F(PrepTest, PrePolyInvalidTest1){
    test_logger->info("Prep - _preprocess_polyline function invalid test1");

    double eps_cut = -1.0;

    std::pair<std::size_t,double> interp = preprocess::_preprocess_polyline(eps_cut, lm1);

    EXPECT_EQ(interp.first, 0u);
    EXPECT_DOUBLE_EQ(interp.second, 0.0);

    test_logger->info("Prep - _preprocess_polyline function unvalid test1 passed");
}

TEST_F(PrepTest, PrePolyInvalidTest2){
    test_logger->info("Prep - _preprocess_polyline function invalid test2");

    double eps_cut = 5.0;

    std::pair<std::size_t,double> interp = preprocess::_preprocess_polyline(eps_cut, lm2);

    EXPECT_EQ(interp.first, 0u);
    EXPECT_DOUBLE_EQ(interp.second, 0.0);

    test_logger->info("Prep - _preprocess_polyline function unvalid test2 passed");
}

TEST_F(PrepTest, PrePolyInvalidTest3){
    test_logger->info("Prep - _preprocess_polyline function invalid test3");

    double eps_cut = 1.0;

    Points lm3;

    std::pair<std::size_t,double> interp = preprocess::_preprocess_polyline(eps_cut, lm3);
    EXPECT_EQ(interp.first, 0u);
    EXPECT_DOUBLE_EQ(interp.second, 0.0);

    test_logger->info("Prep - _preprocess_polyline function unvalid test3 passed");
}

TEST_F(PrepTest, PrePolyExcatMatchTest){
    test_logger->info("Prep - _preprocess_polyline function exact match test");

    double eps_cut = 2.0;

    std::pair<std::size_t,double> interp = preprocess::_preprocess_polyline(eps_cut, lm2);

    EXPECT_EQ(interp.first, 2u) << "expected : 2, got: " << interp.first;
    EXPECT_DOUBLE_EQ(interp.second, -2.0) << "expected : -2.0, got: " << interp.second;

    test_logger->info("Prep - _preprocess_polyline function exact match test passed");
}

TEST_F(PrepTest, PreTest1){
    test_logger->info("Prep - prep function test1");

    double eps_cut = 1.25;

    Points prepped = preprocess::prep(eps_cut, lm1);

    const auto& eps = prepped.get_epsilon();
    const auto& sig = prepped.get_sigma();

    EXPECT_EQ(prepped.size(), 6u); // 0.0, 0.5, 1.0 + intersection + projection + origin

    // Check last three points
    EXPECT_DOUBLE_EQ(eps[3], eps_cut) << "expected : " << eps_cut << ", got: " << eps[3];
    double expected_sig = lm1.get_sigma()[2] + (eps_cut - lm1.get_epsilon()[2]) / (lm1.get_epsilon()[3] - lm1.get_epsilon()[2]) * (lm1.get_sigma()[3] - lm1.get_sigma()[2]);
    EXPECT_DOUBLE_EQ(sig[3], expected_sig) << "expected : " << expected_sig << ", got: " << sig[3];
    EXPECT_DOUBLE_EQ(eps[4], eps_cut) ;
    EXPECT_DOUBLE_EQ(sig[4], 0.0);
    EXPECT_DOUBLE_EQ(eps[5], lm1.get_epsilon()[0]) ;
    EXPECT_DOUBLE_EQ(sig[5], lm1.get_sigma()[0]) ;

    test_logger->info("Prep - prep function test1 passed");
}

TEST_F(PrepTest, PreTest2){
    test_logger->info("Prep - prep function test2");

    double eps_cut = 2.5;

    Points prepped = preprocess::prep(eps_cut, lm2);

    const auto& eps = prepped.get_epsilon();
    const auto& sig = prepped.get_sigma();

    EXPECT_EQ(prepped.size(), 6u); // 0.0, 0.5, 1.0 + intersection + projection + origin

    //check three points before last three
    EXPECT_DOUBLE_EQ(eps[0], 0.0) ;
    EXPECT_DOUBLE_EQ(sig[0], -4.0) ;
    EXPECT_DOUBLE_EQ(eps[1], 1.0) ;
    EXPECT_DOUBLE_EQ(sig[1], -3.0) ;
    EXPECT_DOUBLE_EQ(eps[2], 2.0) ;
    EXPECT_DOUBLE_EQ(sig[2], -2.0) ;
    
    // Check last three points
    EXPECT_DOUBLE_EQ(eps[3], eps_cut) ;
    double expected_sig = lm2.get_sigma()[2] + (eps_cut - lm2.get_epsilon()[2]) / (lm2.get_epsilon()[3] - lm2.get_epsilon()[2]) * (lm2.get_sigma()[3] - lm2.get_sigma()[2]);
    EXPECT_DOUBLE_EQ(sig[3], expected_sig) ;
    EXPECT_DOUBLE_EQ(eps[4], eps_cut) ;
    EXPECT_DOUBLE_EQ(sig[4], 0.0) ;
    EXPECT_DOUBLE_EQ(eps[5], lm2.get_epsilon()[0]) ;
    EXPECT_DOUBLE_EQ(sig[5], lm2.get_sigma()[0]) ;

    test_logger->info("Prep - prep function test2 passed");
}

TEST_F(PrepTest, PreInvalidTest1){
    test_logger->info("Prep - prep function invalid test1");

    double eps_cut = -1.0;

    Points prepped = preprocess::prep(eps_cut, lm1);
    EXPECT_EQ(prepped.size(), 0u) << "expected size 0, got: " << prepped.size();

    test_logger->info("Prep - prep function unvalid test1 passed");
}

TEST_F(PrepTest, PreInvalidTest2){
    test_logger->info("Prep - prep function invalid test2");

    double eps_cut = 5.0;

    Points prepped = preprocess::prep(eps_cut, lm2);

    EXPECT_EQ(prepped.size(), 0u)<< "expected size 0, got: " << prepped.size();

    test_logger->info("Prep - prep function unvalid test2 passed");
}