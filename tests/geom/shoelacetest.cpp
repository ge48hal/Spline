#include <vector>
#include <spdlog/spdlog.h>
#include <gtest/gtest.h>

#include "inputreader/prep.h"
#include "geom/shoelace.h"

class ShoelaceTest : public ::testing::Test {
protected:
    Points points1;
    Points points2;

    std::shared_ptr<spdlog::logger> test_logger = spdlog::get("test_logger");

    void SetUp() override {
        spdlog::set_level(spdlog::level::info);

        // Define a simple trapezoid 
        points1 = Points(
            std::vector<double>{0.0, 2.0, 4.0, 6.0, 8.0, 10.0},
            std::vector<double>{0.0, 2.0, 2.0, 2.0, 2.0, 2.0}
        );

        // Define a triangle 
        points2 = Points(
            std::vector<double>{0.0, 2.0,4.0, 6.0, 8.0,10.0},
            std::vector<double>{0.0, 2.0, 4.0, 6.0, 8.0, 10.0}
        );

        test_logger->info("ShoelaceTest setup complete");
    }

    void TearDown() override {
        test_logger->info("ShoelaceTest teardown complete\n\n");
    }
};

TEST_F(ShoelaceTest, AreaCalculationTest1) {
    test_logger->info("Shoelace - Area calculation test1 (trapezoid)");

    Points polygon1 = preprocess::prep(4.0, points1);

    double area = geom::Shoelace::calculateArea(polygon1);
    std::pair<double,double> area_simd = geom::Shoelace::calculateAreaAndMomentum(polygon1);

    EXPECT_DOUBLE_EQ(area, 6.0) << "Area calculation for trapezoid failed";
    EXPECT_DOUBLE_EQ(area_simd.first, 6.0) << "Area calculation SIMD mismatch";

    test_logger->info("Shoelace - Area calculation test1 passed");
}

TEST_F(ShoelaceTest, AreaCalculationTest2) {
    test_logger->info("Shoelace - Area calculation test2 (triangle)");

    Points polygon2 = preprocess::prep(2.0, points2);

    double area = geom::Shoelace::calculateArea(polygon2);
    std::pair<double,double> area_simd = geom::Shoelace::calculateAreaAndMomentum(polygon2);

    EXPECT_DOUBLE_EQ(area, 2.0) << "Area calculation for triangle failed";
    EXPECT_DOUBLE_EQ(area_simd.first, 2.0) << "Area calculation SIMD mismatch";

    test_logger->info("Shoelace - Area calculation test2 passed");
}

TEST_F(ShoelaceTest, AreaCalculationTest3){
    test_logger -> info("Shoelace - Area calculation test3 (trapezoid) complex");

    Points polygon1 = preprocess::prep(3.0, points1);
    Points polygon2 = preprocess::prep(5.0, points1);
    Points polygon3 = preprocess::prep(7.0, points1);
    Points polygon4 = preprocess::prep(9.0, points1);


    double area1 = geom::Shoelace::calculateArea(polygon1);
    std::pair<double,double> area_simd1 = geom::Shoelace::calculateAreaAndMomentum(polygon1);
    EXPECT_DOUBLE_EQ(area1, 4.0) << "expected area 4.0 , got " << area1;
    EXPECT_DOUBLE_EQ(area_simd1.first, 4.0) << "Area calculation SIMD mismatch";
    polygon1.print();
    double area2 = geom::Shoelace::calculateArea(polygon2);
    std::pair<double,double> area_simd2 = geom::Shoelace::calculateAreaAndMomentum(polygon2);
    EXPECT_DOUBLE_EQ(area2, 8.0) << "expected area 8.0 , got " << area2;
    EXPECT_DOUBLE_EQ(area_simd2.first, 8.0) << "Area calculation SIMD mismatch";
    double area3 = geom::Shoelace::calculateArea(polygon3);
    std::pair<double,double> area_simd3 = geom::Shoelace::calculateAreaAndMomentum(polygon3);
    EXPECT_DOUBLE_EQ(area3, 12.0) << "expected area 12.0 , got " << area3;
    EXPECT_DOUBLE_EQ(area_simd3.first, 12.0) << "Area calculation SIMD mismatch";
    double area4 = geom::Shoelace::calculateArea(polygon4);
    std::pair<double,double> area_simd4 = geom::Shoelace::calculateAreaAndMomentum(polygon4);
    EXPECT_DOUBLE_EQ(area4, 16.0) << "expected area 16.0 , got " << area4;
    EXPECT_DOUBLE_EQ(area_simd4.first, 16.0) << "Area calculation SIMD mismatch";
    test_logger->info("Shoelace - Area calculation test1 passed");
}


