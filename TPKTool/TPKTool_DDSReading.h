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
	InTexture[TexNumber].Child4.Height = DDSHeaderStruct.dwHeight;
	InTexture[TexNumber].Child4.Width = DDSHeaderStruct.dwWidth;
	uint32_t ShiftHeight = DDSHeaderStruct.dwHeight;
	uint32_t ShiftWidth = DDSHeaderStruct.dwWidth;
	uint32_t shift_counter = 0;

	while (ShiftHeight != 1)
	{
		ShiftHeight /= 2;
		shift_counter++;
	}

	InTexture[TexNumber].Child4.ShiftHeight = (uint8_t)shift_counter;

	shift_counter = 0;
	while (ShiftWidth != 1)
	{
		ShiftWidth /= 2;
		shift_counter++;
	}
	InTexture[TexNumber].Child4.ShiftWidth = (uint8_t)shift_counter;


	InTexture[TexNumber].Child4.NumMipMapLevels = DDSHeaderStruct.dwMipMapCount;
	if (!InTexture[TexNumber].Child4.NumMipMapLevels)
		InTexture[TexNumber].Child4.NumMipMapLevels = 1;

	// dropped P8 repack support
	if (DDSHeaderStruct.ddspf.dwFlags >= 0x40)
	{
		InGamePixelFormat[TexNumber].FourCC = FOURCC_ARGB;
		InTexture[TexNumber].Child4.ImageCompressionType = TPK_COMPRESSION_TYPE_RGBA;
	}
	else
	{
		InGamePixelFormat[TexNumber].FourCC = DDSHeaderStruct.ddspf.dwFourCC;
		switch (InGamePixelFormat[TexNumber].FourCC)
		{
		case FOURCC_DXT1:
			InTexture[TexNumber].Child4.ImageCompressionType = TPK_COMPRESSION_TYPE_DXT1;
			break;
		case FOURCC_DXT3:
			InTexture[TexNumber].Child4.ImageCompressionType = TPK_COMPRESSION_TYPE_DXT3;
			break;
		case FOURCC_DXT5:
			InTexture[TexNumber].Child4.ImageCompressionType = TPK_COMPRESSION_TYPE_DXT5;
			break;
		default:
			InTexture[TexNumber].Child4.ImageCompressionType = TPK_COMPRESSION_TYPE_RGBA;
			break;
		}
	}
	fseek(fin, 0L, SEEK_END);
	InTexture[TexNumber].Child4.ImageSize = ftell(fin) - 0x80;
	InTexture[TexNumber].Child4.ImagePlacement = *InRelativeDDSDataOffset;
	*InRelativeDDSDataOffset += InTexture[TexNumber].Child4.ImageSize;
	fclose(fin);
	return 1;
}
