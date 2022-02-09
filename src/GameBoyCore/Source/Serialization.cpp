#pragma once
#include "Serialization.h"
#include "Logging.h"

#define HEADER_NAME "GameboySerializedStateFile"
#define HEADER_MAGIC_TOKEN 4242
#define HEADER_CURRENT_VERSION 1

namespace Serializer_Internal
{
	struct FileHeader
	{
		char m_name[27];
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

	FileHeader& WriteHeader(std::vector<uint8_t>& buffer)
	{
		FileHeader header;
		strcpy_s(header.m_name, HEADER_NAME);
		header.m_magicToken = HEADER_MAGIC_TOKEN;
		header.m_version = HEADER_CURRENT_VERSION;

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

void Serializer::RegisterComponent(ISerializable* component)
{
	m_components.push_back(component);
}

std::vector<uint8_t> Serializer::Serialize(uint8_t headerChecksum, const std::string& romName) const
{
	std::vector<ISerializable::Chunk> chunks;
	std::vector<uint8_t> data;
	for (ISerializable* component : m_components)
	{
		component->Serialize(chunks, data);
	}

	std::vector<uint8_t> buffer;

	uint32_t headerSize = sizeof(Serializer_Internal::FileHeader);
	uint32_t romNameSize = static_cast<uint32_t>(romName.size());
	uint32_t chunkSize = sizeof(ISerializable::Chunk) * static_cast<uint32_t>(chunks.size());
	uint32_t dataSize = static_cast<uint32_t>(data.size());

	uint32_t totalSize = headerSize + romNameSize + chunkSize + dataSize;
	buffer.resize(totalSize);

	Serializer_Internal::FileHeader& header = Serializer_Internal::WriteHeader(buffer);

	header.m_romChecksum = headerChecksum;
	header.m_romNameStartOffset = headerSize;
	header.m_romNameLength = romNameSize;
	header.m_chunkSize = chunkSize;
	header.m_chunkStartOffset = headerSize + romNameSize;
	header.m_dataSize = dataSize;
	header.m_dataStartOffset = headerSize + romNameSize + chunkSize;

	uint8_t* rawData = buffer.data();
	memcpy(rawData + header.m_romNameStartOffset, romName.c_str(), romNameSize);
	memcpy(rawData + header.m_chunkStartOffset, chunks.data(), chunkSize);
	memcpy(rawData + header.m_dataStartOffset, data.data(), dataSize);

	return buffer;
}

void Serializer::Deserialize(const uint8_t* buffer, const uint32_t size, uint8_t headerChecksum)
{
	Serializer_Internal::FileHeader header = Serializer_Internal::ParseHeader(buffer);

	uint32_t expectedSize = sizeof(Serializer_Internal::FileHeader) + header.m_romNameLength + header.m_chunkSize + header.m_dataSize;

	if (size != expectedSize)
	{
		LOG_ERROR("Mismatch of file size and expected contents for serialized state load");
		return;
	}

	if (header.m_romChecksum != headerChecksum)
	{
		const char* name = reinterpret_cast<const char*>(buffer + header.m_romNameStartOffset);
		LOG_ERROR(string_format("Mismatch of checksum in state and loaded rom, please load rom %s", name).c_str());
		return;
	}

	const ISerializable::Chunk* chunks = reinterpret_cast<const ISerializable::Chunk*>(buffer + header.m_chunkStartOffset);
	uint32_t chunkCount = header.m_chunkSize / sizeof(ISerializable::Chunk);

	const uint8_t* data = buffer + header.m_dataStartOffset;
	uint8_t dataCount = header.m_dataSize;

	for (ISerializable* component : m_components)
	{
		component->Deserialize(chunks, chunkCount, data, dataCount);
	}
}

ISerializable::ISerializable(Serializer* serializer)
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

const ISerializable::Chunk* ISerializable::FindChunk(const Chunk* chunks, const uint32_t& chunkCount, const ChunkId& chunkId)
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

void ISerializable::WriteAndMove(uint8_t* destination, const void* source, const uint32_t& size)
{
	memcpy(destination, source, static_cast<size_t>(size));
	destination += size;
}

void ISerializable::ReadAndMove(const uint8_t* source, void* destination, const uint32_t& size)
{
	memcpy(destination, source, static_cast<size_t>(size));
	source += size;
}
