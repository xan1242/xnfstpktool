#pragma once
#include "stdafx.h"
#include <stdlib.h>
#include <direct.h>
#include <iostream>
#include <vector>
using namespace std;

// uncomment this to enable decompression code, requires DecompressionCode.h which contains propriatery game code
// place DecompressionCode.h in the project folder (also named TPKTool)
#define TPKTOOL_DECOMPRESSION

#ifdef TPKTOOL_DECOMPRESSION
#include "includes\injector\injector.hpp"
#include "DecompressionCode.h"
int(__cdecl *Ripped_LZDecompress)(unsigned char* InBuffer, unsigned char* OutBuffer) = (int(__cdecl*)(unsigned char*, unsigned char*))(DecompressionCode + 0x6670);
unsigned int BogusPointer = 0;
bool bCodePatched = 0;
bool bDoCompressedStringsOnce = 0;
vector<uint32_t> TIMoffsets;

/*int OutputDDSFromMemory(const char* OutFilePath, TexStruct InTexture, GamePixelFormatStruct InGamePixelFormat, DirectX::DDS_HEADER InDDSHeaderStruct, DirectX::DDS_PIXELFORMAT InDDSPixelFormatStruct, unsigned int TexNumber, void* DDSDataBuffer)
{
	InDDSHeaderStruct.dwSize = 124;
	InDDSHeaderStruct.dwFlags = 0x21007;
	InDDSHeaderStruct.dwHeight = InTexture.Child4.ResY;
	InDDSHeaderStruct.dwWidth = InTexture.Child4.ResX;
	InDDSHeaderStruct.dwMipMapCount = InTexture.Child4.MipmapCount;
	//InDDSHeaderStruct.dwMipMapCount = 0;
	InDDSPixelFormatStruct.dwSize = 32;
	if (InGamePixelFormat.FourCC == FOURCC_ARGB)
	{
		InDDSPixelFormatStruct.dwFlags = 0x41;
		InDDSPixelFormatStruct.dwRGBBitCount = 0x20;
		InDDSPixelFormatStruct.dwRBitMask = 0xFF0000;
		InDDSPixelFormatStruct.dwGBitMask = 0xFF00;
		InDDSPixelFormatStruct.dwBBitMask = 0xFF;
		InDDSPixelFormatStruct.dwABitMask = 0xFF000000;
		InDDSHeaderStruct.dwCaps = 0x40100A;
	}
	else
	{
		InDDSPixelFormatStruct.dwFlags = 4;
		InDDSPixelFormatStruct.dwFourCC = InGamePixelFormat.FourCC;
		InDDSHeaderStruct.dwCaps = 0x401008;
	}
	InDDSHeaderStruct.ddspf = InDDSPixelFormatStruct;
	//DDSDataBuffer = malloc(texture[TexNumber].Child4.DataSize);
	//fseek(fin1, texture[TexNumber].Child4.DataOffset + RelativeStart, SEEK_SET);
	//fread(DDSDataBuffer, sizeof(char), texture[TexNumber].Child4.DataSize, fin1);

	FILE *fout = fopen(OutFilePath, "wb");
	if (!fout)
	{
		printf("Error opening %s\n", OutFilePath);
		return -1;
	}
	unsigned int DDSMagic = DDS_MAGIC;
	fwrite(&DDSMagic, 4, 1, fout);
	//fseek(fout, 4, SEEK_CUR);
	fwrite(&InDDSHeaderStruct, sizeof(DirectX::DDS_HEADER), 1, fout);
	fwrite(DDSDataBuffer, sizeof(char), InTexture.Child4.DataSize, fout);

	fclose(fout);
	return 1;
}
*/

int OutputDDSFromMemory(const char* OutFilePath, unsigned int TexNumber, void* DDSDataBuffer, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormat)
{
	struct DirectX::DDS_HEADER DDSHeaderStruct = { 0 };
	struct DirectX::DDS_PIXELFORMAT DDSPixelFormatStruct = { 0 };

	DDSHeaderStruct.dwSize = 124;
	DDSHeaderStruct.dwFlags = 0x21007;
	DDSHeaderStruct.dwHeight = InTexStruct[TexNumber].Child4.Height;
	DDSHeaderStruct.dwWidth = InTexStruct[TexNumber].Child4.Width;
	DDSHeaderStruct.dwMipMapCount = InTexStruct[TexNumber].Child4.NumMipMapLevels;
	//DDSHeaderStruct.dwMipMapCount = 0;
	DDSPixelFormatStruct.dwSize = 32;
	if (InGamePixelFormat[TexNumber].FourCC == FOURCC_ARGB)
	{
		DDSPixelFormatStruct.dwFlags = DDS_RGBA;
		DDSPixelFormatStruct.dwRGBBitCount = 0x20;
		DDSPixelFormatStruct.dwRBitMask = 0xFF0000;
		DDSPixelFormatStruct.dwGBitMask = 0xFF00;
		DDSPixelFormatStruct.dwBBitMask = 0xFF;
		DDSPixelFormatStruct.dwABitMask = 0xFF000000;
		DDSHeaderStruct.dwCaps = 0x40100A;
	}
	else if (InGamePixelFormat[TexNumber].FourCC == 0x29)
	{
		DDSPixelFormatStruct.dwFlags = DDS_PAL8A;
		DDSHeaderStruct.dwCaps = 0x1000;
	}
	else
	{
		DDSPixelFormatStruct.dwFlags = DDS_FOURCC;
		DDSPixelFormatStruct.dwFourCC = InGamePixelFormat[TexNumber].FourCC;
		DDSHeaderStruct.dwCaps = 0x401008;
	}
	DDSHeaderStruct.ddspf = DDSPixelFormatStruct;

	FILE *fout = fopen(OutFilePath, "wb");
	if (!fout)
	{
		printf("Error opening %s\n", OutFilePath);
		return -1;
	}
	unsigned int DDSMagic = DDS_MAGIC;
	fwrite(&DDSMagic, 4, 1, fout);
	fwrite(&DDSHeaderStruct, sizeof(DDSHeaderStruct), 1, fout);
	fwrite(DDSDataBuffer, sizeof(char), InTexStruct[TexNumber].Child4.ImageSize, fout);

	fclose(fout);
	return 1;
}

#endif

void* P8toRGBA(void* data, unsigned int Length, unsigned int* NewLength, unsigned int* palette)
{
	void* result = calloc(sizeof(char), (Length) * 4); // 8 bpp * 4 bytes per color
	*NewLength = (Length) * 4;

	// Write result data based on palette
	for (int loop = 0, index = 0; loop < Length; ++loop, index += 4)
	{
		unsigned int color = palette[((unsigned char*)data)[loop]];
		*(unsigned int*)((int)result + index) = color;
	}

	return result;
}

void __stdcall free_wrapper(void* blk)
{
	free(blk);
}

