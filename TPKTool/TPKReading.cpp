#include <stdio.h>
#include "TPKReading.h"
#include "TPKTool.h"

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

bool CheckIfVaildFile(FILE *finput)
{
	unsigned long int oldoffset = ftell(finput);
	unsigned int Magic = 0;
	unsigned int Size = 0;
	ReadChunkTypeAndSize(finput, Magic, Size);
	fseek(finput, oldoffset, SEEK_SET);
	if (Magic != TPKCAPSULE_CHUNKID || Size == 0)
		return 0;
	else
		return 1;
}

int OutputDDS(const char* OutFilePath, unsigned int TexNumber, unsigned int RelativeStart)
{
	DDSHeaderStruct.dwSize = 124;
	DDSHeaderStruct.dwFlags = 0x21007;
	DDSHeaderStruct.dwHeight = texture[TexNumber].ResY;
	DDSHeaderStruct.dwWidth = texture[TexNumber].ResX;
	DDSHeaderStruct.dwMipMapCount = texture[TexNumber].MipmapCount;
	DDSPixelFormatStruct.dwSize = 32;
	if (GamePixelFormat[TexNumber].FourCC == 0x15)
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
		DDSPixelFormatStruct.dwFourCC = GamePixelFormat[TexNumber].FourCC;
		DDSHeaderStruct.dwCaps = 0x401008;
	}
	DDSHeaderStruct.ddspf = DDSPixelFormatStruct;
	DDSDataBuffer = malloc(texture[TexNumber].DataSize);
	fseek(fin1, texture[TexNumber].DataOffset + RelativeStart, SEEK_SET);
	fread(DDSDataBuffer, sizeof(char), texture[TexNumber].DataSize, fin1);
	FILE *fout = NULL;
	fout = fopen(OutFilePath, "wb");
	fwrite(&DDSMagic, 4, 1, fout);
	fwrite(&DDSHeaderStruct, sizeof(DDSHeaderStruct), 1, fout);
	fwrite(DDSDataBuffer, sizeof(char), texture[TexNumber].DataSize, fout);
	fclose(fout);
	return 1;
}

int TPKDataChildType2Reader(FILE *finput, unsigned int ChunkSize)
{
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	char TPKHashPathString[9];
	printf("TPK Data Child 2 size: %X\n", ChunkSize);
	fseek(finput, 0x78, SEEK_CUR);
	unsigned int RelativeStart = ftell(finput);
	sprintf(TPKHashPathString, "%X", HashArray[0]);

	strcpy(SettingsFileName, TPKHashPathString);
	strcat(SettingsFileName, ".ini");
	strcpy(StatFileName, TPKHashPathString);
	strcat(StatFileName, "_statistics");
	strcat(StatFileName, ".txt");

	strcat(OutputFilePath, "\\");
	strcat(OutputFilePath, TPKHashPathString);
	if (stat(OutputFilePath, &st) == -1)
	{
		printf("Making directory %s\n", OutputFilePath);
		mkdir(OutputFilePath);
	}
	for (unsigned int i = 0; i <= TextureHashCount - 1; i++)
	{
		strcpy(TotalFilePath, OutputFilePath); // i'm tired, i can't even anymore
		strcat(TotalFilePath, "\\");
		strcat(TotalFilePath, texture[i].TexName);
		strcat(TotalFilePath, ".dds");
		strcpy(texture[i].FilesystemPath, TotalFilePath);
		OutputDDS(TotalFilePath, i, RelativeStart);
		fseek(finput, texture[i].DataSize, SEEK_CUR);
	}
	printf("Extraction finished.\n");
	return 1;
}

int TPKDataChildType1Reader(FILE *finput, unsigned int ChunkSize)
{
	unsigned int TPKLinkCounter = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Data Child 1 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(&TPKLink.Unknown1, 4, 1, finput);
		fread(&TPKLink.Unknown2, 4, 1, finput);
		fread(&TPKLink.NumberOfTPKs, 4, 1, finput);
		fread(&TPKLink.TPKHash[TPKLinkCounter], 4, 1, finput);
		fread(&TPKLink.Unknown3, 4, 1, finput);
		fread(&TPKLink.Unknown4, 4, 1, finput);
		TPKLinkCounter++;
	}
	return 1;
}

int TPKDataChunkReader(FILE *finput, unsigned int ChunkSize)
{
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
			TPKDataChildType1Reader(finput, Size);
			break;
		case TPKDATA_CHILD2_CHUNKID:
			TPKDataChildType2Reader(finput, Size);
			break;
		default:
			printf("Skipping chunk type %X\n", Magic);
			break;
		}
	}
	return 1;
}

