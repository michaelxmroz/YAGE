#pragma once
#include "CppIncludes.h"
#include "YString.h"
#include "YVector.h"

#define SERIALIZER_HEADER_NAME_MAXLENGTH 27

enum class ChunkId
{
	Memory = 0,
	CPU = 1,
	Timer = 2,
	MBC = 3,
	APU = 4,
	PPU = 5,
	MBC_Save = 6,
	Serial = 7,
	Count
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
	yString m_romName;
};

class SerializationFactory
{
public:
	SerializationFactory(const SerializationParameters& parameters, Chunk* chunks, uint8_t* data, uint8_t* header);
	void Serialize(ISerializable* component);
	void WriteChunkHeader(uint32_t writeDataSize, const ChunkId& chunkId);
	void Finish(uint32_t totalBufferSize);
	static uint32_t GetHeaderAndNameSize();
private:

	SerializationParameters m_parameters;
	bool m_finished;
	Chunk* m_chunks;
	uint8_t* m_data;
	uint8_t* m_header;
	uint32_t m_dataOffset;
	uint32_t m_serializedChunks;
	uint32_t m_writtenData;
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
	void Deserialize(const uint8_t* buffer, ISerializable** components) const;
	const uint8_t* GetDataForChunk(const uint8_t* buffer, uint32_t index) const;
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
	GamestateSerializer();
	void RegisterComponent(ISerializable* component, ChunkId id);
	SerializationView Serialize(uint8_t headerChecksum, const yString& romName, bool rawData);
	void Deserialize(const SerializationView& data, uint8_t headerChecksum);
private:

	void Init();

	uint32_t m_registeredComponentCount;
	ISerializable* m_components[static_cast<uint32_t>(ChunkId::Count)];
	yVector<uint8_t> m_serializationBuffer;
	Chunk* m_chunkView = nullptr;
	uint8_t* m_dataView = nullptr;
};

class ISerializable
{
protected:
	friend class GamestateSerializer;
	friend class SerializationFactory;
	friend class DeserializationFactory;

	ISerializable(GamestateSerializer* serializer, ChunkId id);
	virtual ~ISerializable() = default;

	virtual void Serialize(uint8_t* data) = 0;
	virtual void Deserialize(const uint8_t* data) = 0;
	virtual uint32_t GetSerializationSize() = 0;

	static void WriteAndMove(uint8_t*& destination, const void* source, const uint32_t& size);
	static void ReadAndMove(const uint8_t*& source, void* destination, const uint32_t& size);

	const ChunkId m_id;
};