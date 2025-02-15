#include "Serialization.h"
#include "Logging.h"
#include "Helpers.h"

#define HEADER_DEFAULT_NAME "GameboySerializedStateFile"
#define HEADER_MAGIC_TOKEN 4242

// Bump this on major changes to the file format
#define HEADER_CURRENT_VERSION 2

namespace Serializer_Internal
{
	struct FileHeader
	{
		char m_name[SERIALIZER_HEADER_NAME_MAXLENGTH];
		uint32_t m_magicToken;
		uint32_t m_version;
		uint32_t m_romChecksum;
		uint32_t m_romNameLength;
		uint32_t m_romNameStartOffset;
		uint32_t m_chunkSize;
		uint32_t m_chunkStartOffset;
		uint32_t m_dataSize;
		uint32_t m_dataStartOffset;
	};

	FileHeader& WriteHeader(const char* name, uint32_t version, uint8_t* buffer)
	{
		FileHeader header;
		memcpy_y(header.m_name, name, SERIALIZER_HEADER_NAME_MAXLENGTH);
		header.m_magicToken = HEADER_MAGIC_TOKEN;
		header.m_version = version;

		memcpy_y(buffer, &header, sizeof(FileHeader));
		return *reinterpret_cast<FileHeader*>(buffer);
	}

	FileHeader ParseHeader(uint32_t version, const uint8_t* buffer)
	{
		const FileHeader* header = reinterpret_cast<const FileHeader*>(buffer);
		if (header->m_magicToken != HEADER_MAGIC_TOKEN || header->m_version > version)
		{
			LOG_ERROR("Invalid Header in serialized state file");
			return FileHeader();
		}
		if (header->m_version < version)
		{
			LOG_ERROR("Outdated version of serialized state file");
			return FileHeader();
		}

		return *header;
	}
}

GamestateSerializer::GamestateSerializer()
{
	m_registeredComponentCount = 0;
	for (auto& component : m_components)
	{
		component = nullptr;
	}
}

void GamestateSerializer::RegisterComponent(ISerializable* component, ChunkId id)
{
	m_components[static_cast<uint32_t>(id)] = component;
	m_registeredComponentCount++;
}

void GamestateSerializer::Init()
{
	uint32_t headerSize = sizeof(Serializer_Internal::FileHeader);
	uint32_t romNameSize = SERIALIZER_HEADER_NAME_MAXLENGTH;
	uint32_t chunkSize = sizeof(Chunk) * m_registeredComponentCount;
	uint32_t dataSize = 0;

	for (ISerializable* component : m_components)
	{
		if (component)
		{
			dataSize += component->GetSerializationSize();
		}
	}

	uint32_t totalSize = headerSize + romNameSize + chunkSize + dataSize;
	m_serializationBuffer.resize(totalSize);

	m_chunkView = reinterpret_cast<Chunk*>(m_serializationBuffer.data() + headerSize + romNameSize);
	m_dataView = m_serializationBuffer.data() + headerSize + romNameSize + chunkSize;

}

SerializationView GamestateSerializer::Serialize(uint8_t headerChecksum, const yString& romName, bool rawData)
{
	if(m_serializationBuffer.size() == 0)
	{
		Init();
	}

	SerializationParameters params;
	memcpy_y(params.m_dataName, HEADER_DEFAULT_NAME, SERIALIZER_HEADER_NAME_MAXLENGTH);
	params.m_version = HEADER_CURRENT_VERSION;
	params.m_romChecksum = headerChecksum;
	params.m_romName = romName;

	SerializationFactory serializer(params, m_chunkView, m_dataView, m_serializationBuffer.data());
	
	for (ISerializable* component : m_components)
	{
		if(component)
		{
			serializer.Serialize(component);
		}
	}

	serializer.Finish(m_serializationBuffer.size());

	return { m_serializationBuffer.data(), m_serializationBuffer.size() };
}

void GamestateSerializer::Deserialize(const SerializationView& data, uint8_t headerChecksum)
{
	SerializationParameters params;
	memcpy_y(params.m_dataName, HEADER_DEFAULT_NAME, SERIALIZER_HEADER_NAME_MAXLENGTH);
	params.m_version = HEADER_CURRENT_VERSION;
	params.m_romChecksum = headerChecksum;

	DeserializationFactory deserializer(params, data.data, data.size);

	deserializer.Deserialize(data.data, m_components);

	deserializer.Finish();
}

ISerializable::ISerializable(GamestateSerializer* serializer, ChunkId id) :
	m_id(id)
{
	if (serializer != nullptr)
	{
		serializer->RegisterComponent(this, id);
	}
}

void ISerializable::WriteAndMove(uint8_t*& destination, const void* source, const uint32_t& size)
{
	memcpy_y(destination, source, static_cast<size_t>(size));
	destination += size;
}

void ISerializable::ReadAndMove(const uint8_t*& source, void* destination, const uint32_t& size)
{
	memcpy_y(destination, source, static_cast<size_t>(size));
	source += size;
}

SerializationFactory::SerializationFactory(const SerializationParameters& parameters, Chunk* chunks, uint8_t* data, uint8_t* header)
	: m_parameters(parameters)
	, m_finished(false)
	, m_chunks(chunks)
	, m_data(data)
	, m_header(header)
	, m_dataOffset(0)
	, m_serializedChunks(0)
	, m_writtenData(0)
{
}