int OutputDDS(FILE *finput, const char* OutFilePath, unsigned int TexNumber, unsigned int RelativeStart, TPKToolInternalStruct *OutTPKToolInternal, GamePixelFormatStruct *OutGamePixelFormat, TexStruct *OutTexStruct, bool bByteSwap)
{
	bool bPal8 = false;
	void* P8Decoded = NULL;
	void* P8Pallete = NULL;
	unsigned int P8DecodedSize = 0;
	struct DirectX::DDS_HEADER DDSHeaderStruct = { 0 };
	struct DirectX::DDS_PIXELFORMAT DDSPixelFormatStruct = { 0 };
	//unsigned long int PreviousOffset = ftell(finput);
	DDSHeaderStruct.dwSize = 124;
	DDSHeaderStruct.dwFlags = 0x21007;
	DDSHeaderStruct.dwHeight = OutTexStruct[TexNumber].Child4.Height;
	DDSHeaderStruct.dwWidth = OutTexStruct[TexNumber].Child4.Width;
	DDSHeaderStruct.dwMipMapCount = OutTexStruct[TexNumber].Child4.NumMipMapLevels;
	/*if (bByteSwap)
	{
		// we don't support mipmaps on 360 yet :/
		DDSHeaderStruct.dwMipMapCount = 0;
	}*/
	//DDSHeaderStruct.dwMipMapCount = 0;
	DDSPixelFormatStruct.dwSize = 32;
	if (OutGamePixelFormat[TexNumber].FourCC == FOURCC_ARGB)
	{
		DDSPixelFormatStruct.dwFlags = 0x41;
		DDSPixelFormatStruct.dwRGBBitCount = 0x20;

		DDSPixelFormatStruct.dwRBitMask = 0xFF0000;
		DDSPixelFormatStruct.dwGBitMask = 0xFF00;
		DDSPixelFormatStruct.dwBBitMask = 0xFF;
		DDSPixelFormatStruct.dwABitMask = 0xFF000000;

		DDSHeaderStruct.dwCaps = 0x40100A;
	}
	else if (OutGamePixelFormat[TexNumber].FourCC == 0x29)
	{
		bPal8 = true;
		// reusing ARGB because we're converting P8 to ARGB
		DDSPixelFormatStruct.dwFlags = 0x41;
		DDSPixelFormatStruct.dwRGBBitCount = 0x20;

		DDSPixelFormatStruct.dwRBitMask = 0xFF0000;
		DDSPixelFormatStruct.dwGBitMask = 0xFF00;
		DDSPixelFormatStruct.dwBBitMask = 0xFF;
		DDSPixelFormatStruct.dwABitMask = 0xFF000000;

		DDSHeaderStruct.dwCaps = 0x40100A;
	}
	else
	{
		DDSPixelFormatStruct.dwFlags = 4;
		DDSPixelFormatStruct.dwFourCC = OutGamePixelFormat[TexNumber].FourCC;
		DDSHeaderStruct.dwCaps = 0x401008;
	}
	DDSHeaderStruct.ddspf = DDSPixelFormatStruct;
	void* OutDDSDataBuffer = malloc(OutTexStruct[TexNumber].Child4.ImageSize);

	//(*OutTPKToolInternal).DDSDataBuffer = malloc(OutTexStruct[TexNumber].Child4.ImageSize);
	
	fseek(finput, OutTexStruct[TexNumber].Child4.ImagePlacement + RelativeStart, SEEK_SET);
	//fread((*OutTPKToolInternal).DDSDataBuffer, sizeof(char), OutTexStruct[TexNumber].Child4.ImageSize, finput);
	fread(OutDDSDataBuffer, sizeof(char), OutTexStruct[TexNumber].Child4.ImageSize, finput);
	//fseek(finput, PreviousOffset, SEEK_SET);

	if (bPal8)
	{
		// read palette
		P8Pallete = calloc(sizeof(char), OutTexStruct[TexNumber].Child4.PaletteSize);
		fseek(finput, OutTexStruct[TexNumber].Child4.PalettePlacement + RelativeStart, SEEK_SET);
		fread(P8Pallete, sizeof(char), OutTexStruct[TexNumber].Child4.PaletteSize, finput);
		// decode palette
		P8Decoded = P8toRGBA((*OutTPKToolInternal).DDSDataBuffer, OutTexStruct[TexNumber].Child4.ImageSize, &P8DecodedSize, (unsigned int*)(P8Pallete));
		// remove the buffers and repoint to new data
		free(P8Pallete);
		free(OutDDSDataBuffer);
		(*OutTPKToolInternal).DDSDataBuffer = P8Decoded;
		OutTexStruct[TexNumber].Child4.ImageSize = P8DecodedSize;
	}

	FILE *fout = fopen(OutFilePath, "wb");
	unsigned int DDSMagic = DDS_MAGIC;
	fwrite(&DDSMagic, 4, 1, fout);
	//fseek(fout, 4, SEEK_CUR);
	fwrite(&DDSHeaderStruct, sizeof(DDSHeaderStruct), 1, fout);
	if (bByteSwap)
	{
		if (OutGamePixelFormat[TexNumber].FourCC == FOURCC_ARGB) // uncompressed = 32bit swap
			ByteSwapBuffer_Long(OutDDSDataBuffer, OutTexStruct[TexNumber].Child4.ImageSize);
		else
			ByteSwapBuffer_Short(OutDDSDataBuffer, OutTexStruct[TexNumber].Child4.ImageSize);

		if (OutTexStruct[TexNumber].bSwizzled)
		{
			//OutTexStruct[TexNumber].Child4.MipmapCount--;
			//if (strcmp(OutTexStruct[TexNumber].TexName, "ARC_LI_CLOTH09") == 0)
			//if (strcmp(OutTexStruct[TexNumber].TexName, "SGN_FWY_CASCADEPARK") == 0)
//			if (strcmp(OutTexStruct[TexNumber].TexName, "TRNS_ALL_LZ_PAVEMENTA_W") == 0)
//				bDEBUG = true;

			OutTexStruct[TexNumber].Child4.ImageSize = Deswizzle(OutDDSDataBuffer, OutTexStruct[TexNumber].Child4.ImageSize, OutTexStruct[TexNumber].Child4.Width, OutTexStruct[TexNumber].Child4.Height, OutTexStruct[TexNumber].Child4.NumMipMapLevels, DDSPixelFormatStruct.dwFourCC);
			// OutTexStruct[TexNumber].Child4.ImageSize = Deswizzle(OutDDSDataBuffer, OutTexStruct[TexNumber].Child4.ImageSize, OutTexStruct[TexNumber].Child4.Width, OutTexStruct[TexNumber].Child4.Height, OutTexStruct[TexNumber].Child4.NumMipMapLevels, DDSPixelFormatStruct.dwFourCC);
//			bDEBUG = false;
			//OutTexStruct[TexNumber].Child4.DataSize = Deswizzle_RecalculateSize(OutTexStruct[TexNumber].Child4.ResX, OutTexStruct[TexNumber].Child4.ResY, DDSPixelFormatStruct.dwFourCC);
		}
		else
		{
			printf("WRITING A LINEAR TEXTURE!!! %s\n", OutTexStruct[TexNumber].TexName);
			// detect if image is stretched across its linear buffer and restore it by its block size...
			OutTexStruct[TexNumber].Child4.ImageSize = Check360TexSize((uint8_t*)OutDDSDataBuffer, OutTexStruct[TexNumber].Child4.ImageSize, OutTexStruct[TexNumber].Child4.Width, OutTexStruct[TexNumber].Child4.Height, OutTexStruct[TexNumber].Child4.NumMipMapLevels, DDSPixelFormatStruct.dwFourCC);

		}
	}
	fwrite(OutDDSDataBuffer, sizeof(char), OutTexStruct[TexNumber].Child4.ImageSize, fout);
	//free_wrapper(OutDDSDataBuffer);
	//(*OutTPKToolInternal).DDSDataBuffer = NULL;

	fclose(fout);
	return 1;
}

int ReadChunkTypeAndSize(FILE *finput, unsigned int &ChunkMagic, unsigned int &ChunkSize)
{
	fread(&ChunkMagic, 4, 1, finput);
	fread(&ChunkSize, 4, 1, finput);
	return 1;
}

int ZeroChunkReader(FILE *finput, unsigned int ChunkSize)
{
	printf("Zero chunk: skipping %X bytes\n", ChunkSize);
	fseek(finput, ChunkSize, SEEK_CUR);
	return 1;
}

int TPKDataChildType2Reader(FILE *finput, unsigned int ChunkSize, const char* OutFolderPath, TPKToolInternalStruct *OutTPKToolInternal, GamePixelFormatStruct *OutGamePixelFormat, TexStruct *OutTexStruct, bool bByteSwap)
{
	struct stat st = { 0 }; // filestat for folder existence
	char InputFilePath[1024];
	char TPKHashPathString[13];
	strcpy(InputFilePath, OutFolderPath);
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Data Child 2 size: %X\n", ChunkSize);
	if (bByteSwap)
		printf("Byteswapping enabled.\n");

	unsigned short int Padding = 0;

	do
	{
		fread(&Padding, sizeof(short int), 1, finput);
	} while (Padding == 0x1111);

	fseek(finput, -2, SEEK_CUR); // moving back to where the data is...
	unsigned int RelativeStart = ftell(finput);
	//sprintf(TPKHashPathString, "%X", (*OutTPKToolInternal).HashArray[0]);

	OutTPKToolInternal->RelativeDDSDataOffset = RelativeStart;

	//strcpy((*OutTPKToolInternal).SettingsFileName, TPKHashPathString);
	//strcat((*OutTPKToolInternal).SettingsFileName, ".ini");
	strcpy((*OutTPKToolInternal).StatFileName, TPKHashPathString);
	strcat((*OutTPKToolInternal).StatFileName, "_statistics.txt");

	if ((ReadingMode != TPKTOOL_READINGMODE_PLAT_V2_PS2) && (ReadingMode != TPKTOOL_READINGMODE_PLAT_PS2))
	{
		strcat(InputFilePath, "\\");
		strcat(InputFilePath, TPKHashPathString);

		if (stat(InputFilePath, &st) == -1)
		{
			printf("Making directory %s\n", InputFilePath);
			_mkdir(InputFilePath);
		}
		for (unsigned int i = 0; i < (*OutTPKToolInternal).TextureCategoryHashCount; i++)
		{
			strcpy((*OutTPKToolInternal).TotalFilePath, InputFilePath); // i'm tired, i can't even anymore
			strcat((*OutTPKToolInternal).TotalFilePath, "\\");
			strcat((*OutTPKToolInternal).TotalFilePath, OutTexStruct[i].TexName);
			strcat((*OutTPKToolInternal).TotalFilePath, ".dds");

			if (bFileExists((*OutTPKToolInternal).TotalFilePath))
			{
				strcpy((*OutTPKToolInternal).TotalFilePath, DuplicateFileName((*OutTPKToolInternal).TotalFilePath));
			}

			printf("Outputting %s\n", (*OutTPKToolInternal).TotalFilePath);
			OutputDDS(finput, (*OutTPKToolInternal).TotalFilePath, i, RelativeStart, OutTPKToolInternal, OutGamePixelFormat, OutTexStruct, bByteSwap);

			strcpy(OutTexStruct[i].FilesystemPath, (*OutTPKToolInternal).TotalFilePath);
			fseek(finput, OutTexStruct[i].Child4.ImageSize, SEEK_CUR);
		}
		printf("Extraction finished.\n");
	}
	return 1;
}

int TPKDataChildType1Reader(FILE *finput, unsigned int ChunkSize, TPKLinkStruct *OutTPKLink)
{
	unsigned int TPKLinkCounter = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Data Child 1 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(&(*OutTPKLink).Unknown1, 4, 1, finput);
		fread(&(*OutTPKLink).Unknown2, 4, 1, finput);
		fread(&(*OutTPKLink).NumberOfTPKs, 4, 1, finput);
		fread(&(*OutTPKLink).TPKHash[TPKLinkCounter], 4, 1, finput);
		fread(&(*OutTPKLink).Unknown3, 4, 1, finput);
		fread(&(*OutTPKLink).Unknown4, 4, 1, finput);
		TPKLinkCounter++;
	}
	return 1;
}

int TPKDataChunkReader(FILE *finput, unsigned int ChunkSize, const char* OutFolderPath, TPKToolInternalStruct *OutTPKToolInternal, GamePixelFormatStruct *OutGamePixelFormat, TexStruct *OutTexStruct, TPKLinkStruct *OutTPKLink)
{
	printf("TPK Data chunk size: %X\n", ChunkSize);
	unsigned int Magic = 0;
	unsigned int Size = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	while (ftell(finput) < RelativeEnd)
	{
		ReadChunkTypeAndSize(finput, Magic, Size);
		switch (Magic)
		{
		case 0:
			ZeroChunkReader(finput, Size);
			break;
		case TPKDATA_CHILD1_CHUNKID:
			TPKDataChildType1Reader(finput, Size, OutTPKLink);
			break;
		case TPKDATA_CHILD2_CHUNKID:
			TPKDataChildType2Reader(finput, Size, OutFolderPath, OutTPKToolInternal, OutGamePixelFormat, OutTexStruct, false);
			break;
		case TPKDATA_CHILD3_CHUNKID: // byteswapped
			TPKDataChildType2Reader(finput, Size, OutFolderPath, OutTPKToolInternal, OutGamePixelFormat, OutTexStruct, true);
			break;
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size);
			fseek(finput, Size, SEEK_CUR);
			break;
		}
	}
	return 1;
}

