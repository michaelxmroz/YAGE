#pragma once
#include "Serialization.h"
#include "Logging.h"

#define HEADER_DEFAULT_NAME "GameboySerializedStateFile"
#define HEADER_MAGIC_TOKEN 4242
#define HEADER_CURRENT_VERSION 1

namespace
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

	FileHeader& WriteHeader(const char* name, uint32_t version, std::vector<uint8_t>& buffer)
	{
		FileHeader header;
		strcpy_s(header.m_name, name);
		header.m_magicToken = HEADER_MAGIC_TOKEN;
		header.m_version = version;

		void* rawData = buffer.data();
		memcpy(rawData, &header, sizeof(FileHeader));
		return *reinterpret_cast<FileHeader*>(rawData);
	}

	FileHeader ParseHeader(const uint8_t* buffer)
	{
		const FileHeader* header = reinterpret_cast<const FileHeader*>(buffer);
		if (header->m_magicToken != HEADER_MAGIC_TOKEN || header->m_version > HEADER_CURRENT_VERSION)
		{
			LOG_ERROR("Invalid Header in serialized state file");
			return FileHeader();
		}
		if (header->m_version < HEADER_CURRENT_VERSION)
		{
			LOG_ERROR("Outdated version of serialized state file");
			return FileHeader();
		}

		return *header;
	}
}

void GamestateSerializer::RegisterComponent(ISerializable* component)
{
	m_components.push_back(component);
}

void GamestateSerializer::Serialize(uint8_t headerChecksum, const std::string& romName, bool rawData, std::vector<uint8_t>& dataOut) const
{
	SerializationParameters params;
	memcpy(params.m_dataName, HEADER_DEFAULT_NAME, SERIALIZER_HEADER_NAME_MAXLENGTH);
	params.m_version = HEADER_CURRENT_VERSION;
	params.m_romChecksum = headerChecksum;
	params.m_noHeaders = rawData;
	params.m_romName = romName;

	SerializationFactory serializer(params);
	
	for (ISerializable* component : m_components)
	{
		serializer.Serialize(component);
	}

	serializer.Finish(dataOut);
}

void GamestateSerializer::Deserialize(const uint8_t* buffer, const uint32_t size, uint8_t headerChecksum)
{
	SerializationParameters params;
	memcpy(params.m_dataName, HEADER_DEFAULT_NAME, SERIALIZER_HEADER_NAME_MAXLENGTH);
	params.m_version = HEADER_CURRENT_VERSION;
	params.m_romChecksum = headerChecksum;

	DeserializationFactory deserializer(params, buffer, size);

	deserializer.Deserialize(buffer, m_components);

	deserializer.Finish();
}

ISerializable::ISerializable(GamestateSerializer* serializer)
{
	if (serializer != nullptr)
	{
		serializer->RegisterComponent(this);
	}
}

uint8_t* ISerializable::CreateChunkAndGetDataPtr(std::vector<Chunk>& chunks, std::vector<uint8_t>& data, const uint32_t& writeDataSize, const ChunkId& chunkId)
{
	uint32_t oldSize = static_cast<uint32_t>(data.size());

	Chunk myChunk;
	myChunk.m_id = chunkId;
	myChunk.m_offset = oldSize;
	myChunk.m_size = writeDataSize;
	chunks.push_back(myChunk);

	data.resize(oldSize + writeDataSize);
	uint8_t* rawData = data.data();
	rawData += oldSize;
	return rawData;
}

const Chunk* ISerializable::FindChunk(const Chunk* chunks, const uint32_t& chunkCount, const ChunkId& chunkId)
{
	for (uint32_t i = 0; i < chunkCount; ++i)
	{
		if (chunks[i].m_id == chunkId)
		{
			return chunks + i;
		}
	}
	return nullptr;
}

void ISerializable::WriteAndMove(uint8_t*& destination, const void* source, const uint32_t& size)
{
	memcpy(destination, source, static_cast<size_t>(size));
	destination += size;
}

void ISerializable::ReadAndMove(const uint8_t*& source, void* destination, const uint32_t& size)
{
	memcpy(destination, source, static_cast<size_t>(size));
	source += size;
}

