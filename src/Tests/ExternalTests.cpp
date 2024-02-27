#include "gtest/gtest.h"
#include "VirtualMachine.h"
#include "FileHelper.h"
#include <algorithm>

#define MOONEYE_STOP_INSTR 0x40

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

    vm.Load("External Tests", romBlob.data(), static_cast<uint32_t>(romBlob.size()));

    vm.StopOnInstruction(MOONEYE_STOP_INSTR);

    bool stopReached = false;
    while (!stopReached)
    {
        EmulatorInputs::InputState inputState;
        vm.Step(inputState, 16.67);
        stopReached = vm.HasReachedInstruction(MOONEYE_STOP_INSTR);
    }

    EXPECT_TRUE(IsFibonacci(vm.GetRegisters()));
}

std::string GetTestName(testing::TestParamInfo<std::string> param)
{
    std::string fileName = FileParser::GetFileNameFromPath(param.param);
    std::replace(fileName.begin(), fileName.end(), '_', 'x');
    std::replace(fileName.begin(), fileName.end(), '-', 'x');
    return fileName;
}

INSTANTIATE_TEST_CASE_P(ExternalTests,
    ExternalTestFixture,
    testing::ValuesIn(GetExternalTests()), GetTestName);