int TPKAnimChildType2Reader(FILE *finput, unsigned int ChunkSize, TPKAnimStruct *OutTPKAnim, TPKToolInternalStruct *OutTPKToolInternal)
{
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Animation Child 2 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		for (unsigned int i = 0; i < OutTPKAnim[(*OutTPKToolInternal).AnimFrameCounter].Frames; i++)
		{
			fread(&(*OutTPKToolInternal).AnimFrameHashArray[(*OutTPKToolInternal).AnimFrameCounter][i], sizeof(int), 1, finput);
			fseek(finput, 0x8, SEEK_CUR);
		}
		(*OutTPKToolInternal).AnimFrameCounter++;
	}
	return 1;
}

int TPKAnimChildType1Reader(FILE *finput, unsigned int ChunkSize, TPKAnimStruct *OutTPKAnim, TPKToolInternalStruct *OutTPKToolInternal)
{
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Animation Child 1 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(&OutTPKAnim[(*OutTPKToolInternal).AnimCounter], sizeof(TPKAnimStruct), 1, finput);
		(*OutTPKToolInternal).AnimCounter++;
	}
	return 1;
}

int TPKAnimChunkReader(FILE *finput, unsigned int ChunkSize, TPKAnimStruct *OutTPKAnim, TPKToolInternalStruct *OutTPKToolInternal)
{
	printf("TPK Animation chunk size: %X\n", ChunkSize);
	unsigned int Magic = 0;
	unsigned int Size = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	while (ftell(finput) < RelativeEnd)
	{
		ReadChunkTypeAndSize(finput, Magic, Size);
		switch (Magic)
		{
		case TPK_ANIMCHILD1_CHUNKID:
			TPKAnimChildType1Reader(finput, Size, OutTPKAnim, OutTPKToolInternal);
			break;
		case TPK_ANIMCHILD2_CHUNKID:
			TPKAnimChildType2Reader(finput, Size, OutTPKAnim, OutTPKToolInternal);
			break;
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size);
			fseek(finput, Size, SEEK_CUR);
			break;
		}
	}
	return 1;
}

int TPKExtraChunkReader(FILE *finput, unsigned int ChunkSize, TPKAnimStruct *OutTPKAnim, TPKToolInternalStruct *OutTPKToolInternal)
{
	printf("TPK Extra chunk size: %X\n", ChunkSize);
	unsigned int Magic = 0;
	unsigned int Size = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	while (ftell(finput) < RelativeEnd)
	{
		ReadChunkTypeAndSize(finput, Magic, Size);
		switch (Magic)
		{
		case TPK_ANIM_CHUNKID:
			TPKAnimChunkReader(finput, Size, OutTPKAnim, OutTPKToolInternal);
			break;
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size);
			fseek(finput, Size, SEEK_CUR);
			break;
		}
	}
	return 1;
}

int TPKChildType5Reader(FILE *finput, unsigned int ChunkSize, GamePixelFormatStruct *OutGamePixelFormat)
{
	unsigned int TempBuffer = 0;
	unsigned int TexturePixelFormatCounter = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 5 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fseek(finput, 0xC, SEEK_CUR);
		fread(&OutGamePixelFormat[TexturePixelFormatCounter].FourCC, 4, 1, finput);
		fread(&OutGamePixelFormat[TexturePixelFormatCounter].Unknown1, 4, 1, finput);
		fread(&OutGamePixelFormat[TexturePixelFormatCounter].Unknown2, 4, 1, finput);
		/*fread(&texture[TexturePixelFormatCounter].GamePixelFormat.Unknown3, 4, 1, finput);
		fread(&texture[TexturePixelFormatCounter].GamePixelFormat.Unknown4, 4, 1, finput);
		fread(&texture[TexturePixelFormatCounter].GamePixelFormat.Unknown5, 4, 1, finput);*/
		TexturePixelFormatCounter++;
	}
	return 1;
}

int TPK_v2_360_ChildType5Reader(FILE *finput, unsigned int ChunkSize, GamePixelFormatStruct *OutGamePixelFormat, TexStruct* OutTexStruct)
{
	unsigned int TexturePixelFormatCounter = 0;

	TPKChild5Struct_v2_360* GamePixelFormatBridge = (TPKChild5Struct_v2_360*)calloc(1, sizeof(TPKChild5Struct_v2_360));
	D3DFORMAT inputformat;

	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 5 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(GamePixelFormatBridge, sizeof(TPKChild5Struct_v2_360), 1, finput);
		inputformat = (D3DFORMAT)((*GamePixelFormatBridge).D3DFormat);

		switch (inputformat)
		{
		case D3DFMT_A8R8G8B8:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_ARGB;
			break;
		case D3DFMT_LIN_A8R8G8B8:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_ARGB;
			break;
		case D3DFMT_DXT1:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT1;
			break;
		case D3DFMT_LIN_DXT1:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT1;
			break;
		case D3DFMT_DXT3:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT3;
			break;
		case D3DFMT_LIN_DXT3:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT3;
			break;
		case D3DFMT_DXT5:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT5;
			break;
		case D3DFMT_LIN_DXT5:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT5;
			break;
		case D3DFMT_DXN:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_ATI2;
			break;
		case D3DFMT_LIN_DXN:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_ATI2;
			break;
		case D3DFMT_L8:
			printf("WARNING: unsupported texture format: D3DFMT_L8\tTEXTURE[%d]: %s\n", TexturePixelFormatCounter, OutTexStruct[TexturePixelFormatCounter].TexName);
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_ATI2;
			break;
		case D3DFMT_LIN_L8:
			printf("WARNING: unsupported texture format: D3DFMT_LIN_L8\tTEXTURE[%d]: %s\n", TexturePixelFormatCounter, OutTexStruct[TexturePixelFormatCounter].TexName);
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_ATI2;
			break;
		default:
			// idk
			printf("WARNING: unknown texture format: 0x%X\tTEXTURE[%d]: %s\n", inputformat, TexturePixelFormatCounter, OutTexStruct[TexturePixelFormatCounter].TexName);
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT3;
			break;
		}
		OutTexStruct[TexturePixelFormatCounter].bSwizzled = XGIsTiledFormat(inputformat);

		// DEBUG ZONE DEBUG ZONE DEBUG ZONE
		char* dbgfname = (char*)calloc(strlen(OutTexStruct[TexturePixelFormatCounter].TexName) + 10, sizeof(char));
		sprintf(dbgfname, "%s.bin", OutTexStruct[TexturePixelFormatCounter].TexName);
		FILE* texformatfile = fopen(dbgfname, "wb");
		fwrite(GamePixelFormatBridge, sizeof(char), 32, texformatfile);
		fclose(texformatfile);
		free(dbgfname);
		// DEBUG ZONE DEBUG ZONE DEBUG ZONE

		TexturePixelFormatCounter++;
	}



	free(GamePixelFormatBridge);

	return 1;
}

int TPK_Xbox_ChildType5Reader(FILE *finput, unsigned int ChunkSize, GamePixelFormatStruct *OutGamePixelFormat, TexStruct* OutTexStruct)
{
	unsigned int TexturePixelFormatCounter = 0;

	TPKChild5Struct_Xbox* GamePixelFormatBridge = (TPKChild5Struct_Xbox*)calloc(1, sizeof(TPKChild5Struct_Xbox));

	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 5 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(GamePixelFormatBridge, sizeof(TPKChild5Struct_Xbox), 1, finput);
		switch ((*GamePixelFormatBridge).PixelFormat)
		{
		case 0xC: // DXT1
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT1;
			break;
		case 0xE: // DXT3
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT3;
			break;
		case 0xB: // P8
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = 0x29;
			break;
		case 0x6: // ARGB
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_ARGB;
			break;
		default:
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_ARGB;
			break;
		}
		

	/*	// DEBUG PRINTOUT
		printf("\nDEBUG: %s\n", OutTexStruct[TexturePixelFormatCounter].TexName);
		for (unsigned int i = 0; i < sizeof(TPKChild5Struct_Xbox); i++)
		{
			printf("%.2x ", (*GamePixelFormatBridge).Unknown1[i]);
		}
		printf("\nDEBUG\n");*/

		TexturePixelFormatCounter++;
	}

	free(GamePixelFormatBridge);

	return 1;
}

int TPK_PS3_ChildType5Reader(FILE *finput, unsigned int ChunkSize, GamePixelFormatStruct *OutGamePixelFormat, TexStruct* OutTexStruct)
{
	unsigned int TexturePixelFormatCounter = 0;

	TPKChild5Struct_PS3* GamePixelFormatBridge = (TPKChild5Struct_PS3*)calloc(1, sizeof(TPKChild5Struct_PS3));

	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 5 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(GamePixelFormatBridge, sizeof(TPKChild5Struct_PS3), 1, finput);

		if (((*GamePixelFormatBridge).PixelFormatVal1 == 0) && ((*GamePixelFormatBridge).PixelFormatVal2 == 3) && ((*GamePixelFormatBridge).PixelFormatVal3 == 3))
		{
			// DXT1
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT1;
		}

		else if (((*GamePixelFormatBridge).PixelFormatVal1 == 1) && ((*GamePixelFormatBridge).PixelFormatVal2 == 3) && ((*GamePixelFormatBridge).PixelFormatVal3 == 3))
		{
			// DXT5
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_DXT5;
		}

		else if (((*GamePixelFormatBridge).PixelFormatVal1 == 1) && ((*GamePixelFormatBridge).PixelFormatVal2 == 1) && ((*GamePixelFormatBridge).PixelFormatVal3 == 0))
		{
			// RGBA
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_ARGB;
		}

		else
		{
			// idk
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = FOURCC_ARGB;
		}

		TexturePixelFormatCounter++;
	}

	free(GamePixelFormatBridge);

	return 1;
}