SerializationFactory::SerializationFactory(SerializationParameters parameters)
	: m_parameters(parameters)
	, m_finished(false)
	, m_chunks()
	, m_data()

{
}

void SerializationFactory::Serialize(ISerializable* component)
{
	if (m_finished)
	{
		LOG_ERROR("Trying to serialize a component for an already finished serialization factory.");
		return;
	}
	component->Serialize(m_chunks, m_data);
}

uint8_t* SerializationFactory::CreateChunk(ChunkId id, uint32_t dataSize)
{
	if (m_finished)
	{
		LOG_ERROR("Trying to create a chunk for an already finished serialization factory.");
		return nullptr;
	}
	return ISerializable::CreateChunkAndGetDataPtr(m_chunks, m_data, dataSize, id);
}

void SerializationFactory::Finish(std::vector<uint8_t>& dataOut)
{
	if (m_finished)
	{
		LOG_ERROR("Trying to finish an already finished serialization factory.");
		return;
	}

	uint32_t headerSize = sizeof(FileHeader);
	uint32_t romNameSize = SERIALIZER_HEADER_NAME_MAXLENGTH;
	uint32_t chunkSize = sizeof(Chunk) * static_cast<uint32_t>(m_chunks.size());
	uint32_t dataSize = static_cast<uint32_t>(m_data.size());

	uint32_t totalSize = headerSize + romNameSize + chunkSize + dataSize;
	dataOut.resize(totalSize);

	FileHeader& header = WriteHeader(m_parameters.m_dataName, m_parameters.m_version, dataOut);

	header.m_romChecksum = m_parameters.m_romChecksum;
	header.m_romNameStartOffset = headerSize;
	header.m_romNameLength = romNameSize;
	header.m_chunkSize = chunkSize;
	header.m_chunkStartOffset = headerSize + romNameSize;
	header.m_dataSize = dataSize;
	header.m_dataStartOffset = headerSize + romNameSize + chunkSize;

	uint8_t* rawBuffer = dataOut.data();
	if (!m_parameters.m_noHeaders)
	{
		memcpy(rawBuffer + header.m_romNameStartOffset, m_parameters.m_romName.c_str(), romNameSize);
		memcpy(rawBuffer + header.m_chunkStartOffset, m_chunks.data(), chunkSize);
		memcpy(rawBuffer + header.m_dataStartOffset, m_data.data(), dataSize);
	}
	else
	{
		memcpy(rawBuffer, m_data.data(), dataSize);
	}

	m_finished = true;
}

DeserializationFactory::DeserializationFactory(SerializationParameters parameters, const uint8_t* buffer, const uint32_t size)
	: m_parameters(parameters)
	, m_finished(false)
{
	//TODO also compare header file names & version

	FileHeader header = ParseHeader(buffer);

	uint32_t expectedSize = sizeof(FileHeader) + header.m_romNameLength + header.m_chunkSize + header.m_dataSize;

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

void DeserializationFactory::Deserialize(const uint8_t* buffer, std::vector<ISerializable*>& components)
{
	if (m_finished)
	{
		LOG_ERROR("Trying to deserialize a component for an already finished deserialization factory.");
		return;
	}

	const Chunk* chunks = reinterpret_cast<const Chunk*>(buffer + m_chunkStartOffset);
	uint32_t chunkCount = m_chunkCount;

	const uint8_t* data = buffer + m_dataStartOffset;
	uint8_t dataCount = m_dataSize;

	for (ISerializable* component : components)
	{
		component->Deserialize(chunks, chunkCount, data, dataCount);
	}
}

DeserializationFactory::RawBuffers DeserializationFactory::GetRawBuffers(const uint8_t* buffer)
{
	if (m_finished)
	{
		LOG_ERROR("Trying to access buffers for an already finished deserialization factory.");
		return RawBuffers{nullptr, 0, nullptr, 0};
	}

	RawBuffers buffers{
	reinterpret_cast<const Chunk*>(buffer + m_chunkStartOffset)
	, m_chunkCount
	, buffer + m_dataStartOffset
	, m_dataSize
	};
	return buffers;
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
