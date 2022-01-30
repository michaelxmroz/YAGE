#include "gtest/gtest.h"
#include <string>
#include <vector>
#include "FileHelper.h"

int main(int argc, char** argv) 
{
    CommandLineParser::GlobalCMDParser = new CommandLineParser(argc, argv);
	::testing::InitGoogleTest(&argc, argv);

    uint32_t retVal = RUN_ALL_TESTS();

    delete CommandLineParser::GlobalCMDParser;
    return retVal;
}