int TPK_v2_ChildType5Reader(FILE *finput, unsigned int ChunkSize, GamePixelFormatStruct *OutGamePixelFormat, TexStruct* OutTexStruct)
{
	unsigned int TempBuffer = 0;
	unsigned int TexturePixelFormatCounter = 0;

	GamePixelFormatStruct_v2* GamePixelFormatBridge = (GamePixelFormatStruct_v2*)calloc(1, sizeof(GamePixelFormatStruct_v2));

	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 5 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(GamePixelFormatBridge, sizeof(GamePixelFormatStruct_v2), 1, finput);
		OutGamePixelFormat[TexturePixelFormatCounter].FourCC = (*GamePixelFormatBridge).FourCC;

		// repurpose v2 - use carbon pixel format to store the unknowns then write/read them in the ini
		OutGamePixelFormat[TexturePixelFormatCounter].Unknown1 = (*GamePixelFormatBridge).Unknown3;
		OutGamePixelFormat[TexturePixelFormatCounter].Unknown2 = (*GamePixelFormatBridge).Unknown4;
		OutGamePixelFormat[TexturePixelFormatCounter].Unknown3 = (*GamePixelFormatBridge).Unknown5;

		// REPURPOSING CHILD4's VALUES! IF CARBON IS USED, THESE ARE ZEROED OUT! HACK!
		//OutTexStruct[TexturePixelFormatCounter].Child4.Unknown10 = (*GamePixelFormatBridge).Unknown3;
		//OutTexStruct[TexturePixelFormatCounter].Child4.Unknown11 = (*GamePixelFormatBridge).Unknown4;
		//OutTexStruct[TexturePixelFormatCounter].Child4.Unknown12 = (*GamePixelFormatBridge).Unknown5;


		/*fseek(finput, 0xC, SEEK_CUR);
		fread(&OutGamePixelFormat[TexturePixelFormatCounter].FourCC, 4, 1, finput);
		fread(&OutGamePixelFormat[TexturePixelFormatCounter].Unknown1, 4, 1, finput);
		fread(&OutGamePixelFormat[TexturePixelFormatCounter].Unknown2, 4, 1, finput);*/
		/*fread(&texture[TexturePixelFormatCounter].GamePixelFormat.Unknown3, 4, 1, finput);
		fread(&texture[TexturePixelFormatCounter].GamePixelFormat.Unknown4, 4, 1, finput);
		fread(&texture[TexturePixelFormatCounter].GamePixelFormat.Unknown5, 4, 1, finput);*/
		TexturePixelFormatCounter++;
	}

	free(GamePixelFormatBridge);

	return 1;
}

int TPKChildType4Reader(FILE *finput, unsigned int ChunkSize, TexStruct* OutTexStruct, TPKToolInternalStruct *OutTPKToolInternal)
{
	unsigned char TexNameSize = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 4 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(&OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4, sizeof(TextureInfo), 1, finput);

		fread(&TexNameSize, 1, 1, finput);
		fread(&OutTexStruct[(*OutTPKToolInternal).TextureDataCount].TexName, 1, TexNameSize, finput);
		(*OutTPKToolInternal).TextureDataCount++;
	}
	return 1;
}

int TPK_v2_ChildType4Reader(FILE *finput, unsigned int ChunkSize, TexStruct* OutTexStruct, TPKToolInternalStruct *OutTPKToolInternal)
{
	// UG2 & MW

	//TPKChild4Struct_TPKv4 *TPKv4Child4_Bridge = (TPKChild4Struct_TPKv4*)calloc(1, sizeof(TPKChild4Struct_TPKv4));
	OldTextureInfo *TPKv4Child4_Bridge = (OldTextureInfo*)calloc(1, sizeof(OldTextureInfo));

	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 4 size: %X\n", ChunkSize);
	uint32_t tdc = OutTPKToolInternal->TextureDataCount;

	while (ftell(finput) < RelativeEnd)
	{
		fread(TPKv4Child4_Bridge, sizeof(OldTextureInfo), 1, finput);
		strncpy(OutTexStruct[tdc].TexName, (*TPKv4Child4_Bridge).DebugName, 0x18);
		OutTexStruct[tdc].Child4.NameHash = (*TPKv4Child4_Bridge).NameHash;
		OutTexStruct[tdc].Child4.ClassNameHash = (*TPKv4Child4_Bridge).ClassNameHash;
		OutTexStruct[tdc].Child4.ImagePlacement = (*TPKv4Child4_Bridge).ImagePlacement;
		OutTexStruct[tdc].Child4.PalettePlacement = (*TPKv4Child4_Bridge).PalettePlacement;
		OutTexStruct[tdc].Child4.ImageSize = (*TPKv4Child4_Bridge).ImageSize;
		OutTexStruct[tdc].Child4.PaletteSize = (*TPKv4Child4_Bridge).PaletteSize;
		OutTexStruct[tdc].Child4.BaseImageSize = (*TPKv4Child4_Bridge).BaseImageSize;
		OutTexStruct[tdc].Child4.Width = (*TPKv4Child4_Bridge).Width;
		OutTexStruct[tdc].Child4.Height = (*TPKv4Child4_Bridge).Height;
		OutTexStruct[tdc].Child4.ShiftWidth = (*TPKv4Child4_Bridge).ShiftWidth;
		OutTexStruct[tdc].Child4.ShiftHeight = (*TPKv4Child4_Bridge).ShiftHeight;
		OutTexStruct[tdc].Child4.ImageCompressionType = (*TPKv4Child4_Bridge).ImageCompressionType;
		OutTexStruct[tdc].Child4.PaletteCompressionType = (*TPKv4Child4_Bridge).PaletteCompressionType;
		OutTexStruct[tdc].Child4.NumPaletteEntries = (*TPKv4Child4_Bridge).NumPaletteEntries;
		OutTexStruct[tdc].Child4.NumMipMapLevels = (*TPKv4Child4_Bridge).NumMipMapLevels;
		OutTexStruct[tdc].Child4.TilableUV = (*TPKv4Child4_Bridge).TilableUV;
		OutTexStruct[tdc].Child4.BiasLevel = (*TPKv4Child4_Bridge).BiasLevel;
		OutTexStruct[tdc].Child4.RenderingOrder = (*TPKv4Child4_Bridge).RenderingOrder;
		OutTexStruct[tdc].Child4.ScrollType = (*TPKv4Child4_Bridge).ScrollType;
		OutTexStruct[tdc].Child4.UsedFlag = (*TPKv4Child4_Bridge).UsedFlag;
		OutTexStruct[tdc].Child4.ApplyAlphaSorting = (*TPKv4Child4_Bridge).ApplyAlphaSorting;
		OutTexStruct[tdc].Child4.AlphaUsageType = (*TPKv4Child4_Bridge).AlphaUsageType;
		OutTexStruct[tdc].Child4.AlphaBlendType = (*TPKv4Child4_Bridge).AlphaBlendType;
		OutTexStruct[tdc].Child4.Flags = (*TPKv4Child4_Bridge).Flags;
		//OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.MipmapBiasType = (*TPKv4Child4_Bridge).MipmapBiasType;
		//OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Padding = (*TPKv4Child4_Bridge).Unknown3;
		OutTexStruct[tdc].Child4.ScrollTimeStep = (*TPKv4Child4_Bridge).ScrollTimeStep;
		OutTexStruct[tdc].Child4.ScrollSpeedS = (*TPKv4Child4_Bridge).ScrollSpeedS;
		OutTexStruct[tdc].Child4.ScrollSpeedT = (*TPKv4Child4_Bridge).ScrollSpeedT;
		OutTexStruct[tdc].Child4.OffsetS = (*TPKv4Child4_Bridge).OffsetS;
		OutTexStruct[tdc].Child4.OffsetT = (*TPKv4Child4_Bridge).OffsetT;
		OutTexStruct[tdc].Child4.ScaleS = (*TPKv4Child4_Bridge).ScaleS;
		OutTexStruct[tdc].Child4.ScaleT = (*TPKv4Child4_Bridge).ScaleT;

		if (ReadingMode == TPKTOOL_READINGMODE_PLAT_V2_PS2)
		{
			if (bCompressed)
			{
				OutTexStruct[tdc].Child4.ImagePlacement = TIMoffsets.at(tdc);
				OutTexStruct[tdc].Child4.PalettePlacement = (*TPKv4Child4_Bridge).PalettePlacement + TIMoffsets.at(tdc);
			}
			else
			{
				OutTexStruct[tdc].Child4.PalettePlacement = (*TPKv4Child4_Bridge).PalettePlacement;
			}
		}
		tdc++;
	}

	OutTPKToolInternal->TextureDataCount = tdc;
	free(TPKv4Child4_Bridge);
	return 1;
}

