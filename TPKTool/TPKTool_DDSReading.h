#pragma once
#include "stdafx.h"
#include "TPKTool.h"
#include "DDS.h"

bool CheckIfValidDDS(const char* InFileName)
{
	struct DirectX::DDS_HEADER DDSHeaderStruct = { 0 };

	FILE *fin = fopen(InFileName, "rb");
	if (fin == NULL)
	{
		perror("ERROR");
		return false;
	}
	unsigned int ReadMagic = 0;
	fread(&ReadMagic, 4, 1, fin);
	if (ReadMagic != DDS_MAGIC)
	{
		fclose(fin);
		return false;
	}
	fread(&DDSHeaderStruct, sizeof(DirectX::DDS_HEADER), 1, fin);
	if (DDSHeaderStruct.dwSize != 124 || DDSHeaderStruct.dwFlags < 0x1000 || DDSHeaderStruct.dwCaps < 0x1000)
	{
		fclose(fin);
		return false;
	}
	fclose(fin);
	return true;
}

int ReadDDSData(TexStruct *InTexture, GamePixelFormatStruct *InGamePixelFormat, unsigned int *InRelativeDDSDataOffset, int TexNumber)
{
	struct DirectX::DDS_HEADER DDSHeaderStruct = { 0 };
	struct DirectX::DDS_PIXELFORMAT DDSPixelFormatStruct = { 0 };
	
	FILE *fin = fopen(InTexture[TexNumber].FilesystemPath, "rb");
	if (fin == NULL)
	{
		perror("ERROR");
		return -1;
	}
	fseek(fin, 4, SEEK_SET);
	DDSHeaderStruct.ddspf = DDSPixelFormatStruct;
	fread(&DDSHeaderStruct, sizeof(DirectX::DDS_HEADER), 1, fin);
	InTexture[TexNumber].Child4.ResY = DDSHeaderStruct.dwHeight;
	InTexture[TexNumber].Child4.ResX = DDSHeaderStruct.dwWidth;
	InTexture[TexNumber].Child4.MipmapCount = DDSHeaderStruct.dwMipMapCount;
	if (!InTexture[TexNumber].Child4.MipmapCount)
		InTexture[TexNumber].Child4.MipmapCount = 1;

	if (DDSHeaderStruct.ddspf.dwFlags >= 0x40)
		InGamePixelFormat[TexNumber].FourCC = 0x15;
	else
		InGamePixelFormat[TexNumber].FourCC = DDSHeaderStruct.ddspf.dwFourCC;
	fseek(fin, 0L, SEEK_END);
	InTexture[TexNumber].Child4.DataSize = ftell(fin) - 0x80;
	InTexture[TexNumber].Child4.DataOffset = *InRelativeDDSDataOffset;
	*InRelativeDDSDataOffset += InTexture[TexNumber].Child4.DataSize;
	fclose(fin);
	return 1;
}
