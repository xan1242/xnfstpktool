#pragma once
#include "stdafx.h"
#include <stdlib.h>
#include <direct.h>

// uncomment this to enable decompression code, requires DecompressionCode.h which contains propriatery game code
// place DecompressionCode.h in the project folder (also named TPKTool)
#define TPKTOOL_DECOMPRESSION

#ifdef TPKTOOL_DECOMPRESSION
#include "includes\injector\injector.hpp"
#include "DecompressionCode.h"
int(__cdecl *Ripped_LZDecompress)(unsigned char* InBuffer, unsigned char* OutBuffer) = (int(__cdecl*)(unsigned char*, unsigned char*))(DecompressionCode + 0x6670);
unsigned int BogusPointer = 0;
bool bCodePatched = 0;
bool bCompressed = 0;
bool bDoCompressedStringsOnce = 0;

/*int OutputDDSFromMemory(const char* OutFilePath, TexStruct InTexture, GamePixelFormatStruct InGamePixelFormat, DirectX::DDS_HEADER InDDSHeaderStruct, DirectX::DDS_PIXELFORMAT InDDSPixelFormatStruct, unsigned int TexNumber, void* DDSDataBuffer)
{
	InDDSHeaderStruct.dwSize = 124;
	InDDSHeaderStruct.dwFlags = 0x21007;
	InDDSHeaderStruct.dwHeight = InTexture.Child4.ResY;
	InDDSHeaderStruct.dwWidth = InTexture.Child4.ResX;
	InDDSHeaderStruct.dwMipMapCount = InTexture.Child4.MipmapCount;
	//InDDSHeaderStruct.dwMipMapCount = 0;
	InDDSPixelFormatStruct.dwSize = 32;
	if (InGamePixelFormat.FourCC == 0x15)
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
	DDSHeaderStruct.dwHeight = InTexStruct[TexNumber].Child4.ResY;
	DDSHeaderStruct.dwWidth = InTexStruct[TexNumber].Child4.ResX;
	DDSHeaderStruct.dwMipMapCount = InTexStruct[TexNumber].Child4.MipmapCount;
	//DDSHeaderStruct.dwMipMapCount = 0;
	DDSPixelFormatStruct.dwSize = 32;
	if (InGamePixelFormat[TexNumber].FourCC == 0x15)
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
	fwrite(DDSDataBuffer, sizeof(char), InTexStruct[TexNumber].Child4.DataSize, fout);

	fclose(fout);
	return 1;
}

#endif

int ReadingMode = 0;

int OutputDDS(FILE *finput, const char* OutFilePath, unsigned int TexNumber, unsigned int RelativeStart, TPKToolInternalStruct *OutTPKToolInternal, GamePixelFormatStruct *OutGamePixelFormat, TexStruct *OutTexStruct, bool bByteSwap)
{
	struct DirectX::DDS_HEADER DDSHeaderStruct = { 0 };
	struct DirectX::DDS_PIXELFORMAT DDSPixelFormatStruct = { 0 };
	//unsigned long int PreviousOffset = ftell(finput);
	DDSHeaderStruct.dwSize = 124;
	DDSHeaderStruct.dwFlags = 0x21007;
	DDSHeaderStruct.dwHeight = OutTexStruct[TexNumber].Child4.ResY;
	DDSHeaderStruct.dwWidth = OutTexStruct[TexNumber].Child4.ResX;
	DDSHeaderStruct.dwMipMapCount = OutTexStruct[TexNumber].Child4.MipmapCount;
	//DDSHeaderStruct.dwMipMapCount = 0;
	DDSPixelFormatStruct.dwSize = 32;
	if (OutGamePixelFormat[TexNumber].FourCC == 0x15)
	{
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
	(*OutTPKToolInternal).DDSDataBuffer = malloc(OutTexStruct[TexNumber].Child4.DataSize);
	fseek(finput, OutTexStruct[TexNumber].Child4.DataOffset + RelativeStart, SEEK_SET);
	fread((*OutTPKToolInternal).DDSDataBuffer, sizeof(char), OutTexStruct[TexNumber].Child4.DataSize, finput);
	//fseek(finput, PreviousOffset, SEEK_SET);

	FILE *fout = fopen(OutFilePath, "wb");
	unsigned int DDSMagic = DDS_MAGIC;
	fwrite(&DDSMagic, 4, 1, fout);
	//fseek(fout, 4, SEEK_CUR);
	fwrite(&DDSHeaderStruct, sizeof(DDSHeaderStruct), 1, fout);
	if (bByteSwap)
	{
		ByteSwapBuffer_Short((*OutTPKToolInternal).DDSDataBuffer, OutTexStruct[TexNumber].Child4.DataSize);
		Deswizzle((*OutTPKToolInternal).DDSDataBuffer, OutTexStruct[TexNumber].Child4.ResX, OutTexStruct[TexNumber].Child4.ResY, OutTexStruct[TexNumber].Child4.MipmapCount, DDSPixelFormatStruct.dwFourCC);
		OutTexStruct[TexNumber].Child4.DataSize = Deswizzle_RecalculateSize(OutTexStruct[TexNumber].Child4.ResX, OutTexStruct[TexNumber].Child4.ResY, DDSPixelFormatStruct.dwFourCC);
	}
	fwrite((*OutTPKToolInternal).DDSDataBuffer, sizeof(char), OutTexStruct[TexNumber].Child4.DataSize, fout);
	free((*OutTPKToolInternal).DDSDataBuffer);

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
	sprintf(TPKHashPathString, "%X", (*OutTPKToolInternal).HashArray[0]);


	strcpy((*OutTPKToolInternal).SettingsFileName, TPKHashPathString);
	strcat((*OutTPKToolInternal).SettingsFileName, ".ini");
	strcpy((*OutTPKToolInternal).StatFileName, TPKHashPathString);
	strcat((*OutTPKToolInternal).StatFileName, "_statistics.txt");


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
		fseek(finput, OutTexStruct[i].Child4.DataSize, SEEK_CUR);
	}
	printf("Extraction finished.\n");
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

	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 5 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(GamePixelFormatBridge, sizeof(TPKChild5Struct_v2_360), 1, finput);

		switch ((*GamePixelFormatBridge).SomeVal3)
		{
		case 0x18280186:
			// RGBA
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = 0x15;
			break;
		case 0x1A200152:
			// DXT1
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = 0x31545844;
			break;
		case 0x1A200153:
			// DXT3
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = 0x33545844;
			break;
		case 0x1A200154:
			// DXT5
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = 0x35545844;
			break;
		default:
			// idk
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = 0x15;
			break;
		}

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
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = 0x31545844;
		}

		else if (((*GamePixelFormatBridge).PixelFormatVal1 == 1) && ((*GamePixelFormatBridge).PixelFormatVal2 == 3) && ((*GamePixelFormatBridge).PixelFormatVal3 == 3))
		{
			// DXT5
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = 0x35545844;
		}

		else if (((*GamePixelFormatBridge).PixelFormatVal1 == 1) && ((*GamePixelFormatBridge).PixelFormatVal2 == 1) && ((*GamePixelFormatBridge).PixelFormatVal3 == 0))
		{
			// RGBA
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = 0x15;
		}

		else
		{
			// idk
			OutGamePixelFormat[TexturePixelFormatCounter].FourCC = 0x15;
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

		// REPURPOSING CHILD4's VALUES! IF CARBON IS USED, THESE ARE ZEROED OUT! HACK!
		OutTexStruct[TexturePixelFormatCounter].Child4.Unknown10 = (*GamePixelFormatBridge).Unknown3;
		OutTexStruct[TexturePixelFormatCounter].Child4.Unknown11 = (*GamePixelFormatBridge).Unknown4;
		OutTexStruct[TexturePixelFormatCounter].Child4.Unknown12 = (*GamePixelFormatBridge).Unknown5;


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
		fread(&OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4, sizeof(TPKChild4Struct), 1, finput);

		fread(&TexNameSize, 1, 1, finput);
		fread(&OutTexStruct[(*OutTPKToolInternal).TextureDataCount].TexName, 1, TexNameSize, finput);
		(*OutTPKToolInternal).TextureDataCount++;
	}
	return 1;
}

int TPK_v2_ChildType4Reader(FILE *finput, unsigned int ChunkSize, TexStruct* OutTexStruct, TPKToolInternalStruct *OutTPKToolInternal)
{
	// UG2 & MW

	TPKChild4Struct_TPKv2 *TPKv2Child4_Bridge = (TPKChild4Struct_TPKv2*)calloc(1, sizeof(TPKChild4Struct_TPKv2));

	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 4 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(TPKv2Child4_Bridge, sizeof(TPKChild4Struct_TPKv2), 1, finput);
		strncpy(OutTexStruct[(*OutTPKToolInternal).TextureDataCount].TexName, (*TPKv2Child4_Bridge).TexName, 0x18);
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Hash = (*TPKv2Child4_Bridge).Hash;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Hash2 = (*TPKv2Child4_Bridge).Hash2;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.DataOffset = (*TPKv2Child4_Bridge).DataOffset;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown14 = (*TPKv2Child4_Bridge).Unknown14;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.DataSize = (*TPKv2Child4_Bridge).DataSize;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown1 = (*TPKv2Child4_Bridge).Unknown1;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Scaler = (*TPKv2Child4_Bridge).Scaler;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.ResX = (*TPKv2Child4_Bridge).ResX;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.ResY = (*TPKv2Child4_Bridge).ResY;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.MipmapCount = (*TPKv2Child4_Bridge).MipmapCount;

//		memcpy(OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown3, TPKv2Child4_Bridge[(*OutTPKToolInternal).TextureDataCount].Unknown3, 3);

		//OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.TexFlags = (*TPKv2Child4_Bridge).TexFlags;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.UnkByteVal1 = (*TPKv2Child4_Bridge).UnkByteVal1;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.UnkByteVal2 = (*TPKv2Child4_Bridge).UnkByteVal2;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.UnkByteVal3 = (*TPKv2Child4_Bridge).UnkByteVal3;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown17 = (*TPKv2Child4_Bridge).Unknown17;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown18 = (*TPKv2Child4_Bridge).Unknown18;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown3 = (*TPKv2Child4_Bridge).Unknown3;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown4 = (*TPKv2Child4_Bridge).Unknown4;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown5 = (*TPKv2Child4_Bridge).Unknown5;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown6 = (*TPKv2Child4_Bridge).Unknown6;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown7 = (*TPKv2Child4_Bridge).Unknown7;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown8 = (*TPKv2Child4_Bridge).Unknown8;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown9 = (*TPKv2Child4_Bridge).Unknown9;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown10 = (*TPKv2Child4_Bridge).Unknown10;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown11 = (*TPKv2Child4_Bridge).Unknown11;
		OutTexStruct[(*OutTPKToolInternal).TextureDataCount].Child4.Unknown12 = (*TPKv2Child4_Bridge).Unknown12;


		(*OutTPKToolInternal).TextureDataCount++;
	}
	free(TPKv2Child4_Bridge);
	return 1;
}

// Code for compressed data start
#ifdef TPKTOOL_DECOMPRESSION
int CompressedChild4and5Reader_v2(unsigned char* InBuffer, unsigned int TexNumber, TexStruct *OutTexStruct, GamePixelFormatStruct *OutGamePixelFormat)
{
	
	TPKChild4Struct_TPKv2 *TPKv2Child4_Bridge = (TPKChild4Struct_TPKv2*)calloc(1, sizeof(TPKChild4Struct_TPKv2));
	GamePixelFormatStruct_v2* GamePixelFormatBridge = (GamePixelFormatStruct_v2*)calloc(1, sizeof(GamePixelFormatStruct_v2));

	memcpy(TPKv2Child4_Bridge, InBuffer + 0x64, sizeof(TPKChild4Struct_TPKv2));
	memcpy(GamePixelFormatBridge, InBuffer + 0x64 + sizeof(TPKChild4Struct_TPKv2), sizeof(GamePixelFormatStruct_v2));

	strncpy(OutTexStruct[TexNumber].TexName, (*TPKv2Child4_Bridge).TexName, 0x18);
	OutTexStruct[TexNumber].Child4.Hash = (*TPKv2Child4_Bridge).Hash;
	OutTexStruct[TexNumber].Child4.Hash2 = (*TPKv2Child4_Bridge).Hash2;
	OutTexStruct[TexNumber].Child4.DataOffset = (*TPKv2Child4_Bridge).DataOffset;
	OutTexStruct[TexNumber].Child4.Unknown14 = (*TPKv2Child4_Bridge).Unknown14;
	OutTexStruct[TexNumber].Child4.DataSize = (*TPKv2Child4_Bridge).DataSize;
	OutTexStruct[TexNumber].Child4.Unknown1 = (*TPKv2Child4_Bridge).Unknown1;
	OutTexStruct[TexNumber].Child4.Scaler = (*TPKv2Child4_Bridge).Scaler;
	OutTexStruct[TexNumber].Child4.ResX = (*TPKv2Child4_Bridge).ResX;
	OutTexStruct[TexNumber].Child4.ResY = (*TPKv2Child4_Bridge).ResY;
	OutTexStruct[TexNumber].Child4.MipmapCount = (*TPKv2Child4_Bridge).MipmapCount;
	//OutTexStruct[TexNumber].Child4.TexFlags = (*TPKv2Child4_Bridge).TexFlags;
	OutTexStruct[TexNumber].Child4.UnkByteVal1 = (*TPKv2Child4_Bridge).UnkByteVal1;
	OutTexStruct[TexNumber].Child4.UnkByteVal2 = (*TPKv2Child4_Bridge).UnkByteVal2;
	OutTexStruct[TexNumber].Child4.UnkByteVal3 = (*TPKv2Child4_Bridge).UnkByteVal3;
	OutTexStruct[TexNumber].Child4.Unknown17 = (*TPKv2Child4_Bridge).Unknown17;
	OutTexStruct[TexNumber].Child4.Unknown18 = (*TPKv2Child4_Bridge).Unknown18;
	OutTexStruct[TexNumber].Child4.Unknown3 = (*TPKv2Child4_Bridge).Unknown3;
	OutTexStruct[TexNumber].Child4.Unknown4 = (*TPKv2Child4_Bridge).Unknown4;
	OutTexStruct[TexNumber].Child4.Unknown5 = (*TPKv2Child4_Bridge).Unknown5;
	OutTexStruct[TexNumber].Child4.Unknown6 = (*TPKv2Child4_Bridge).Unknown6;
	OutTexStruct[TexNumber].Child4.Unknown7 = (*TPKv2Child4_Bridge).Unknown7;
	OutTexStruct[TexNumber].Child4.Unknown8 = (*TPKv2Child4_Bridge).Unknown8;
	OutTexStruct[TexNumber].Child4.Unknown9 = (*TPKv2Child4_Bridge).Unknown9;
	OutTexStruct[TexNumber].Child4.Unknown10 = (*TPKv2Child4_Bridge).Unknown10;
	OutTexStruct[TexNumber].Child4.Unknown11 = (*TPKv2Child4_Bridge).Unknown11;
	OutTexStruct[TexNumber].Child4.Unknown12 = (*TPKv2Child4_Bridge).Unknown12;

	OutGamePixelFormat[TexNumber].FourCC = (*GamePixelFormatBridge).FourCC;

	// REPURPOSING CHILD4's VALUES! IF CARBON IS USED, THESE ARE ZEROED OUT! HACK!
	OutTexStruct[TexNumber].Child4.Unknown10 = (*GamePixelFormatBridge).Unknown3;
	OutTexStruct[TexNumber].Child4.Unknown11 = (*GamePixelFormatBridge).Unknown4;
	OutTexStruct[TexNumber].Child4.Unknown12 = (*GamePixelFormatBridge).Unknown5;

	free(GamePixelFormatBridge);
	free(TPKv2Child4_Bridge);

	return 0;
}

int CompressedChild4and5Reader(unsigned char* InBuffer, unsigned int TexNumber, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormat)
{
	unsigned char TexNameSize = 0;
	memcpy(&InTexStruct[TexNumber].Child4, InBuffer, sizeof(TPKChild4Struct));
	memcpy(&TexNameSize, InBuffer + sizeof(TPKChild4Struct), 1);
	memcpy(&InTexStruct[TexNumber].TexName, InBuffer + sizeof(TPKChild4Struct) + 1, TexNameSize);
	memcpy(&InGamePixelFormat[TexNumber].FourCC, InBuffer + sizeof(TPKChild4Struct) + 0x30, sizeof(int));
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
		
		DDSDataBuffer = (unsigned char*)malloc(InTexStruct[(*InTPKToolInternal).TextureDataCount].Child4.DataSize);
		memcpy(DDSDataBuffer, OutBuffer, InTexStruct[(*InTPKToolInternal).TextureDataCount].Child4.DataSize);
		free(OutBuffer);

		TPKChildType3DDSOutputter(DDSDataBuffer, (*InTPKToolInternal).TextureDataCount, InTPKToolInternal, InTexStruct, InGamePixelFormatStruct);

		free(DDSDataBuffer);
		free(InfoBuffer);
		free(InBuffer);
		(*InTPKToolInternal).TextureDataCount++;
	} while (ftell(finput) < RelativeEnd);

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
		fread((*OutTPKToolInternal).TPKTypeName, 1, TPK_TYPENAME_SIZE, finput);
		printf("TPK Type name: %s\n", (*OutTPKToolInternal).TPKTypeName);
		fread((*OutTPKToolInternal).TPKPathName, 1, TPK_PATHNAME_SIZE, finput);
		printf("TPK Path: %s\n", (*OutTPKToolInternal).TPKPathName);
		fread((*OutTPKToolInternal).HashArray, 4, 7, finput);
	}
	for (unsigned int i = 0; i <= 6; i++)
		printf("TPK hash array [%d] = %X\n", i, (*OutTPKToolInternal).HashArray[i]);
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
			case TPKTOOL_READINGMODE_V2:
				TPK_v2_ChildType3Reader(finput, Size, OutTPKToolInternal, OutTexStruct, OutGamePixelFormat);
				break;
			case TPKTOOL_READINGMODE_PLAT_V2_360:
				TPK_v2_ChildType3Reader(finput, Size, OutTPKToolInternal, OutTexStruct, OutGamePixelFormat);
				break;
			default:
				TPKChildType3Reader(finput, Size, OutTPKToolInternal, OutTexStruct, OutGamePixelFormat);
				break;
			}
			break;
		case TPK_CHILD4_CHUNKID:
			switch (ReadingMode)
			{
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
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size2);
			fseek(finput, Size2, SEEK_CUR);
			break;
		}
	}
	fclose(finput);
	return 1;
}