// Code for compressed data start
#ifdef TPKTOOL_DECOMPRESSION
int CompressedChild4and5Reader_v2(unsigned char* InBuffer, unsigned int TexNumber, TexStruct *OutTexStruct, GamePixelFormatStruct *OutGamePixelFormat)
{
	
	//TPKChild4Struct_TPKv4 *TPKv4Child4_Bridge = (TPKChild4Struct_TPKv4*)calloc(1, sizeof(TPKChild4Struct_TPKv4));
	OldTextureInfo *TPKv4Child4_Bridge = (OldTextureInfo*)calloc(1, sizeof(OldTextureInfo));
	GamePixelFormatStruct_v2* GamePixelFormatBridge = (GamePixelFormatStruct_v2*)calloc(1, sizeof(GamePixelFormatStruct_v2));

	memcpy(TPKv4Child4_Bridge, InBuffer + 0x64, sizeof(OldTextureInfo));
	memcpy(GamePixelFormatBridge, InBuffer + 0x64 + sizeof(OldTextureInfo), sizeof(GamePixelFormatStruct_v2));

	strncpy(OutTexStruct[TexNumber].TexName, (*TPKv4Child4_Bridge).DebugName, 0x18);

	OutTexStruct[TexNumber].Child4.NameHash = (*TPKv4Child4_Bridge).NameHash;
	OutTexStruct[TexNumber].Child4.ClassNameHash = (*TPKv4Child4_Bridge).ClassNameHash;
	OutTexStruct[TexNumber].Child4.ImagePlacement = (*TPKv4Child4_Bridge).ImagePlacement;
	OutTexStruct[TexNumber].Child4.PalettePlacement = (*TPKv4Child4_Bridge).PalettePlacement;
	OutTexStruct[TexNumber].Child4.ImageSize = (*TPKv4Child4_Bridge).ImageSize;
	OutTexStruct[TexNumber].Child4.PaletteSize = (*TPKv4Child4_Bridge).PaletteSize;
	OutTexStruct[TexNumber].Child4.BaseImageSize = (*TPKv4Child4_Bridge).BaseImageSize;
	OutTexStruct[TexNumber].Child4.Width = (*TPKv4Child4_Bridge).Width;
	OutTexStruct[TexNumber].Child4.Height = (*TPKv4Child4_Bridge).Height;
	OutTexStruct[TexNumber].Child4.ShiftWidth = (*TPKv4Child4_Bridge).ShiftWidth;
	OutTexStruct[TexNumber].Child4.ShiftHeight = (*TPKv4Child4_Bridge).ShiftHeight;
	OutTexStruct[TexNumber].Child4.ImageCompressionType = (*TPKv4Child4_Bridge).ImageCompressionType;
	OutTexStruct[TexNumber].Child4.PaletteCompressionType = (*TPKv4Child4_Bridge).PaletteCompressionType;
	OutTexStruct[TexNumber].Child4.NumPaletteEntries = (*TPKv4Child4_Bridge).NumPaletteEntries;
	OutTexStruct[TexNumber].Child4.NumMipMapLevels = (*TPKv4Child4_Bridge).NumMipMapLevels;
	OutTexStruct[TexNumber].Child4.TilableUV = (*TPKv4Child4_Bridge).TilableUV;
	OutTexStruct[TexNumber].Child4.BiasLevel = (*TPKv4Child4_Bridge).BiasLevel;
	OutTexStruct[TexNumber].Child4.RenderingOrder = (*TPKv4Child4_Bridge).RenderingOrder;
	OutTexStruct[TexNumber].Child4.ScrollType = (*TPKv4Child4_Bridge).ScrollType;
	OutTexStruct[TexNumber].Child4.UsedFlag = (*TPKv4Child4_Bridge).UsedFlag;
	OutTexStruct[TexNumber].Child4.ApplyAlphaSorting = (*TPKv4Child4_Bridge).ApplyAlphaSorting;
	OutTexStruct[TexNumber].Child4.AlphaUsageType = (*TPKv4Child4_Bridge).AlphaUsageType;
	OutTexStruct[TexNumber].Child4.AlphaBlendType = (*TPKv4Child4_Bridge).AlphaBlendType;
	OutTexStruct[TexNumber].Child4.Flags = (*TPKv4Child4_Bridge).Flags;
	//OutTexStruct[TexNumber].Child4.MipmapBiasType = (*TPKv4Child4_Bridge).MipmapBiasType;
	//OutTexStruct[TexNumber].Child4.Padding = (*TPKv4Child4_Bridge).Unknown3;
	OutTexStruct[TexNumber].Child4.ScrollTimeStep = (*TPKv4Child4_Bridge).ScrollTimeStep;
	OutTexStruct[TexNumber].Child4.ScrollSpeedS = (*TPKv4Child4_Bridge).ScrollSpeedS;
	OutTexStruct[TexNumber].Child4.ScrollSpeedT = (*TPKv4Child4_Bridge).ScrollSpeedT;
	OutTexStruct[TexNumber].Child4.OffsetS = (*TPKv4Child4_Bridge).OffsetS;
	OutTexStruct[TexNumber].Child4.OffsetT = (*TPKv4Child4_Bridge).OffsetT;
	OutTexStruct[TexNumber].Child4.ScaleS = (*TPKv4Child4_Bridge).ScaleS;
	OutTexStruct[TexNumber].Child4.ScaleT = (*TPKv4Child4_Bridge).ScaleT;

	OutGamePixelFormat[TexNumber].FourCC = (*GamePixelFormatBridge).FourCC;
	OutGamePixelFormat[TexNumber].Unknown1 = (*GamePixelFormatBridge).Unknown3;
	OutGamePixelFormat[TexNumber].Unknown2 = (*GamePixelFormatBridge).Unknown4;
	OutGamePixelFormat[TexNumber].Unknown3 = (*GamePixelFormatBridge).Unknown5;


	// REPURPOSING CHILD4's VALUES! IF CARBON IS USED, THESE ARE ZEROED OUT! HACK!
	//OutTexStruct[TexNumber].Child4.Unknown10 = (*GamePixelFormatBridge).Unknown3;
	//OutTexStruct[TexNumber].Child4.Unknown11 = (*GamePixelFormatBridge).Unknown4;
	//OutTexStruct[TexNumber].Child4.Unknown12 = (*GamePixelFormatBridge).Unknown5;

	free(GamePixelFormatBridge);
	free(TPKv4Child4_Bridge);

	return 0;
}

int CompressedChild4and5Reader(unsigned char* InBuffer, unsigned int TexNumber, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormat)
{
	unsigned char TexNameSize = 0;
	memcpy(&InTexStruct[TexNumber].Child4, InBuffer, sizeof(TextureInfo));
	memcpy(&TexNameSize, InBuffer + sizeof(TextureInfo), 1);
	memcpy(&InTexStruct[TexNumber].TexName, InBuffer + sizeof(TextureInfo) + 1, TexNameSize);
	memcpy(&InGamePixelFormat[TexNumber].FourCC, InBuffer + sizeof(TextureInfo) + 0x30, sizeof(int));
	return 0;
}

volatile void __stdcall PatchCode()
{
	for (unsigned int i = 0; i < 0x66E8; i++)
		if (*(unsigned int*)(DecompressionCode + i) == (unsigned int)0x0DB8FF0)
			*(unsigned int*)(DecompressionCode + i) = (unsigned int)&BogusPointer;

	injector::MakeCALL(DecompressionCode + 0x8E8, memset, false);
	injector::MakeCALL(DecompressionCode + 0x8FC, memset, false);
	injector::MakeCALL(DecompressionCode + 0xE00, memcpy, false);
	injector::MakeCALL(DecompressionCode + 0xE69, memcpy, false);
}

unsigned int LZDecompress(unsigned char *InBuffer, unsigned char *OutBuffer)
{
	if (!bCodePatched)
	{
		DWORD OldProtectTemp = 0;
		PatchCode();
		if (!VirtualProtect(&DecompressionCode, DecompressionCode_len, PAGE_EXECUTE_READ, &OldProtectTemp))
			printf("VirtualProtect failed during code patching!");
		bCodePatched = 1;
	}
	// WARNING: x64 INCOMPATIBILITY!
	return Ripped_LZDecompress(InBuffer, OutBuffer);
}

unsigned int ExtractCompressedBlock(unsigned char *InBuffer, unsigned char *OutBuffer, CompressBlockHead CompressFile)
{
	memcpy(&CompressFile, InBuffer, sizeof(CompressBlockHead));
	if (CompressFile.CompressBlockMagic != 0x55441122)
		return 0;

	LZDecompress(InBuffer + sizeof(CompressBlockHead), OutBuffer);
	return CompressFile.TotalBlockSize;
}

void GenerateOutputFileName_Type3DDS(char* OutHashFolder, TPKToolInternalStruct *InTPKToolInternal)
{
	struct stat st = { 0 }; // filestat for file/folder existence
	char TPKHashPathString[13];

	if ((*InTPKToolInternal).TotalFilePath[strlen((*InTPKToolInternal).TotalFilePath) - 1] != '\\') // FIXME: fix all strings in this abomination, 12/2017
		strcat((*InTPKToolInternal).TotalFilePath, "\\");

	sprintf(TPKHashPathString, "%X", (*InTPKToolInternal).HashArray[0]);

	strcpy(OutHashFolder, (*InTPKToolInternal).TotalFilePath);
	strcat(OutHashFolder, TPKHashPathString);
	strcat(OutHashFolder, "\\");

	if (!bDoCompressedStringsOnce)
	{

		strcpy((*InTPKToolInternal).SettingsFileName, TPKHashPathString);
		strcat((*InTPKToolInternal).SettingsFileName, ".ini");
		strcpy((*InTPKToolInternal).StatFileName, TPKHashPathString);
		strcat((*InTPKToolInternal).StatFileName, "_statistics.txt");

		if (stat(OutHashFolder, &st) == -1)
		{
			printf("Making directory %s\n", OutHashFolder);
			_mkdir(OutHashFolder);
		}
		bDoCompressedStringsOnce = 1;
	}
}


int TPKChildType3DDSOutputter(void* DDSDataBuffer, unsigned int TexNumber, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormatStruct)
{

	char OutFileName[MAX_PATH];
	char OutHashFolder[MAX_PATH];

	GenerateOutputFileName_Type3DDS(OutHashFolder, InTPKToolInternal);


	strcpy(OutFileName, OutHashFolder);
	strcat(OutFileName, InTexStruct[TexNumber].TexName);
	strcat(OutFileName, ".dds");

	if (bFileExists(OutFileName))
	{
		strcpy(OutFileName, DuplicateFileName(OutFileName));
	}

	printf("Outputting %s\n", OutFileName);

	strcpy(InTexStruct[TexNumber].FilesystemPath, OutFileName);

	return OutputDDSFromMemory(OutFileName, TexNumber, DDSDataBuffer, InTexStruct, InGamePixelFormatStruct);
}

