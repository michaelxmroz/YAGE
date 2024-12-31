#pragma once
#include "CppIncludes.h"

#define SERIALIZER_HEADER_NAME_MAXLENGTH 27

enum class ChunkId
{
	Memory = 0,
	CPU = 1,
	Timer = 2,
	MBC = 3,
	APU = 4,
	PPU = 5,
	MBC_Save = 6
};

class ISerializable;

struct Chunk
{
	ChunkId m_id;
	uint32_t m_offset;
	uint32_t m_size;
};

struct SerializationParameters
{
	char m_dataName[SERIALIZER_HEADER_NAME_MAXLENGTH];
	uint32_t m_version;
	uint32_t m_romChecksum;
	bool m_noHeaders;
	std::string m_romName;
};

class SerializationFactory
{
public:
	SerializationFactory(SerializationParameters parameters);
	void Serialize(ISerializable* component);
	uint8_t* CreateChunk(ChunkId id, uint32_t dataSize);
	void Finish(std::vector<uint8_t>& dataOut);
private:
	SerializationParameters m_parameters;
	bool m_finished;
	std::vector<Chunk> m_chunks;
	std::vector<uint8_t> m_data;
};

class DeserializationFactory
{
public:

	struct RawBuffers
	{
		const Chunk* m_chunks;
		const uint32_t m_chunkCount;
		const uint8_t* m_data;
		const uint32_t m_dataSize;
	};

	DeserializationFactory(SerializationParameters parameters, const uint8_t* buffer, const uint32_t size);
	void Deserialize(const uint8_t* buffer, std::vector<ISerializable*>& components);
	RawBuffers GetRawBuffers(const uint8_t* buffer);
	void Finish();

private:
	SerializationParameters m_parameters;
	bool m_finished;
	uint32_t m_chunkStartOffset;
	uint32_t m_chunkCount;
	uint32_t m_dataStartOffset;
	uint32_t m_dataSize;

};

class GamestateSerializer
{
public:
	void RegisterComponent(ISerializable* component);
	void Serialize(uint8_t headerChecksum, const std::string& romName, bool rawData, std::vector<uint8_t>& dataOut) const;
	void Deserialize(const uint8_t* buffer, const uint32_t size, uint8_t headerChecksum);
private:
	std::vector<ISerializable*> m_components;
};

class ISerializable
{
protected:
	friend class GamestateSerializer;
	friend class SerializationFactory;
	friend class DeserializationFactory;

	ISerializable(GamestateSerializer* serializer);

	virtual void Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data) = 0;
	virtual void Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize) = 0;

	static uint8_t* CreateChunkAndGetDataPtr(std::vector<Chunk>& chunks, std::vector<uint8_t>& data, const uint32_t& writeDataSize, const ChunkId& chunkId);
	static const Chunk* FindChunk(const Chunk* chunks, const uint32_t& chunkCount, const ChunkId& chunkId);
	static void WriteAndMove(uint8_t*& destination, const void* source, const uint32_t& size);
	static void ReadAndMove(const uint8_t*& source, void* destination, const uint32_t& size);
};