TEST_F(ShoelaceTest, AreaCalculationTest4){
    test_logger -> info("Shoelace - Area calculation test4 (triangle) complex");

    Points polygon1 = preprocess::prep(3.0, points2);
    Points polygon2 = preprocess::prep(5.0, points2);
    Points polygon3 = preprocess::prep(7.0, points2);
    Points polygon4 = preprocess::prep(9.0, points2);


    double area1 = geom::Shoelace::calculateArea(polygon1);
    std::pair<double,double> area_simd1 = geom::Shoelace::calculateAreaAndMomentum(polygon1);
    EXPECT_DOUBLE_EQ(area1, 4.5) << "expected area 4.5 , got " << area1;
    EXPECT_DOUBLE_EQ(area_simd1.first, 4.5) << "Area calculation SIMD mismatch";
    polygon1.print();
    double area2 = geom::Shoelace::calculateArea(polygon2);
    std::pair<double,double> area_simd2 = geom::Shoelace::calculateAreaAndMomentum(polygon2);
    EXPECT_DOUBLE_EQ(area2, 12.5) << "expected area 12.5 , got " << area2;
    EXPECT_DOUBLE_EQ(area_simd2.first, 12.5) << "Area calculation SIMD mismatch";
    double area3 = geom::Shoelace::calculateArea(polygon3);
    std::pair<double,double> area_simd3 = geom::Shoelace::calculateAreaAndMomentum(polygon3);
    EXPECT_DOUBLE_EQ(area3, 24.5) << "expected area 24.5 , got " << area3;
    EXPECT_DOUBLE_EQ(area_simd3.first, 24.5) << "Area calculation SIMD mismatch";
    double area4 = geom::Shoelace::calculateArea(polygon4);
    std::pair<double,double> area_simd4 = geom::Shoelace::calculateAreaAndMomentum(polygon4);
    EXPECT_DOUBLE_EQ(area4, 40.5) << "expected area 40.5 , got " << area4;
    EXPECT_DOUBLE_EQ(area_simd4.first, 40.5) << "Area calculation SIMD mismatch";
    test_logger->info("Shoelace - Area calculation test4 passed");
}

TEST_F(ShoelaceTest,AreaInvalidTest1){
    test_logger->info("Shoelace - Area calculation invalid test1 (less than 3 points)");

    Points polygon1 = preprocess::prep(-1.0, points1);
    Points polygon2 = preprocess::prep(11.0, points2);

    double area = geom::Shoelace::calculateArea(polygon1);
    std::pair<double,double> area_simd = geom::Shoelace::calculateAreaAndMomentum(polygon1);
    EXPECT_DOUBLE_EQ(area, 0.0) << "Area calculation for invalid polygon failed";
    EXPECT_DOUBLE_EQ(area_simd.first, 0.0) << "Area calculation SIMD mismatch for invalid polygon";

    double area2 = geom::Shoelace::calculateArea(polygon2);
    std::pair<double,double> area_simd2 = geom::Shoelace::calculateAreaAndMomentum(polygon2);
    EXPECT_DOUBLE_EQ(area2, 0.0) << "Area calculation for invalid polygon failed";
    EXPECT_DOUBLE_EQ(area_simd2.first, 0.0) << "Area calculation SIMD mismatch for invalid polygon";

    test_logger->info("Shoelace - Area calculation invalid test1 passed");
}

