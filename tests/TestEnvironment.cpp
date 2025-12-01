#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <fstream>

class TestEnvironment: public ::testing::Environment {
    void SetUp() override {
        const std::string logFile = "logs/test.txt";

        std::remove(logFile.c_str());

        auto test_logger = spdlog::basic_logger_mt("test_logger", "logs/test.txt");
        spdlog::set_default_logger(test_logger);
    }
};


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Add the global environment
    ::testing::AddGlobalTestEnvironment(new TestEnvironment);

    return RUN_ALL_TESTS();
}