int TPKChildType3Reader(FILE *finput, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormatStruct)
{
	//printf("TPK Child 3 size: %X\nSorry, compressed texture data not fully supported yet. Skipping...\nFIXME: add support, add option to extract manually\n", ChunkSize);
	//fseek(finput, ChunkSize, SEEK_CUR);
	printf("TPK Child 3 size: %X\n", ChunkSize);
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	unsigned int SavedOffset = 0;
	unsigned char *InBuffer, *OutBuffer, *InfoBuffer;
	unsigned int NumberOfCompBlocks = 0;
	unsigned long long int CompBlockLocations[0xFFFF];
	unsigned int i = 0;
	unsigned int TextureDataSize = 0;
	unsigned int WrittenSize = 0;
	unsigned int PartSize[0xFFFF];
	TPKChild3Struct Child3;
	CompressBlockHead CompressFile1, *CompressFile2;
	bCompressed = 1;

	do
	{
		fread(&Child3, sizeof(TPKChild3Struct), 1, finput);
		SavedOffset = ftell(finput);
		fseek(finput, Child3.AbsoluteOffset, SEEK_SET);
		// do stuff here
		InBuffer = (unsigned char*)malloc(Child3.Size);
		fread(InBuffer, Child3.Size, 1, finput);
		fseek(finput, SavedOffset, SEEK_SET); // input file is unnecessary at this point

		NumberOfCompBlocks = 0;
		i = 0;
		TextureDataSize = 0;
		WrittenSize = 0;

		while (i < Child3.Size)
		{
			memcpy(&CompressFile1, InBuffer + i, sizeof(CompressBlockHead));
			if (CompressFile1.CompressBlockMagic == 0x55441122)
			{
				CompBlockLocations[NumberOfCompBlocks] = ((unsigned long long int)InBuffer) + i;
				i += CompressFile1.TotalBlockSize;
				PartSize[NumberOfCompBlocks] = CompressFile1.OutSize;
				TextureDataSize += CompressFile1.OutSize;
				NumberOfCompBlocks++;
			}
			else
				i++;
		}

		CompressFile2 = (CompressBlockHead*)malloc(NumberOfCompBlocks * sizeof(CompressBlockHead));

		for (i = 0; i < NumberOfCompBlocks; i++)
		{
			memcpy(&CompressFile2[i], (void*)CompBlockLocations[i], sizeof(CompressBlockHead));
		}


		TextureDataSize -= PartSize[NumberOfCompBlocks - 2];
		OutBuffer = (unsigned char*)malloc(TextureDataSize);
		ExtractCompressedBlock((unsigned char*)CompBlockLocations[NumberOfCompBlocks - 1], OutBuffer, CompressFile2[NumberOfCompBlocks - 1]); // the last compress block = texture start
		WrittenSize += PartSize[NumberOfCompBlocks - 1];

		for (i = 0; i < NumberOfCompBlocks - 2; i++) // the rest is in order until second to last block (which is the info block)
		{
			ExtractCompressedBlock((unsigned char*)CompBlockLocations[i], OutBuffer + WrittenSize, CompressFile2[i]);
			WrittenSize += PartSize[i];
		}

		InfoBuffer = (unsigned char*)malloc(PartSize[NumberOfCompBlocks - 2]);
		ExtractCompressedBlock((unsigned char*)CompBlockLocations[NumberOfCompBlocks - 2], InfoBuffer, CompressFile2[NumberOfCompBlocks - 2]);

		CompressedChild4and5Reader(InfoBuffer, (*InTPKToolInternal).TextureDataCount, InTexStruct, InGamePixelFormatStruct);
		TPKChildType3DDSOutputter(OutBuffer, (*InTPKToolInternal).TextureDataCount, InTPKToolInternal, InTexStruct, InGamePixelFormatStruct);

		free(CompressFile2);
		free(OutBuffer);
		free(InfoBuffer);
		free(InBuffer);
		(*InTPKToolInternal).TextureDataCount++;
	} while (ftell(finput) < RelativeEnd);

	return 1;
}


int TPK_v2_ChildType3Reader(FILE *finput, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormatStruct)
{
	//printf("TPK Child 3 size: %X\nSorry, compressed texture data not fully supported yet. Skipping...\nFIXME: add support, add option to extract manually\n", ChunkSize);
	//fseek(finput, ChunkSize, SEEK_CUR);
	printf("TPK Child 3 size: %X\n", ChunkSize);
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	unsigned int SavedOffset = 0;
	unsigned char *InBuffer, *OutBuffer, *InfoBuffer, *DDSDataBuffer;
	TPKChild3Struct Child3;
	bCompressed = 1;

	do
	{
		fread(&Child3, sizeof(TPKChild3Struct), 1, finput);
		SavedOffset = ftell(finput);
		fseek(finput, Child3.AbsoluteOffset, SEEK_SET);
		// do stuff here
		InBuffer = (unsigned char*)malloc(Child3.Size);
		OutBuffer = (unsigned char*)malloc(Child3.OutSize);
		InfoBuffer = (unsigned char*)malloc(Child3.FromEndToHeaderOffset);

		fread(InBuffer, Child3.Size, 1, finput);
		fseek(finput, SavedOffset, SEEK_SET); // input file is unnecessary at this point

	//	NumberOfCompBlocks = 0;

		LZDecompress(InBuffer, OutBuffer);
		
		memcpy(InfoBuffer, OutBuffer + (Child3.OutSize - Child3.FromEndToHeaderOffset), Child3.FromEndToHeaderOffset);

		CompressedChild4and5Reader_v2(InfoBuffer, (*InTPKToolInternal).TextureDataCount, InTexStruct, InGamePixelFormatStruct);
		
		DDSDataBuffer = (unsigned char*)malloc(InTexStruct[(*InTPKToolInternal).TextureDataCount].Child4.ImageSize);
		memcpy(DDSDataBuffer, OutBuffer, InTexStruct[(*InTPKToolInternal).TextureDataCount].Child4.ImageSize);
		free(OutBuffer);

		TPKChildType3DDSOutputter(DDSDataBuffer, (*InTPKToolInternal).TextureDataCount, InTPKToolInternal, InTexStruct, InGamePixelFormatStruct);

		free(DDSDataBuffer);
		free(InfoBuffer);
		free(InBuffer);
		(*InTPKToolInternal).TextureDataCount++;
	} while (ftell(finput) < RelativeEnd);

	return 1;
}

int TPK_v2_ChildType3Reader_PS2_MW(FILE* finput, unsigned int ChunkSize, TPKToolInternalStruct* InTPKToolInternal, TexStruct* InTexStruct, GamePixelFormatStruct* InGamePixelFormatStruct)
{
	printf("TPK Child 3 size: %X\n", ChunkSize);
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	unsigned int SavedOffset = 0;
	unsigned char* InBuffer, * OutBuffer, * DDSDataBuffer;
	TPKChild3Struct Child3;
	bCompressed = true;
	bUG2_PS2 = true;
	uint32_t tdc = 0;

	sprintf(InTPKToolInternal->SettingsFileName, "%X", InTPKToolInternal->HashArray[0]);
	strcat(InTPKToolInternal->SettingsFileName, ".ini");

	if (TIMDataName.length() == 0)
	{
		TIMDataName = InTPKToolInternal->TotalFilePath;
		TIMDataName += "\\";
		TIMDataName += "timdata.bin";

	}
	FILE* ftim = fopen(TIMDataName.c_str(), "wb");

	do
	{
		fread(&Child3, sizeof(TPKChild3Struct), 1, finput);
		SavedOffset = ftell(finput);
		fseek(finput, Child3.AbsoluteOffset, SEEK_SET);

		if (Child3.FromEndToHeaderOffset)
		{
			JDLZhead cmphdr;
			fread(&cmphdr, sizeof(JDLZhead), 1, finput);
			fseek(finput, Child3.AbsoluteOffset, SEEK_SET);

			InBuffer = (unsigned char*)malloc(Child3.Size);
			OutBuffer = (unsigned char*)malloc(cmphdr.OutSize);
			//InfoBuffer = (unsigned char*)malloc(Child3.FromEndToHeaderOffset);

			fread(InBuffer, Child3.Size, 1, finput);
			fseek(finput, SavedOffset, SEEK_SET); // input file is unnecessary at this point

			LZDecompress(InBuffer, OutBuffer);

			OldTextureInfo* ti = (OldTextureInfo*)((uint64_t)(OutBuffer) + cmphdr.OutSize - 0xD0); // address this via 64 bits properly

			//memcpy(&(InTexStruct[tdc].Child4), ti, sizeof(OldTextureInfo));

			InTexStruct[tdc].Child4.NameHash = ti->NameHash;
			InTexStruct[tdc].Child4.ClassNameHash = ti->ClassNameHash;
			InTexStruct[tdc].Child4.ImagePlacement = ftell(ftim);
			if (ti->PalettePlacement > 0)
				InTexStruct[tdc].Child4.PalettePlacement = ftell(ftim) + ti->PalettePlacement;
			else
				InTexStruct[tdc].Child4.PalettePlacement = ti->PalettePlacement;
			InTexStruct[tdc].Child4.ImageSize = ti->ImageSize;
			InTexStruct[tdc].Child4.PaletteSize = ti->PaletteSize;
			InTexStruct[tdc].Child4.BaseImageSize = ti->BaseImageSize;
			InTexStruct[tdc].Child4.Width = ti->Width;
			InTexStruct[tdc].Child4.Height = ti->Height;
			InTexStruct[tdc].Child4.ShiftWidth = ti->ShiftWidth;
			InTexStruct[tdc].Child4.ShiftHeight = ti->ShiftHeight;
			InTexStruct[tdc].Child4.ImageCompressionType = ti->ImageCompressionType;
			InTexStruct[tdc].Child4.PaletteCompressionType = ti->PaletteCompressionType;
			InTexStruct[tdc].Child4.NumPaletteEntries = ti->NumPaletteEntries;
			InTexStruct[tdc].Child4.NumMipMapLevels = ti->NumMipMapLevels;
			InTexStruct[tdc].Child4.TilableUV = ti->TilableUV;
			InTexStruct[tdc].Child4.BiasLevel = ti->BiasLevel;
			InTexStruct[tdc].Child4.RenderingOrder = ti->RenderingOrder;
			InTexStruct[tdc].Child4.ScrollType = ti->ScrollType;
			InTexStruct[tdc].Child4.UsedFlag = ti->UsedFlag;
			InTexStruct[tdc].Child4.ApplyAlphaSorting = ti->ApplyAlphaSorting;
			InTexStruct[tdc].Child4.AlphaUsageType = ti->AlphaUsageType;
			InTexStruct[tdc].Child4.AlphaBlendType = ti->AlphaBlendType;
			InTexStruct[tdc].Child4.Flags = ti->Flags;
			//OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.MipmapBiasType = ti->MipmapBiasType;
			//OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Padding = ti->Unknown3;
			InTexStruct[tdc].Child4.ScrollTimeStep = ti->ScrollTimeStep;
			InTexStruct[tdc].Child4.ScrollSpeedS = ti->ScrollSpeedS;
			InTexStruct[tdc].Child4.ScrollSpeedT = ti->ScrollSpeedT;
			InTexStruct[tdc].Child4.OffsetS = ti->OffsetS;
			InTexStruct[tdc].Child4.OffsetT = ti->OffsetT;
			InTexStruct[tdc].Child4.ScaleS = ti->ScaleS;
			InTexStruct[tdc].Child4.ScaleT = ti->ScaleT;

			strcpy_s(InTexStruct[tdc].TexName, ti->DebugName);

			//InTexStruct[tdc].Child4.

			TIMoffsets.push_back(ftell(ftim));
			fwrite(OutBuffer, cmphdr.OutSize - 0xD0, 1, ftim);


			free(OutBuffer);
			free(InBuffer);
		}
		else
		{
			fseek(finput, Child3.AbsoluteOffset, SEEK_SET);
			InBuffer = (unsigned char*)malloc(Child3.Size);
			fread(InBuffer, Child3.Size, 1, finput);
			fseek(finput, SavedOffset, SEEK_SET); // input file is unnecessary at this point

			OldTextureInfo* ti = (OldTextureInfo*)((uint64_t)(InBuffer)+Child3.Size - 0xD0); // address this via 64 bits properly
			memcpy(&(InTexStruct[tdc].Child4), ti, sizeof(OldTextureInfo));

			TIMoffsets.push_back(ftell(ftim));
			fwrite(InBuffer, Child3.Size, 1, ftim);

			free(InBuffer);
		}

		tdc++;
		//(*InTPKToolInternal).TextureDataCount++;
	} while (ftell(finput) < RelativeEnd);

	InTPKToolInternal->TextureDataCount = tdc;

	fclose(ftim);
	return 1;
}