void SerializationFactory::Serialize(ISerializable* component)
{
	if (m_finished)
	{
		LOG_ERROR("Trying to serialize a component for an already finished serialization factory.");
		return;
	}

	component->Serialize(m_data + m_writtenData);

	uint32_t size = component->GetSerializationSize();
	WriteChunkHeader(size, component->m_id);
}

void SerializationFactory::Finish(uint32_t totalBufferSize)
{
	if (m_finished)
	{
		LOG_ERROR("Trying to finish an already finished serialization factory.");
		return;
	}

	uint32_t headerSize = sizeof(Serializer_Internal::FileHeader);
	uint32_t romNameSize = SERIALIZER_HEADER_NAME_MAXLENGTH;
	uint32_t chunkSize = sizeof(Chunk) * m_serializedChunks;
	uint32_t dataSize = m_writtenData;

	uint32_t totalSize = headerSize + romNameSize + chunkSize + dataSize;

#if _DEBUG
	if(totalSize != totalBufferSize)
	{
		LOG_ERROR("Mismatch between serialization buffer size and actual serialized data!");
	}
#endif

	Serializer_Internal::FileHeader& header = Serializer_Internal::WriteHeader(m_parameters.m_dataName, m_parameters.m_version, m_header);

	header.m_romChecksum = m_parameters.m_romChecksum;
	header.m_romNameStartOffset = headerSize;
	header.m_romNameLength = romNameSize;
	header.m_chunkSize = chunkSize;
	header.m_chunkStartOffset = headerSize + romNameSize;
	header.m_dataSize = dataSize;
	header.m_dataStartOffset = headerSize + romNameSize + chunkSize;

	m_finished = true;
}

uint32_t SerializationFactory::GetHeaderAndNameSize()
{
	uint32_t headerSize = sizeof(Serializer_Internal::FileHeader);
	uint32_t romNameSize = SERIALIZER_HEADER_NAME_MAXLENGTH;
	return headerSize + romNameSize;
}

void SerializationFactory::WriteChunkHeader(uint32_t writeDataSize, const ChunkId& chunkId)
{
	m_chunks->m_id = chunkId;
	m_chunks->m_offset = m_dataOffset;
	m_chunks->m_size = writeDataSize;

	++m_chunks;
	m_dataOffset += writeDataSize;
	m_writtenData += writeDataSize;

	m_serializedChunks++;
}

DeserializationFactory::DeserializationFactory(SerializationParameters parameters, const uint8_t* buffer, const uint32_t size)
	: m_parameters(parameters)
	, m_finished(false)
{

	Serializer_Internal::FileHeader header = Serializer_Internal::ParseHeader(parameters.m_version, buffer);

	uint32_t expectedSize = sizeof(Serializer_Internal::FileHeader) + header.m_romNameLength + header.m_chunkSize + header.m_dataSize;

	if (size != expectedSize)
	{
		LOG_ERROR("Mismatch of file size and expected contents for deserialization. Possible file corruption.");
		m_finished = true;
		return;
	}

	if (header.m_romChecksum != m_parameters.m_romChecksum)
	{
		const char* name = reinterpret_cast<const char*>(buffer + header.m_romNameStartOffset);
		LOG_ERROR(string_format("Mismatch of checksum in state and loaded rom, please load rom %s", name).c_str());
		m_finished = true;
		return;
	}

	m_chunkStartOffset = header.m_chunkStartOffset;
	m_chunkCount = header.m_chunkSize / sizeof(Chunk);

	m_dataStartOffset = header.m_dataStartOffset;
	m_dataSize = header.m_dataSize;
}

void DeserializationFactory::Deserialize(const uint8_t* buffer, ISerializable** components) const
{
	if (m_finished)
	{
		LOG_ERROR("Trying to deserialize a component for an already finished deserialization factory.");
		return;
	}

	const Chunk* chunks = reinterpret_cast<const Chunk*>(buffer + m_chunkStartOffset);

	const uint8_t* data = buffer + m_dataStartOffset;

	for( uint32_t i = 0; i < static_cast<uint32_t>(ChunkId::Count); ++i)
	{
		ISerializable* component = components[i];
		if(component)
		{
			if(component->m_id != chunks->m_id)
			{
				LOG_ERROR("Chunk id mismatch, possible file corruption");
				return;
			}

			component->Deserialize(data + chunks->m_offset);
			++chunks;
		}
	}
}

const uint8_t* DeserializationFactory::GetDataForChunk(const uint8_t* buffer, uint32_t index) const
{
	if (m_finished)
	{
		LOG_ERROR("Trying to deserialize a component for an already finished deserialization factory.");
		return nullptr;
	}

	const Chunk* chunks = reinterpret_cast<const Chunk*>(buffer + m_chunkStartOffset);
	chunks += index;

	return buffer + chunks->m_offset;
}

void DeserializationFactory::Finish()
{
	if (m_finished)
	{
		LOG_ERROR("Trying to finish an already finished serialization factory.");
		return;
	}
	m_finished = true;
}