TEST_F(ShoelaceTest, MomentumCalculationTest1) {
    test_logger->info("Shoelace - Momentum calculation test1 (trapezoid)");

    Points polygon1 = preprocess::prep(4.0, points1);

    double momentum = geom::Shoelace::calculateMomentum(polygon1);
    std::pair<double,double> momentum_simd = geom::Shoelace::calculateAreaAndMomentum(polygon1);

    EXPECT_DOUBLE_EQ(momentum, 88.0/6.0) << "Momentum calculation for trapezoid failed";
    EXPECT_DOUBLE_EQ(momentum_simd.second, 88.0/6.0) << "Momentum calculation SIMD mismatch";

    test_logger->info("Shoelace - Momentum calculation test1 passed");
}

TEST_F(ShoelaceTest, MomentumCalculationTest2) {
    test_logger->info("Shoelace - Momentum calculation test2 (triangle)");

    Points polygon2 = preprocess::prep(2.0, points2);

    double momentum = geom::Shoelace::calculateMomentum(polygon2);
    std::pair<double,double> momentum_simd = geom::Shoelace::calculateAreaAndMomentum(polygon2);

    spdlog::info("Calculated momentum: {}", momentum);

    EXPECT_DOUBLE_EQ(momentum, 16.0 / 6.0) << "Momentum calculation for triangle failed";
    EXPECT_DOUBLE_EQ(momentum_simd.second, 16.0 / 6.0) << "Momentum calculation SIMD mismatch";
    test_logger->info("Shoelace - Momentum calculation test2 passed");
}

TEST_F(ShoelaceTest, MomentumCalculationTest3){
    test_logger -> info("Shoelace - Momentum calculation test3 (trapezoid) complex");

    Points polygon1 = preprocess::prep(3.0, points1);
    Points polygon2 = preprocess::prep(5.0, points1);
    Points polygon3 = preprocess::prep(7.0, points1);
    Points polygon4 = preprocess::prep(9.0, points1);

    double momentum1 = geom::Shoelace::calculateMomentum(polygon1);
    std::pair<double,double> momentum_simd1 = geom::Shoelace::calculateAreaAndMomentum(polygon1);
    EXPECT_DOUBLE_EQ(momentum1, 46.0/6.0) << "expected momentum 46.0/6.0 , got " << momentum1;
    EXPECT_DOUBLE_EQ(momentum_simd1.second, 46.0/6.0) << "Momentum calculation SIMD mismatch";

    double momentum2 = geom::Shoelace::calculateMomentum(polygon2);
    std::pair<double,double> momentum_simd2 = geom::Shoelace::calculateAreaAndMomentum(polygon2);
    EXPECT_DOUBLE_EQ(momentum2, 142.0/6.0) << "expected momentum 142.0/6.0 , got " << momentum2;
    EXPECT_DOUBLE_EQ(momentum_simd2.second, 142.0/6.0) << "Momentum calculation SIMD mismatch";

    double momentum3 = geom::Shoelace::calculateMomentum(polygon3);
    std::pair<double,double> momentum_simd3 = geom::Shoelace::calculateAreaAndMomentum(polygon3);
    EXPECT_DOUBLE_EQ(momentum3, 286.0/6.0) << "expected momentum 284.0/6.0 , got " << momentum3;
    EXPECT_DOUBLE_EQ(momentum_simd3.second, 286.0/6.0) << "Momentum calculation SIMD mismatch";

    double momentum4 = geom::Shoelace::calculateMomentum(polygon4);
    std::pair<double,double> momentum_simd4 = geom::Shoelace::calculateAreaAndMomentum(polygon4);
    EXPECT_DOUBLE_EQ(momentum4, 478.0/6.0) << "expected momentum 478.0/6.0 , got " << momentum4;
    EXPECT_DOUBLE_EQ(momentum_simd4.second, 478.0/6.0) << "Momentum calculation SIMD mismatch";

    test_logger->info("Shoelace - Momentum calculation test3 passed");
}