int TPK_v2_ChildType3Reader_PS2(FILE* finput, unsigned int ChunkSize, TPKToolInternalStruct* InTPKToolInternal, TexStruct* InTexStruct, GamePixelFormatStruct* InGamePixelFormatStruct)
{
	printf("TPK Child 3 size: %X\n", ChunkSize);
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	unsigned int SavedOffset = 0;
	unsigned char* InBuffer, * OutBuffer, * DDSDataBuffer;
	TPKChild3_v2_Struct Child3;
	bCompressed = true;

	sprintf(InTPKToolInternal->SettingsFileName, "%X", InTPKToolInternal->HashArray[0]);
	strcat(InTPKToolInternal->SettingsFileName, ".ini");

	if (TIMDataName.length() == 0)
	{
		TIMDataName = InTPKToolInternal->TotalFilePath;
		TIMDataName += "\\";
		TIMDataName += "timdata.bin";

	}
	FILE* ftim = fopen(TIMDataName.c_str(), "wb");

	do
	{
		fread(&Child3, sizeof(TPKChild3_v2_Struct), 1, finput);
		SavedOffset = ftell(finput);
		fseek(finput, Child3.AbsoluteOffset, SEEK_SET);

		if (Child3.comp)
		{
			JDLZhead cmphdr;
			fread(&cmphdr, sizeof(JDLZhead), 1, finput);
			fseek(finput, Child3.AbsoluteOffset, SEEK_SET);

			InBuffer = (unsigned char*)malloc(Child3.Size);
			OutBuffer = (unsigned char*)malloc(cmphdr.OutSize);
			//InfoBuffer = (unsigned char*)malloc(Child3.FromEndToHeaderOffset);

			fread(InBuffer, Child3.Size, 1, finput);
			fseek(finput, SavedOffset, SEEK_SET); // input file is unnecessary at this point
			uint32_t test = 0;
			fread(&test, sizeof(uint32_t), 1, finput);
			if (test != 0)
				fseek(finput, -4, SEEK_CUR);
			else
				bUG2_PS2 = true;

			LZDecompress(InBuffer, OutBuffer);

			TIMoffsets.push_back(ftell(ftim));
			fwrite(OutBuffer, cmphdr.OutSize, 1, ftim);


			free(OutBuffer);
			free(InBuffer);
		}
		else
		{
			fseek(finput, Child3.AbsoluteOffset, SEEK_SET);
			InBuffer = (unsigned char*)malloc(Child3.Size);
			fread(InBuffer, Child3.Size, 1, finput);
			fseek(finput, SavedOffset, SEEK_SET); // input file is unnecessary at this point
			uint32_t test = 0;
			fread(&test, sizeof(uint32_t), 1, finput);
			if (test != 0)
				fseek(finput, -4, SEEK_CUR);

			TIMoffsets.push_back(ftell(ftim));
			fwrite(InBuffer, Child3.Size, 1, ftim);
			
			free(InBuffer);
		}


		//(*InTPKToolInternal).TextureDataCount++;
	} while (ftell(finput) < RelativeEnd);
	fclose(ftim);
	return 1;
}


#else
int TPKChildType3Reader(FILE *finput, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormatStruct)
{
	printf("TPK Child 3 size: %X\nSorry, decompression was disabled.\nTo enable, compile with TPKTOOL_DECOMPRESSION and provide DecompressionCode.h.\nSkipping...\n", ChunkSize);
	fseek(finput, ChunkSize, SEEK_CUR);
	return 1;
}
#endif
// Code for compressed data end

int TPKChildType2Reader(FILE *finput, unsigned int ChunkSize, TPKToolInternalStruct *OutTPKToolInternal)
{
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	unsigned int HashCounter = 0;
	printf("TPK Child 2 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(&(*OutTPKToolInternal).TextureCategoryHashArray[HashCounter], 4, 1, finput);
		fseek(finput, 4, SEEK_CUR);
		HashCounter++;
	}
	printf("Number of texture hashes in array: %d\n", HashCounter);
	for (unsigned int i = 0; i <= HashCounter - 1; i++)
		printf("Texture %d hash: %X\n", i, (*OutTPKToolInternal).TextureCategoryHashArray[i]);
	(*OutTPKToolInternal).TextureCategoryHashCount = HashCounter;
	return 1;
}

int TPKChildType1Reader(FILE *finput, unsigned int ChunkSize, TPKToolInternalStruct *OutTPKToolInternal)
{
	printf("TPK Child 1 size: %X\n", ChunkSize);
	while (ftell(finput) < ChunkSize)
	{
		//fseek(finput, 4, SEEK_CUR);
		fread(&(*OutTPKToolInternal).TPKTypeValue, 1, sizeof(int), finput);

		if (OutTPKToolInternal->TPKTypeValue == 3)
		{
			fread((*OutTPKToolInternal).TPKTypeName, 1, 0x20, finput);
			printf("TPK Type name: %s\n", (*OutTPKToolInternal).TPKTypeName);
			fread((*OutTPKToolInternal).TPKPathName, 1, 0x58, finput);
			printf("TPK Path: %s\n", (*OutTPKToolInternal).TPKPathName);
			OutTPKToolInternal->HashArray[0] = 0xDEADBEEF;
		}
		else
		{
			fread((*OutTPKToolInternal).TPKTypeName, 1, TPK_TYPENAME_SIZE, finput);
			printf("TPK Type name: %s\n", (*OutTPKToolInternal).TPKTypeName);
			fread((*OutTPKToolInternal).TPKPathName, 1, TPK_PATHNAME_SIZE, finput);
			printf("TPK Path: %s\n", (*OutTPKToolInternal).TPKPathName);
			fread((*OutTPKToolInternal).HashArray, 4, 7, finput);
		}
	}
	printf("TPK hash: %X\n", OutTPKToolInternal->HashArray[0]);
	return 1;
}

int TPKChunkReader(FILE *finput, unsigned int ChunkSize, TexStruct *OutTexStruct, TPKToolInternalStruct *OutTPKToolInternal, GamePixelFormatStruct *OutGamePixelFormat, TPKAnimStruct* OutTPKAnim)
{
	printf("TPK main chunk size: %X\n", ChunkSize);
	unsigned int Magic = 0;
	unsigned int Size = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	while (ftell(finput) < RelativeEnd)
	{
		ReadChunkTypeAndSize(finput, Magic, Size);
		switch (Magic)
		{
			/*case 0:
			ZeroChunkReader(finput, Size);
			break;*/
		case TPK_CHILD1_CHUNKID:
			TPKChildType1Reader(finput, Size, OutTPKToolInternal);
			break;
		case TPK_CHILD2_CHUNKID:
			TPKChildType2Reader(finput, Size, OutTPKToolInternal);
			break;
		case TPK_CHILD3_CHUNKID:
			switch (ReadingMode)
			{
			case TPKTOOL_READINGMODE_PLAT_V2_360:
			case TPKTOOL_READINGMODE_V2:
				TPK_v2_ChildType3Reader(finput, Size, OutTPKToolInternal, OutTexStruct, OutGamePixelFormat);
				break;
			case TPKTOOL_READINGMODE_PLAT_V2_PS2:
				if (bMW_PS2)
					TPK_v2_ChildType3Reader_PS2_MW(finput, Size, OutTPKToolInternal, OutTexStruct, OutGamePixelFormat);
				else
					TPK_v2_ChildType3Reader_PS2(finput, Size, OutTPKToolInternal, OutTexStruct, OutGamePixelFormat);
				break;
			default:
				TPKChildType3Reader(finput, Size, OutTPKToolInternal, OutTexStruct, OutGamePixelFormat);
				break;
			}
			break;
		case TPK_CHILD4_CHUNKID:
			switch (ReadingMode)
			{
			case TPKTOOL_READINGMODE_PLAT_V2_PS2:
			case TPKTOOL_READINGMODE_V2:
				TPK_v2_ChildType4Reader(finput, Size, OutTexStruct, OutTPKToolInternal);
				break;
			case TPKTOOL_READINGMODE_PLAT_V2_360:
				TPK_v2_ChildType4Reader(finput, Size, OutTexStruct, OutTPKToolInternal);
				break;
			default:
				TPKChildType4Reader(finput, Size, OutTexStruct, OutTPKToolInternal);
				break;
			}
			break;
		case TPK_CHILD5_CHUNKID:
			switch (ReadingMode)
			{
			case TPKTOOL_READINGMODE_V2:
				TPK_v2_ChildType5Reader(finput, Size, OutGamePixelFormat, OutTexStruct);
				break;
			case TPKTOOL_READINGMODE_PLAT_PS3:
				TPK_PS3_ChildType5Reader(finput, Size, OutGamePixelFormat, OutTexStruct);
				break;
			case TPKTOOL_READINGMODE_PLAT_V2_360:
				TPK_v2_360_ChildType5Reader(finput, Size, OutGamePixelFormat, OutTexStruct);
				break;
			case TPKTOOL_READINGMODE_PLAT_360:
				TPK_v2_360_ChildType5Reader(finput, Size, OutGamePixelFormat, OutTexStruct);
				break;
			case TPKTOOL_READINGMODE_PLAT_XBOX:
				TPK_Xbox_ChildType5Reader(finput, Size, OutGamePixelFormat, OutTexStruct);
				break;
			case TPKTOOL_READINGMODE_PLAT_V2_PS2:
				printf("Skipping child 5 chunk size %X\n", Magic, Size);
				fseek(finput, Size, SEEK_CUR);
			    break;
			default:
				TPKChildType5Reader(finput, Size, OutGamePixelFormat);
				break;
			}
			break;
		case TPK_EXTRACAPSULE_CHUNKID:
			TPKExtraChunkReader(finput, Size, OutTPKAnim, OutTPKToolInternal);
			break;
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size);
			fseek(finput, Size, SEEK_CUR);
			break;

		}
	}
	return 1;
}