int TPKChildType5Reader(FILE *finput, unsigned int ChunkSize)
{
	unsigned int TempBuffer = 0;
	unsigned int TexturePixelFormatCounter = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 5 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fseek(finput, 0xC, SEEK_CUR);
		fread(&GamePixelFormat[TexturePixelFormatCounter].FourCC, 4, 1, finput);
		fread(&GamePixelFormat[TexturePixelFormatCounter].Unknown1, 4, 1, finput);
		fread(&GamePixelFormat[TexturePixelFormatCounter].Unknown2, 4, 1, finput);
		/*fread(&texture[TexturePixelFormatCounter].GamePixelFormat.Unknown3, 4, 1, finput);
		fread(&texture[TexturePixelFormatCounter].GamePixelFormat.Unknown4, 4, 1, finput);
		fread(&texture[TexturePixelFormatCounter].GamePixelFormat.Unknown5, 4, 1, finput);*/
		TexturePixelFormatCounter++;
	}
	return 1;
}

int TPKChildType4Reader(FILE *finput, unsigned int ChunkSize)
{
	unsigned char TexNameSize = 0;
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 4 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fseek(finput, 0xC, SEEK_CUR);
		fread(&texture[TextureDataCount].Hash, 4, 1, finput);
		fread(&texture[TextureDataCount].Hash2, 4, 1, finput);
		fread(&texture[TextureDataCount].DataOffset, 4, 1, finput);
		fseek(finput, 0x4, SEEK_CUR);
		fread(&texture[TextureDataCount].DataSize, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown1, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown2, 4, 1, finput);
		fread(&texture[TextureDataCount].ResX, 2, 1, finput);
		fread(&texture[TextureDataCount].ResY, 2, 1, finput);
		fread(&texture[TextureDataCount].MipmapCount, 1, 1, finput);
		fread(&texture[TextureDataCount].Unknown3, 3, 1, finput);
		fread(&texture[TextureDataCount].TexFlags, 4, 1, finput);

		fread(&texture[TextureDataCount].Unknown4, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown5, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown6, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown7, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown8, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown9, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown10, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown11, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown12, 4, 1, finput);


		fread(&TexNameSize, 1, 1, finput);
		fread(&texture[TextureDataCount].TexName, 1, TexNameSize, finput);
		TextureDataCount++;
	}
	return 1;
}

int TPKChildType3Reader(FILE *finput, unsigned int ChunkSize)
{
	printf("TPK Child 3 size: %X\nSorry, compressed texture data not fully supported yet. Skipping...\nFIXME: add support, add option to extract manually\n", ChunkSize);
	fseek(finput, ChunkSize, SEEK_CUR);
	bCompressed = 1;
	return 1;
}

int TPKChildType2Reader(FILE *finput, unsigned int ChunkSize)
{
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 2 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(&TextureHashArray[TextureHashCount], 4, 1, finput);
		fseek(finput, 4, SEEK_CUR);
		TextureHashCount++;
	}
	printf("Number of texture hashes in array: %d\n", TextureHashCount);
	for (unsigned int i = 0; i <= TextureHashCount - 1; i++)
		printf("Texture %d hash: %X\n", i, TextureHashArray[i]);
	return 1;
}

int TPKChildType1Reader(FILE *finput, unsigned int ChunkSize)
{
	printf("TPK Child 1 size: %X\n", ChunkSize);
	while (ftell(finput) < ChunkSize)
	{
		fseek(finput, 4, SEEK_CUR);
		fread(TPKTypeName, 1, 0x1C, finput);
		printf("TPK Type name: %s\n", TPKTypeName);
		fread(TPKPath, 1, 0x40, finput);
		printf("TPK Path: %s\n", TPKPath);
		fread(HashArray, 4, 7, finput);
	}
	for (unsigned int i = 0; i <= 6; i++)
		printf("TPK hash array [%d] = %X\n", i, HashArray[i]);
	return 1;
}

int TPKChunkReader(FILE *finput, unsigned int ChunkSize)
{
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
			TPKChildType1Reader(finput, Size);
			break;
		case TPK_CHILD2_CHUNKID:
			TPKChildType2Reader(finput, Size);
			break;
		case TPK_CHILD3_CHUNKID:
			TPKChildType3Reader(finput, Size);
			break;
		case TPK_CHILD4_CHUNKID:
			TPKChildType4Reader(finput, Size);
			break;
		case TPK_CHILD5_CHUNKID:
			TPKChildType5Reader(finput, Size);
			break;
		default:
			printf("Skipping chunk type %X\n", Magic);
			break;

		}
	}
	return 1;
}

int MasterChunkReader(FILE *finput)
{
	unsigned int Magic = 0;
	unsigned int Size = 0;
	unsigned int Size2 = 0;
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
			TPKChunkReader(finput, Size2);
			break;
		case TPKDATA_CHUNKID:
			TPKDataChunkReader(finput, Size2);
			break;
		default:
			printf("Skipping chunk type %X\n", Magic);
			fseek(finput, Size2, SEEK_CUR);
			break;
		}
	}
	return 1;
}