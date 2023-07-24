#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "Serialization.h"

void Serialize(Serializer* serializer, void* dataPtr, size_t dataSize)
{
	if (serializer->IsWriting)
	{
		fwrite(dataPtr, dataSize, 1, serializer->FilePtr);

	}
	else
	{
		fread(dataPtr, dataSize, 1, serializer->FilePtr);

	}
}


void Serialize(Serializer* serializer, int32_t* dataPtr) {
	Serialize(serializer, dataPtr, sizeof(*dataPtr));
}

void Serialize(Serializer* serializer, float* dataPtr) {
	Serialize(serializer, dataPtr, sizeof(*dataPtr));
}

void Serialize(Serializer* serializer, bool* dataPtr) {
	Serialize(serializer, dataPtr, sizeof(*dataPtr));
}

Serializer SerializerOpenFile(bool isWriting, const char* file)
{
	Serializer serializer;
	serializer.IsWriting = isWriting;
	if(serializer.IsWriting)
	{
		serializer.FilePtr = fopen(file, "wb");
	}else
	{
		serializer.FilePtr = fopen(file, "rb");
	}
	return serializer;
}

void SerializeIncludingVersion(Serializer* serializer)
{
	if (serializer->IsWriting)
	{
		serializer->DataVersion = SV_LatestVersion;
	}

	Serialize(serializer, &serializer->DataVersion);
}

void SerializerFileClose(Serializer* serializer)
{
	fclose(serializer->FilePtr);
}

void Serialize(Serializer* ser, Vector2F* vector2f)
{
	Serialize(ser, &vector2f->X);
	Serialize(ser, &vector2f->Y);
}
