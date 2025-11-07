#include "gtest/gtest.h"
#include "FileHelper.h"
#include <algorithm>
#include <RewindController.h>
#include "VirtualMachine.h"

void* RewindAllocFunc(uint32_t size)
{
    return new uint8_t[size];
}

void RewindFreeFunc(void* ptr)
{
    delete[] reinterpret_cast<uint8_t*>(ptr);
}

void FillWithPseudoRandomData(std::vector<uint8_t>& data, size_t size, unsigned int seed)
{
	data.resize(size);
	srand(seed);
	for (size_t i = 0; i < size; ++i)
	{
		data[i] = static_cast<uint8_t>(rand() % 256);
	}
}

SerializationView CopySerializationView(std::vector<uint8_t>& buffer, SerializationView other)
{
	buffer.resize(other.size);
	memcpy(buffer.data(), other.data, other.size);
	return { buffer.data(), other.size };
}

bool TestRewind(SerializationView frame1, SerializationView frame2)
{
    RewindController RewindController;

    //Save initial frame
    RewindController.EncodeFrameDelta(1,frame1);

    // Save delta
    RewindController.EncodeFrameDelta(1,frame2);

    //Rewind to previous frame
    SerializationView* rewoundData = RewindController.Rewind();

    if (!rewoundData || rewoundData->size != frame1.size)
    {
        return false;
    }

    return memcmp(frame1.data, rewoundData->data, frame1.size) == 0;
}

std::vector<std::string> GetRewindTestFiles()
{
    auto tests = FileParser::GetFilesInPathRecursive(CommandLineParser::GlobalCMDParser->GetArgument("rewindTestDir"), ".bin");
    return tests;
}

class RewindTestFixture : public testing::TestWithParam<std::string> 
{
};

TEST_P(RewindTestFixture, Main) 
{
    // Inside a test, access the test parameter with the GetParam() method
    // of the TestWithParam<T> class:
    std::string test = GetParam();

    std::vector<char> baseFrameBlob;
    if (!FileParser::Read(test, baseFrameBlob))
    {
        FAIL();
    }
    
	std::vector<uint8_t> randomizedNextFrameData;
	randomizedNextFrameData.reserve(baseFrameBlob.size());
	FillWithPseudoRandomData(randomizedNextFrameData, baseFrameBlob.size(), 42);

	// Base->Random->Base
    EXPECT_TRUE(TestRewind({ reinterpret_cast<uint8_t*>(baseFrameBlob.data()), baseFrameBlob.size()}, { reinterpret_cast<uint8_t*>(randomizedNextFrameData.data()), randomizedNextFrameData.size() }));
    // Random->Base->Random
    EXPECT_TRUE(TestRewind({ reinterpret_cast<uint8_t*>(randomizedNextFrameData.data()), randomizedNextFrameData.size() }, { reinterpret_cast<uint8_t*>(baseFrameBlob.data()), baseFrameBlob.size() }));
    // Base->Base->Base
    EXPECT_TRUE(TestRewind({ reinterpret_cast<uint8_t*>(baseFrameBlob.data()), baseFrameBlob.size() }, { reinterpret_cast<uint8_t*>(baseFrameBlob.data()), baseFrameBlob.size() }));

}

#define SPLASH_PATH "../../../splash.gb"

TEST(RewindIntegrationTest, Main) 
{
    std::vector<char> romBlob;
    if (!FileParser::Read(SPLASH_PATH, romBlob))
    {
        FAIL();
    }

    VirtualMachine* emu = static_cast<VirtualMachine*>(Emulator::Create(RewindAllocFunc, RewindFreeFunc));

    emu->Load(SPLASH_PATH, romBlob.data(), static_cast<uint32_t>(romBlob.size()));

    // Step the first frame
    EmulatorInputs::InputState inputState;
    emu->Step(inputState, 16.67, false);

	//Serialize state after first frame
    SerializationView frame1 = emu->Serialize(false);

    std::vector<uint8_t> CachedFrame1Data;
    frame1 = CopySerializationView(CachedFrame1Data, frame1);

	//Step second frame
    emu->Step(inputState, 16.67, false);

	//Serialize state after second frame
    SerializationView frame2 = emu->Serialize(false);
    std::vector<uint8_t> CachedFrame2Data;
    frame2 = CopySerializationView(CachedFrame2Data, frame2);

    RewindController RewindController;

    //Save initial frame
    RewindController.EncodeFrameDelta(1,frame1);

    // Save delta
    RewindController.EncodeFrameDelta(1,frame2);

	//Rewind to previous frame
	SerializationView* rewoundData = RewindController.Rewind();

    if (!rewoundData )
    {
        FAIL();
    }

	//Compare rewound data to frame1
    EXPECT_TRUE(memcmp(frame1.data, rewoundData->data, frame1.size) == 0);

	emu->Deserialize(*rewoundData);

	SerializationView ImmediateReserialization = emu->Serialize(false);

	//Compare immediate reserialization to frame1
    EXPECT_TRUE(memcmp(frame1.data, ImmediateReserialization.data, frame1.size) == 0);

	//Step forward one frame again
    emu->Step(inputState, 16.67, false);
	SerializationView RerunFrame2 = emu->Serialize(false);
	//Compare re-run frame2 to original frame2
	EXPECT_TRUE(memcmp(frame2.data, RerunFrame2.data, frame2.size) == 0);

    Emulator::Delete(emu);
}

