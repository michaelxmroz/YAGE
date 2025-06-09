#include "gtest/gtest.h"
#include "VirtualMachine.h"
#include "FileHelper.h"
#include <algorithm>

#define MOONEYE_STOP_INSTR 0x40

void* AllocFunc(uint32_t size)
{
    return new uint8_t[size];
}

void FreeFunc(void* ptr)
{
    delete[] reinterpret_cast<uint8_t*>(ptr);
}

std::vector<std::string> GetExternalTests()
{
    auto tests = FileParser::GetFilesInPathRecursive(CommandLineParser::GlobalCMDParser->GetArgument("-externalTestDir"));
    return tests;
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
    VirtualMachine* emu = static_cast<VirtualMachine*>(Emulator::Create(AllocFunc, FreeFunc));

    emu->Load(test.c_str(), romBlob.data(), static_cast<uint32_t>(romBlob.size()));

    emu->StopOnInstruction(MOONEYE_STOP_INSTR);

    bool stopReached = false;
    //If a test takes longer than 2 sec, abort
    constexpr int MAX_FRAMES = 120;
    int currentFrames = 0;

    while (!stopReached && currentFrames < MAX_FRAMES)
    {
        EmulatorInputs::InputState inputState;
        emu->Step(inputState, 16.67, false);
        stopReached = emu->HasReachedInstruction(MOONEYE_STOP_INSTR);
        currentFrames++;
    }

    EXPECT_TRUE(IsFibonacci(emu->GetRegisters()));
    Emulator::Delete(emu);
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