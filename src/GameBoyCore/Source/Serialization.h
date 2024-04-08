#pragma once
#include <vector>
#include <string>

enum class ChunkId
{
	Memory = 0,
	CPU = 1,
	Timer = 2,
	MBC = 3,
	APU = 4,
	PPU = 5,
};

class ISerializable;

class Serializer
{
public:
	void RegisterComponent(ISerializable* component);
	std::vector<uint8_t> Serialize(uint8_t headerChecksum, const std::string& romName, bool rawData) const;
	void Deserialize(const uint8_t* buffer, const uint32_t size, uint8_t headerChecksum);
private:
	std::vector<ISerializable*> m_components;
};

class ISerializable
{
protected:
	friend class Serializer;

	struct Chunk
	{
		ChunkId m_id;
		uint32_t m_offset;
		uint32_t m_size;
	};

	ISerializable(Serializer* serializer);

	virtual void Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data) = 0;
	virtual void Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize) = 0;

	static uint8_t* CreateChunkAndGetDataPtr(std::vector<Chunk>& chunks, std::vector<uint8_t>& data, const uint32_t& writeDataSize, const ChunkId& chunkId);
	static const Chunk* FindChunk(const Chunk* chunks, const uint32_t& chunkCount, const ChunkId& chunkId);
	static void WriteAndMove(uint8_t*& destination, const void* source, const uint32_t& size);
	static void ReadAndMove(const uint8_t*& source, void* destination, const uint32_t& size);
};