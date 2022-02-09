#include "gtest/gtest.h"
#include "VirtualMachine.h"
#include "FileHelper.h"
#include <algorithm>

std::vector<std::string> GetExternalTests()
{
    return FileParser::GetFilesInPathRecursive(CommandLineParser::GlobalCMDParser->GetArgument("-externalTestDir"));
}

bool IsFibonacci(Registers& regs)
{
    return regs.B == 3
        && regs.C == 5
        && regs.D == 8
        && regs.E == 13
        && regs.H == 21
        && regs.L == 34;
}

class ExternalTestFixture : public testing::TestWithParam<std::string> {
    // You can implement all the usual fixture class members here.
    // To access the test parameter, call GetParam() from class
    // TestWithParam<T>.
};

TEST_P(ExternalTestFixture, Main) {
    // Inside a test, access the test parameter with the GetParam() method
    // of the TestWithParam<T> class:
    std::string test = GetParam();

    std::vector<char> romBlob;
    if (!FileParser::Read(test, romBlob))
    {
        FAIL();
    }

    VirtualMachine vm;

    vm.Load("External Test", romBlob.data(), static_cast<uint32_t>(romBlob.size()));

    vm.StopOnInstruction(0x40);

    uint32_t frameCount = 0;
    bool stopReached = false;
    while (!stopReached)
    {
        EmulatorInputs::InputState inputState;
        vm.Step(inputState);
        stopReached = vm.HasReachedInstruction();
        frameCount++;
    }

    EXPECT_TRUE(IsFibonacci(vm.GetRegisters()));
}

std::string GetTestName(testing::TestParamInfo<std::string> param)
{
    std::string fileName = FileParser::GetFileNameFromPath(param.param);
    std::replace(fileName.begin(), fileName.end(), '_', 'X');
    std::replace(fileName.begin(), fileName.end(), '-', 'X');
    return fileName;
}

INSTANTIATE_TEST_CASE_P(ExternalTests,
    ExternalTestFixture,
    testing::ValuesIn(GetExternalTests()), GetTestName);