TEST(RewindIntegrationTest, 60Frames)
{
    std::vector<char> romBlob;
    if (!FileParser::Read(SPLASH_PATH, romBlob))
    {
        FAIL();
    }

    VirtualMachine* emu = static_cast<VirtualMachine*>(Emulator::Create(RewindAllocFunc, RewindFreeFunc));

    emu->Load(SPLASH_PATH, romBlob.data(), static_cast<uint32_t>(romBlob.size()));

    // Step the first frame
    EmulatorInputs::InputState inputState;
    emu->Step(inputState, 16.67, false);


    SerializationView frame1 = emu->Serialize(false);
    std::vector<uint8_t> CachedFrame1Data;
    frame1 = CopySerializationView(CachedFrame1Data, frame1);

    //Step 60 frames
    emu->Step(inputState, 1000.0, false);


    SerializationView frame2 = emu->Serialize(false);
    std::vector<uint8_t> CachedFrame2Data;
    frame2 = CopySerializationView(CachedFrame2Data, frame2);

    RewindController RewindController;

    //Save initial frame
    RewindController.EncodeFrameDelta(1,frame1);

    // Save delta
    RewindController.EncodeFrameDelta(1,frame2);

    //Rewind to previous frame
    SerializationView* rewoundData = RewindController.Rewind();

    if (!rewoundData)
    {
        FAIL();
    }

    //Compare rewound data to frame1
    EXPECT_TRUE(memcmp(frame1.data, rewoundData->data, frame1.size) == 0);

    emu->Deserialize(*rewoundData);

    SerializationView ImmediateReserialization = emu->Serialize(false);

    //Compare immediate reserialization to frame1
    EXPECT_TRUE(memcmp(frame1.data, ImmediateReserialization.data, frame1.size) == 0);

    //Step forward one second again
    emu->Step(inputState, 1000.0, false);
    SerializationView RerunFrame2 = emu->Serialize(false);
    //Compare re-run frame2 to original frame2
    EXPECT_TRUE(memcmp(frame2.data, RerunFrame2.data, frame2.size) == 0);

    Emulator::Delete(emu);
}

TEST(RewindIntegrationTest, MultiRewind)
{
    std::vector<char> romBlob;
    if (!FileParser::Read(SPLASH_PATH, romBlob))
    {
        FAIL();
    }

    VirtualMachine* emu = static_cast<VirtualMachine*>(Emulator::Create(RewindAllocFunc, RewindFreeFunc));

    emu->Load(SPLASH_PATH, romBlob.data(), static_cast<uint32_t>(romBlob.size()));

    // Step the first frame
    EmulatorInputs::InputState inputState;
    emu->Step(inputState, 16.67, false);


    SerializationView frame1 = emu->Serialize(false);
    std::vector<uint8_t> CachedFrame1Data;
    frame1 = CopySerializationView(CachedFrame1Data, frame1);

    RewindController RewindController;
    //Save initial frame
    RewindController.EncodeFrameDelta(1,frame1);

    for (uint64_t i = 0; i < 10; ++i)
    {
        emu->Step(inputState, 16.67, false);
        SerializationView frameN = emu->Serialize(false);
        // Save delta
        RewindController.EncodeFrameDelta(1,frameN);
    }

    for (uint64_t i = 0; i < 10; ++i)
    {
        SerializationView* rewoundData = RewindController.Rewind();

        if (!rewoundData)
        {
            FAIL();
        }
        emu->Deserialize(*rewoundData);
    }

    SerializationView RewoundFrame1 = emu->Serialize(false);

    EXPECT_TRUE(memcmp(frame1.data, RewoundFrame1.data, frame1.size) == 0);

    Emulator::Delete(emu);
}

INSTANTIATE_TEST_CASE_P(RewindTests,
    RewindTestFixture,
    testing::ValuesIn(GetRewindTestFiles()));