int TPK_HP2_ChildType4Bridge(FILE* finput, unsigned int ChunkSize, TexStruct* OutTexStruct, TPKToolInternalStruct* OutTPKToolInternal)
{
	// UG2 & MW

//TPKChild4Struct_TPKv4 *TPKv4Child4_Bridge = (TPKChild4Struct_TPKv4*)calloc(1, sizeof(TPKChild4Struct_TPKv4));
	//OldTextureInfo* TPKv4Child4_Bridge = (OldTextureInfo*)calloc(1, sizeof(OldTextureInfo));
	AncientTextureInfo TPK_HP2_Bridge = { 0 };

	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("HP2 TPK Chunk 3 size: %X\n", ChunkSize);
	uint32_t tdc = OutTPKToolInternal->TextureDataCount;

	while (ftell(finput) < RelativeEnd)
	{
		fread(&TPK_HP2_Bridge, sizeof(AncientTextureInfo), 1, finput);

		strcpy_s(OutTexStruct[tdc].TexName, TPK_HP2_Bridge.DebugName);
		OutTexStruct[tdc].Child4.NameHash = TPK_HP2_Bridge.NameHash;
		OutTexStruct[tdc].Child4.ImagePlacement = TPK_HP2_Bridge.ImagePlacement;
		OutTexStruct[tdc].Child4.PalettePlacement = TPK_HP2_Bridge.PalettePlacement;
		OutTexStruct[tdc].Child4.ImageSize = TPK_HP2_Bridge.ImageSize;
		OutTexStruct[tdc].Child4.PaletteSize = TPK_HP2_Bridge.PaletteSize;
		//OutTexStruct[tdc].Child4.BaseImageSize = TPK_HP2_Bridge.BaseImageSize;
		//OutTexStruct[tdc].Child4.Width = (uint16_t)pow(2, TPK_HP2_Bridge.ShiftWidth);
		//OutTexStruct[tdc].Child4.Height = (uint16_t)pow(2, TPK_HP2_Bridge.ShiftHeight);
		OutTexStruct[tdc].Child4.Width = TPK_HP2_Bridge.Width;
		OutTexStruct[tdc].Child4.Height = TPK_HP2_Bridge.Height;
		OutTexStruct[tdc].Child4.ShiftWidth = TPK_HP2_Bridge.ShiftWidth;
		OutTexStruct[tdc].Child4.ShiftHeight = TPK_HP2_Bridge.ShiftHeight;
		OutTexStruct[tdc].Child4.ImageCompressionType = TPK_HP2_Bridge.ImageCompressionType;
		OutTexStruct[tdc].Child4.Unknown = TPK_HP2_Bridge.swizzled;
		//OutTexStruct[tdc].Child4.PaletteCompressionType = TPK_HP2_Bridge.PaletteCompressionType;
		//OutTexStruct[tdc].Child4.NumPaletteEntries = (*TPKv4Child4_Bridge).NumPaletteEntries;
		//OutTexStruct[tdc].Child4.NumMipMapLevels = (*TPKv4Child4_Bridge).NumMipMapLevels;
		//OutTexStruct[tdc].Child4.TilableUV = (*TPKv4Child4_Bridge).TilableUV;
		//OutTexStruct[tdc].Child4.BiasLevel = (*TPKv4Child4_Bridge).BiasLevel;
		OutTexStruct[tdc].Child4.RenderingOrder = TPK_HP2_Bridge.RenderingOrder;
		//OutTexStruct[tdc].Child4.ScrollType = TPK_HP2_Bridge.ScrollType;
		//OutTexStruct[tdc].Child4.UsedFlag = (*TPKv4Child4_Bridge).UsedFlag;
		//OutTexStruct[tdc].Child4.ApplyAlphaSorting = TPK_HP2_Bridge.ApplyAlphaSorting;
		OutTexStruct[tdc].Child4.AlphaUsageType = TPK_HP2_Bridge.AlphaUsageType;
		//OutTexStruct[tdc].Child4.AlphaBlendType = (*TPKv4Child4_Bridge).AlphaBlendType;
		//OutTexStruct[tdc].Child4.Flags = (*TPKv4Child4_Bridge).Flags;
		//OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.MipmapBiasType = (*TPKv4Child4_Bridge).MipmapBiasType;
		//OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Padding = (*TPKv4Child4_Bridge).Unknown3;
		//OutTexStruct[tdc].Child4.ScrollTimeStep = (*TPKv4Child4_Bridge).ScrollTimeStep;
		//OutTexStruct[tdc].Child4.ScrollSpeedS = (*TPKv4Child4_Bridge).ScrollSpeedS;
		//OutTexStruct[tdc].Child4.ScrollSpeedT = (*TPKv4Child4_Bridge).ScrollSpeedT;
		//OutTexStruct[tdc].Child4.OffsetS = (*TPKv4Child4_Bridge).OffsetS;
		//OutTexStruct[tdc].Child4.OffsetT = (*TPKv4Child4_Bridge).OffsetT;
		//OutTexStruct[tdc].Child4.ScaleS = (*TPKv4Child4_Bridge).ScaleS;
		//OutTexStruct[tdc].Child4.ScaleT = (*TPKv4Child4_Bridge).ScaleT;

		//if (ReadingMode == TPKTOOL_READINGMODE_PLAT_V2_PS2)
		//{
		//	if (bCompressed)
		//	{
		//		OutTexStruct[tdc].Child4.ImagePlacement = TIMoffsets.at(tdc);
		//		OutTexStruct[tdc].Child4.PalettePlacement = (*TPKv4Child4_Bridge).PalettePlacement + TIMoffsets.at(tdc);
		//	}
		//	else
		//	{
		//		OutTexStruct[tdc].Child4.PalettePlacement = (*TPKv4Child4_Bridge).PalettePlacement;
		//	}
		//}
		tdc++;
	}

	OutTPKToolInternal->TextureDataCount = tdc;

	return 1;
}

int TPK_HP2_ChunkReader(FILE* finput, unsigned int ChunkSize, TexStruct* OutTexStruct, TPKToolInternalStruct* OutTPKToolInternal, GamePixelFormatStruct* OutGamePixelFormat, TPKAnimStruct* OutTPKAnim)
{
	printf("HP2 TPK main chunk size: %X\n", ChunkSize);
	unsigned int Magic = 0;
	unsigned int Size = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	unsigned short int Padding = 0;
	while (ftell(finput) < RelativeEnd)
	{
		ReadChunkTypeAndSize(finput, Magic, Size);
		switch (Magic)
		{
		case TPK_HP2_CHUNK2_CHUNKID:
			fread(OutTPKToolInternal->TPKTypeName, 1, Size, finput);
			//printf("TPK Type name: %s\n", (*OutTPKToolInternal).TPKTypeName);
			cout << "TPK Type name: " << OutTPKToolInternal->TPKTypeName << '\n';

			break;
		case TPK_HP2_CHUNK3_CHUNKID:
			TPK_HP2_ChildType4Bridge(finput, Size, OutTexStruct, OutTPKToolInternal);
			break;
		case TPK_HP2_CHUNK4_CHUNKID:
			do
			{
				fread(&Padding, sizeof(short int), 1, finput);
			} while (Padding == 0x1111);

			fseek(finput, -2, SEEK_CUR); // moving back to where the data is...
			OutTPKToolInternal->RelativeDDSDataOffset = ftell(finput);
			fseek(finput, Size, SEEK_CUR);
			break;
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size);
			fseek(finput, Size, SEEK_CUR);
			break;

		}
	}
	return 1;
}

int MasterChunkReader(const char* FileName, const char* OutFolderPath, TPKToolInternalStruct *OutTPKToolInternal, TexStruct* OutTexStruct, GamePixelFormatStruct *OutGamePixelFormat, TPKAnimStruct* OutTPKAnim, TPKLinkStruct *OutTPKLink)
{
	if (!bFileExists(FileName))
		return -1;

	FILE *finput = fopen(FileName, "rb");
	unsigned int Magic = 0;
	unsigned int Size = 0;
	unsigned int Size2 = 0;
	fseek(finput, 0, SEEK_SET);
	ReadChunkTypeAndSize(finput, Magic, Size);
	printf("Reading the master TPK capsule: %X bytes\n", Size);
	while (ftell(finput) < Size)
	{
		ReadChunkTypeAndSize(finput, Magic, Size2);
		switch (Magic)
		{
		case 0:
			ZeroChunkReader(finput, Size2);
			break;
		case TPK_CHUNKID:
			TPKChunkReader(finput, Size2, OutTexStruct, OutTPKToolInternal, OutGamePixelFormat, OutTPKAnim);
			break;
		case TPKDATA_CHUNKID:
			if (bCompressed)
			{
				printf("Skipping data chunk size %X due to compression...\n", Size2);
				fseek(finput, Size2, SEEK_CUR);
			}
			else
				TPKDataChunkReader(finput, Size2, OutFolderPath, OutTPKToolInternal, OutGamePixelFormat, OutTexStruct, OutTPKLink);
			break;
		case TPKDATA_CHILD1_CHUNKID:
			if ((OutTPKToolInternal->TPKTypeValue == 3) && !bCompressed)
			{
				unsigned int RelativeEnd = ftell(finput) + Size2;
				unsigned short int Padding = 0;
				do
				{
					fread(&Padding, sizeof(short int), 1, finput);
				} while (Padding == 0x1111);

				fseek(finput, -2, SEEK_CUR); // moving back to where the data is...
				unsigned int RelativeStart = ftell(finput);
				OutTPKToolInternal->RelativeDDSDataOffset = RelativeStart;
				fseek(finput, RelativeEnd, SEEK_SET);
				break;
			}
		case TPK_HP2_CHUNK2_CHUNKID:
			ReadingMode = TPKTOOL_READINGMODE_HP2;
			cout << "Detected HP2 TPK chunk. Going to HP2 mode." << '\n';
			fseek(finput, -8, SEEK_CUR);
			TPK_HP2_ChunkReader(finput, Size, OutTexStruct, OutTPKToolInternal, OutGamePixelFormat, OutTPKAnim);
			break;
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size2);
			fseek(finput, Size2, SEEK_CUR);
			break;
		}
	}
	fclose(finput);
	return 1;
}