TEST_F(ShoelaceTest, MomentumCalculationTest4){
    test_logger -> info("Shoelace - Momentum calculation test4 (triangle) complex");

    Points polygon1 = preprocess::prep(3.0, points2);
    Points polygon2 = preprocess::prep(5.0, points2);
    Points polygon3 = preprocess::prep(7.0, points2);
    Points polygon4 = preprocess::prep(9.0, points2);

    double momentum1 = geom::Shoelace::calculateMomentum(polygon1);
    std::pair<double,double> momentum_simd1 = geom::Shoelace::calculateAreaAndMomentum(polygon1);
    EXPECT_DOUBLE_EQ(momentum1, 9.0) << "expected momentum 54.0/6.0 , got " << momentum1;
    EXPECT_DOUBLE_EQ(momentum_simd1.second, 9.0) << "Momentum calculation SIMD mismatch";

    double momentum2 = geom::Shoelace::calculateMomentum(polygon2);
    std::pair<double,double> momentum_simd2 = geom::Shoelace::calculateAreaAndMomentum(polygon2);
    EXPECT_DOUBLE_EQ(momentum2, 250.0/6.0) << "expected momentum 250.0/6.0 , got " << momentum2;
    EXPECT_DOUBLE_EQ(momentum_simd2.second, 250.0/6.0) << "Momentum calculation SIMD mismatch";

    double momentum3 = geom::Shoelace::calculateMomentum(polygon3);
    std::pair<double,double> momentum_simd3 = geom::Shoelace::calculateAreaAndMomentum(polygon3);
    EXPECT_DOUBLE_EQ(momentum3, 686.0/6.0) << "expected momentum 686.0/6.0 , got " << momentum3;
    EXPECT_DOUBLE_EQ(momentum_simd3.second, 686.0/6.0) << "Momentum calculation SIMD mismatch";

    double momentum4 = geom::Shoelace::calculateMomentum(polygon4);
    std::pair<double,double> momentum_simd4 = geom::Shoelace::calculateAreaAndMomentum(polygon4);
    EXPECT_DOUBLE_EQ(momentum4, 243.0) << "expected momentum 1458.0/6.0 , got " << momentum4;
    EXPECT_DOUBLE_EQ(momentum_simd4.second, 243.0) << "Momentum calculation SIMD mismatch";

    test_logger->info("Shoelace - Momentum calculation test4 passed");
}

TEST_F(ShoelaceTest,MomentumInvalidTest1){
    test_logger->info("Shoelace - Momentum calculation invalid test1 (less than 3 points)");

    Points polygon1 = preprocess::prep(-1.0, points1);
    Points polygon2 = preprocess::prep(11.0, points2);

    double momentum = geom::Shoelace::calculateMomentum(polygon1);
    std::pair<double,double> momentum_simd = geom::Shoelace::calculateAreaAndMomentum(polygon1);
    EXPECT_DOUBLE_EQ(momentum, 0.0) << "Momentum calculation for invalid polygon failed";
    EXPECT_DOUBLE_EQ(momentum_simd.second, 0.0) << "Momentum calculation SIMD mismatch for invalid polygon";

    double momentum2 = geom::Shoelace::calculateMomentum(polygon2);
    std::pair<double,double> momentum_simd2 = geom::Shoelace::calculateAreaAndMomentum(polygon2);
    EXPECT_DOUBLE_EQ(momentum2, 0.0) << "Momentum calculation for invalid polygon failed";
    EXPECT_DOUBLE_EQ(momentum_simd2.second, 0.0) << "Momentum calculation SIMD mismatch for invalid polygon";
    
    test_logger->info("Shoelace - Momentum calculation invalid test1 passed");
}