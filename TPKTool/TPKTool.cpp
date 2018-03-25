// XNFSTPKTool
// NFS TPK extractor & repacker
// TODO: separate reading/writing functions from the main file

#include "stdafx.h"
#include "DDS.h"
#include "TPKTool.h"
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <ctype.h>

#ifdef TPKTOOL_DECOMPRESSION
#include "includes\injector\injector.hpp"
#include "DecompressionCode.h"
int(__cdecl *Ripped_LZDecompress)(unsigned char* InBuffer, unsigned char* OutBuffer) = (int(__cdecl*)(unsigned char*, unsigned char*))(DecompressionCode + 0x6670);
#endif

char TPKTypeName[0x1C];
char TPKPath[0x40];
bool bCompressed = 0;
bool bDoCompressedStringsOnce = 0;
int WritingMode = 0;
bool bIndexMode = 0;
unsigned int HashArray[7];
unsigned int TextureCategoryHashArray[0xFFFF];
unsigned int TextureCategoryHashCount = 0;
unsigned int TextureDataCount = 0;

unsigned int AnimFrameHashArray[0xFFFF][255];
unsigned int AnimCounter = 0;
unsigned int AnimFrameCounter = 0;

unsigned int TotalSize = 0;
unsigned int TPKCapsuleSize = 0;
unsigned int TPKChunkSize = 0;
unsigned int TPKChunkAlignSize = 0;
unsigned int TPKDataChunkSize = 0;
unsigned int TPKDataChunkAlignSize = 0;

unsigned int TPKChild1Size = 0x7C;
unsigned int TPKChild2Size = 0;
unsigned int TPKChild4Size = 0;
unsigned int TPKChild5Size = 0;
unsigned int TPKExtraCapsuleSize = 0;
unsigned int TPKAnimChunkSize[0xFFFF];
unsigned int TPKAnimChild1Size[0xFFFF];
unsigned int TPKAnimChild2Size[0xFFFF];

unsigned int TPKDataChild1Size = 0x18;
unsigned int TPKDataChild2Size = 0;

unsigned int RelativeDDSDataOffset = 0;

unsigned int DDSMagic = DDS_MAGIC;
void* DDSDataBuffer;

char OutputFilePath[1024]; // the string operations need optimizaton, stat
char TotalFilePath[1124]; // final file path buffer
char IndexBasePath[1124];
char Buffer4[1124];
char StatFileName[32];
char SettingsFileName[32];
unsigned int IndexCounter = 0;
unsigned int IndexToWrite = 0;
char TPKHashAndIndexString[32];
char TPKHashPathString[13];

const char HelpMessage[] = TPKTOOL_HELPMESSAGE;

struct DirectX::DDS_HEADER DDSHeaderStruct;
struct DirectX::DDS_PIXELFORMAT DDSPixelFormatStruct;

GamePixelFormatStruct GamePixelFormat[0xFFFF];
TPKLinkStruct TPKLink;
TPKAnimStruct TPKAnim[0xFFFF];
TexStruct texture[0xFFFF];
Index TPK[0xFFFF];

unsigned int BogusPointer = 0;
bool bCodePatched = 0;

bool bFileExists(const char* filename)
{
	FILE* fin = fopen(filename, "rb");
	if (fin)
	{
		fclose(fin);
		return true;
	}
	perror("ERROR");
	return false;
}

int WriteChunkTypeAndSize(FILE *fout, unsigned int ChunkMagic, unsigned int ChunkSize)
{
	fwrite(&ChunkMagic, 4, 1, fout);
	fwrite(&ChunkSize, 4, 1, fout);
	return 1;
}

int ReadChunkTypeAndSize(FILE *finput, unsigned int &ChunkMagic, unsigned int &ChunkSize)
{
	fread(&ChunkMagic, 4, 1, finput);
	fread(&ChunkSize, 4, 1, finput);
	return 1;
}

int ZeroChunkWriter(FILE *fout, unsigned int ChunkSize)
{
	WriteChunkTypeAndSize(fout, 0, ChunkSize);
	printf("%s Zero chunk: writing %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	for (unsigned int i = 0; i <= ChunkSize - 1; i++)
		fputc(0, fout);
	return 1;
}

int ZeroChunkReader(FILE *finput, unsigned int ChunkSize)
{
	printf("Zero chunk: skipping %X bytes\n", ChunkSize);
	fseek(finput, ChunkSize, SEEK_CUR);
	return 1;
}

bool CheckIfValidSettingsFile(FILE *finput)
{
	unsigned long int oldoffset = ftell(finput);
	char TempBuffer[32];
	fgets(TempBuffer, 32, finput);
	if (strncmp(TempBuffer, "[TPK]\n", 32) == 0)
		return 1;
	else
		return 0;
}

bool bCheckIfVaildFile(const char* FileName)
{
	if (!bFileExists(FileName))
		return false;

	FILE *finput = fopen(FileName, "rb");
	unsigned long int oldoffset = ftell(finput);
	unsigned int Magic = 0;
	unsigned int Size = 0;
	ReadChunkTypeAndSize(finput, Magic, Size);
	fclose(finput);
	if (Magic != TPKCAPSULE_CHUNKID || Size == 0)
	{
		printf("%s Invalid file.\n", PRINTTYPE_ERROR);
		return false;
	}
	return true;
}

int OutputDDSFromMemory(const char* OutFilePath, unsigned int TexNumber, void* DDSDataBuffer)
{
	DDSHeaderStruct.dwSize = 124;
	DDSHeaderStruct.dwFlags = 0x21007;
	DDSHeaderStruct.dwHeight = texture[TexNumber].Child4.ResY;
	DDSHeaderStruct.dwWidth = texture[TexNumber].Child4.ResX;
//	DDSHeaderStruct.dwMipMapCount = texture[TexNumber].Child4.MipmapCount;
	DDSHeaderStruct.dwMipMapCount = 0;
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
	//DDSDataBuffer = malloc(texture[TexNumber].Child4.DataSize);
	//fseek(fin1, texture[TexNumber].Child4.DataOffset + RelativeStart, SEEK_SET);
	//fread(DDSDataBuffer, sizeof(char), texture[TexNumber].Child4.DataSize, fin1);

	FILE *fout = fopen(OutFilePath, "wb");
	if (!fout)
	{ 
		printf("Error opening %s\n", OutFilePath);
		return -1;
	}
	fwrite(&DDSMagic, 4, 1, fout);
	fwrite(&DDSHeaderStruct, sizeof(DDSHeaderStruct), 1, fout);
	fwrite(DDSDataBuffer, sizeof(char), texture[TexNumber].Child4.DataSize, fout);

	fclose(fout);
	return 1;
}

int OutputDDS(FILE *finput, const char* OutFilePath, unsigned int TexNumber, unsigned int RelativeStart)
{
	//unsigned long int PreviousOffset = ftell(finput);
	DDSHeaderStruct.dwSize = 124;
	DDSHeaderStruct.dwFlags = 0x21007;
	DDSHeaderStruct.dwHeight = texture[TexNumber].Child4.ResY;
	DDSHeaderStruct.dwWidth = texture[TexNumber].Child4.ResX;
	//DDSHeaderStruct.dwMipMapCount = texture[TexNumber].Child4.MipmapCount;
	DDSHeaderStruct.dwMipMapCount = 0;
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
	DDSDataBuffer = malloc(texture[TexNumber].Child4.DataSize);
	fseek(finput, texture[TexNumber].Child4.DataOffset + RelativeStart, SEEK_SET);
	fread(DDSDataBuffer, sizeof(char), texture[TexNumber].Child4.DataSize, finput);
	//fseek(finput, PreviousOffset, SEEK_SET);

	FILE *fout = fopen(OutFilePath, "wb");
	fwrite(&DDSMagic, 4, 1, fout);
	fwrite(&DDSHeaderStruct, sizeof(DDSHeaderStruct), 1, fout);
	fwrite(DDSDataBuffer, sizeof(char), texture[TexNumber].Child4.DataSize, fout);

	free(DDSDataBuffer);

	fclose(fout);
	return 1;
}

int WriteDDSDataToFile(const char* InFileName, FILE *fout)
{
	unsigned long int Filesize = 0;
	printf("%s Allocating a buffer and writing %s\n", PRINTTYPE_INFO, InFileName);
	FILE *fin = fopen(InFileName, "rb");
	if (fin == NULL)
	{
		perror("ERROR ");
		return -1;
	}
	fseek(fin, 0, SEEK_END);
	Filesize = ftell(fin) - 0x80;
	fseek(fin, 0x80, SEEK_SET);
	void* InputFileBuffer = malloc(sizeof(char) * Filesize);
	fread(InputFileBuffer, sizeof(char), Filesize, fin);
	fwrite(InputFileBuffer, sizeof(char), Filesize, fout);
	free(InputFileBuffer);
	fclose(fin);
	return 1;
}

int TPKDataChildType2Writer(FILE *fout, unsigned int ChunkSize)
{
	printf("%s Writing TPK data child 2 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPKDATA_CHILD2_CHUNKID, ChunkSize);

	for (unsigned int i = 0; i <= 0x77; i++)
		fputc(0x11, fout);
	for (unsigned int i = 0; i <= TextureCategoryHashCount - 1; i++)
		WriteDDSDataToFile(texture[i].FilesystemPath, fout);
	return 1;
}

int TPKDataChildType2Reader(FILE *finput, unsigned int ChunkSize)
{
	struct stat st = { 0 }; // filestat for folder existence
	char InputFilePath[1024];
	strcpy(InputFilePath, OutputFilePath);
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Data Child 2 size: %X\n", ChunkSize);
	fseek(finput, 0x78, SEEK_CUR);
	unsigned int RelativeStart = ftell(finput);
	sprintf(TPKHashPathString, "%X", HashArray[0]);
	if (bIndexMode)
	{

		strcat(TPKHashPathString, "_");
		strcat(TPKHashPathString, TPKHashAndIndexString);
	}
	strcpy(SettingsFileName, TPKHashPathString);
	strcat(SettingsFileName, ".ini");
	strcpy(StatFileName, TPKHashPathString);
	strcat(StatFileName, "_statistics.txt");

	strcat(InputFilePath, "\\");
	if (!bIndexMode)
		strcat(InputFilePath, TPKHashPathString);
	if (stat(InputFilePath, &st) == -1)
	{
		printf("Making directory %s\n", InputFilePath);
		_mkdir(InputFilePath);
	}
	for (unsigned int i = 0; i <= TextureCategoryHashCount - 1; i++)
	{
		strcpy(TotalFilePath, InputFilePath); // i'm tired, i can't even anymore
		strcat(TotalFilePath, "\\");
		strcat(TotalFilePath, texture[i].TexName);
		strcat(TotalFilePath, ".dds");
		printf("Outputting %s\n", TotalFilePath);
		OutputDDS(finput, TotalFilePath, i, RelativeStart);
		if (bIndexMode)
			strcpy(TotalFilePath, strchr(TotalFilePath, '\\') + 1);
		strcpy(texture[i].FilesystemPath, TotalFilePath);
		fseek(finput, texture[i].Child4.DataSize, SEEK_CUR);
	}
	printf("Extraction finished.\n");
	return 1;
}

int TPKDataChildType1Writer(FILE *fout, unsigned int ChunkSize)  // add this later in the settings file, if it has a point, add support here
{
	printf("%s Writing TPK data child 1 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPKDATA_CHILD1_CHUNKID, ChunkSize);
	TPKLink.Unknown1 = 0;
	TPKLink.Unknown2 = 0;
	TPKLink.NumberOfTPKs = 1;
	TPKLink.TPKHash[0] = HashArray[0];
	TPKLink.Unknown3 = 0;
	TPKLink.Unknown4 = 0;

	fwrite(&TPKLink.Unknown1, 4, 1, fout);
	fwrite(&TPKLink.Unknown2, 4, 1, fout);
	fwrite(&TPKLink.NumberOfTPKs, 4, 1, fout);
	fwrite(&TPKLink.TPKHash[0], 4, 1, fout);
	fwrite(&TPKLink.Unknown3, 4, 1, fout);
	fwrite(&TPKLink.Unknown4, 4, 1, fout);

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

int TPKDataChunkWriter(FILE *fout, unsigned int ChunkSize)
{
	printf("%s Writing TPK data chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPKDATA_CHUNKID, ChunkSize);
	TPKDataChildType1Writer(fout, TPKDataChild1Size);
	ZeroChunkWriter(fout, 0x50);
	TPKDataChildType2Writer(fout, TPKDataChild2Size);
	return 1;
}

int TPKDataChunkReader(FILE *finput, unsigned int ChunkSize)
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
			TPKDataChildType1Reader(finput, Size);
			break;
		case TPKDATA_CHILD2_CHUNKID:
			TPKDataChildType2Reader(finput, Size);
			break;
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size);
			fseek(finput, Size, SEEK_CUR);
			break;
		}
	}
	return 1;
}

int TPKAnimChildType2Writer(FILE *fout, unsigned int ChunkSize, unsigned int AnimToWrite)
{
	printf("%s Writing TPK animation %d child 2 chunk: %X bytes\n", PRINTTYPE_INFO, AnimToWrite, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_ANIMCHILD2_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i <= TPKAnim[AnimToWrite].Frames - 1; i++)
	{
		fwrite(&AnimFrameHashArray[AnimToWrite][i], sizeof(int), 1, fout);
		for (unsigned int j = 0; j < 0x8; j++)
			fputc(0, fout);
	}
	return 1;
}

int TPKAnimChildType2Reader(FILE *finput, unsigned int ChunkSize)
{
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Animation Child 2 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		for (unsigned int i = 0; i <= TPKAnim[AnimFrameCounter].Frames - 1; i++)
		{
			fread(&AnimFrameHashArray[AnimFrameCounter][i], sizeof(int), 1, finput);
			fseek(finput, 0x8, SEEK_CUR);
		}
		AnimFrameCounter++;
	}
	return 1;
}

int TPKAnimChildType1Writer(FILE *fout, unsigned int ChunkSize, unsigned int AnimToWrite)
{
	printf("%s Writing TPK animation %d child 1 chunk: %X bytes\n", PRINTTYPE_INFO, AnimToWrite, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_ANIMCHILD1_CHUNKID, ChunkSize);
	fwrite(&TPKAnim[AnimToWrite], sizeof(TPKAnimStruct), 1, fout);
	return 1;
}

int TPKAnimChildType1Reader(FILE *finput, unsigned int ChunkSize)
{
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Animation Child 1 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(&TPKAnim[AnimCounter], sizeof(TPKAnimStruct), 1, finput);
		AnimCounter++;
	}
	return 1;
}

int TPKAnimChunkWriter(FILE *fout, unsigned int ChunkSize, unsigned int AnimToWrite)
{
	printf("%s Writing TPK animation %d chunk: %X bytes\n", PRINTTYPE_INFO, AnimToWrite, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_ANIM_CHUNKID, ChunkSize);
	TPKAnimChildType1Writer(fout, TPKAnimChild1Size[AnimToWrite], AnimToWrite);
	TPKAnimChildType2Writer(fout, TPKAnimChild2Size[AnimToWrite], AnimToWrite);
	return 1;
}

int TPKAnimChunkReader(FILE *finput, unsigned int ChunkSize)
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
			TPKAnimChildType1Reader(finput, Size);
			break;
		case TPK_ANIMCHILD2_CHUNKID:
			TPKAnimChildType2Reader(finput, Size);
			break;
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size);
			fseek(finput, Size, SEEK_CUR);
			break;
		}
	}
	return 1;
}

int TPKExtraChunkWriter(FILE *fout, unsigned int ChunkSize)
{
	printf("%s Writing TPK extra chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_EXTRACAPSULE_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < AnimCounter; i++)
		TPKAnimChunkWriter(fout, TPKAnimChunkSize[i], i);
	return 1;
}

int TPKExtraChunkReader(FILE *finput, unsigned int ChunkSize)
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
			TPKAnimChunkReader(finput, Size);
			break;
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size);
			fseek(finput, Size, SEEK_CUR);
			break;
		}
	}
	return 1;
}

int TPKChildType5Writer(FILE *fout, unsigned int ChunkSize)
{
	printf("%s Writing TPK child 5 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_CHILD5_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i <= TextureCategoryHashCount - 1; i++)
	{
		for (unsigned int j = 0; j <= 0xB; j++)
			fputc(0, fout);
		fwrite(&GamePixelFormat[i].FourCC, 4, 1, fout);
		fwrite(&GamePixelFormat[i].Unknown1, 4, 1, fout);
		fwrite(&GamePixelFormat[i].Unknown2, 4, 1, fout);
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

int TPKChildType4Writer(FILE *fout, unsigned int ChunkSize)
{
	unsigned char TexNameSize = 0;
	printf("%s Writing TPK child 4 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_CHILD4_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i <= TextureCategoryHashCount - 1; i++)
	{
		TexNameSize = strlen(texture[i].TexName) + 1;
		for (unsigned int j = 0; j <= 0xB; j++)
			fputc(0, fout);
		fwrite(&texture[i].Child4.Hash, sizeof(int), 1, fout);
		fwrite(&texture[i].Child4.Hash2, sizeof(int), 1, fout);
		fwrite(&texture[i].Child4.DataOffset, sizeof(int), 1, fout);
		for (unsigned int j = 0; j <= 0x3; j++)
			fputc(0xFF, fout);
		fwrite(&texture[i].Child4.DataSize, 4, 1, fout);
		fwrite(&texture[i].Child4.Unknown1, 4, 1, fout);
		fwrite(&texture[i].Child4.Scaler, 4, 1, fout);
		fwrite(&texture[i].Child4.ResX, 2, 1, fout);
		fwrite(&texture[i].Child4.ResY, 2, 1, fout);
		fwrite(&texture[i].Child4.MipmapCount, 1, 1, fout);
		//fwrite(&texture[i].MipmapCount, 1, 1, fout);
		//fputc(0x26, fout);
		fwrite(&texture[i].Child4.Unknown3, 2, 1, fout);
		fputc(0x00, fout);
		fwrite(&texture[i].Child4.TexFlags, 4, 1, fout);

		fwrite(&texture[i].Child4.Unknown4, 4, 1, fout);
		fwrite(&texture[i].Child4.Unknown5, 4, 1, fout);
		fwrite(&texture[i].Child4.Unknown6, 4, 1, fout);
		fwrite(&texture[i].Child4.Unknown7, 4, 1, fout);
		fwrite(&texture[i].Child4.Unknown8, 4, 1, fout);
		fwrite(&texture[i].Child4.Unknown9, 4, 1, fout);
		fwrite(&texture[i].Child4.Unknown10, 4, 1, fout);
		fwrite(&texture[i].Child4.Unknown11, 4, 1, fout);
		fwrite(&texture[i].Child4.Unknown12, 4, 1, fout);

		fwrite(&TexNameSize, sizeof(char), 1, fout);
		fwrite(&texture[i].TexName, sizeof(char), TexNameSize, fout);
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
		fread(&texture[TextureDataCount].Child4, sizeof(TPKChild4Struct), 1, finput);
		/*fseek(finput, 0xC, SEEK_CUR);
		fread(&texture[TextureDataCount].Hash, 4, 1, finput);
		fread(&texture[TextureDataCount].Hash2, 4, 1, finput);
		fread(&texture[TextureDataCount].DataOffset, 4, 1, finput);
		fseek(finput, 0x4, SEEK_CUR);
		fread(&texture[TextureDataCount].DataSize, 4, 1, finput);
		fread(&texture[TextureDataCount].Unknown1, 4, 1, finput);
		fread(&texture[TextureDataCount].Scaler, 4, 1, finput);
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
		fread(&texture[TextureDataCount].Unknown12, 4, 1, finput);*/


		fread(&TexNameSize, 1, 1, finput);
		fread(&texture[TextureDataCount].TexName, 1, TexNameSize, finput);
		TextureDataCount++;
	}
	return 1;
}

// Code for compressed data start
#ifdef TPKTOOL_DECOMPRESSION

int CompressedChild4and5Reader(unsigned char* InBuffer, unsigned int TexNumber)
{
	unsigned char TexNameSize = 0;
	memcpy(&texture[TexNumber].Child4, InBuffer, sizeof(TPKChild4Struct));
	memcpy(&TexNameSize, InBuffer + sizeof(TPKChild4Struct), 1);
	memcpy(&texture[TexNumber].TexName, InBuffer + sizeof(TPKChild4Struct) + 1, TexNameSize);
	memcpy(&GamePixelFormat[TexNumber].FourCC, InBuffer + sizeof(TPKChild4Struct) + 0x30, sizeof(int));
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

int TPKChildType3DDSOutputter(void* DDSDataBuffer, unsigned int TexNumber)
{
	struct stat st = { 0 }; // filestat for file/folder existence
	char OutFileName[1024];
	char OutHashFolder[1024];

	if (OutputFilePath[strlen(OutputFilePath) - 1] != '\\') // FIXME: fix all strings in this abomination, 12/2017
		strcat(OutputFilePath, "\\");

	sprintf(TPKHashPathString, "%X", HashArray[0]);

	strcpy(OutHashFolder, OutputFilePath);
	strcat(OutHashFolder, TPKHashPathString);
	strcat(OutHashFolder, "\\");

	if (!bDoCompressedStringsOnce)
	{

		strcpy(SettingsFileName, TPKHashPathString);
		strcat(SettingsFileName, ".ini");
		strcpy(StatFileName, TPKHashPathString);
		strcat(StatFileName, "_statistics.txt");

		if (stat(OutHashFolder, &st) == -1)
		{
			printf("Making directory %s\n", OutHashFolder);
			_mkdir(OutHashFolder);
		}
		bDoCompressedStringsOnce = 1;
	}

	strcpy(OutFileName, OutHashFolder);
	strcat(OutFileName, texture[TexNumber].TexName);
	strcat(OutFileName, ".dds");
	printf("Outputting %s\n", OutFileName);
	
	strcpy(texture[TexNumber].FilesystemPath, OutFileName);

	return OutputDDSFromMemory(OutFileName, TexNumber, DDSDataBuffer);
}

int TPKChildType3Reader(FILE *finput, unsigned int ChunkSize)
{
	//printf("TPK Child 3 size: %X\nSorry, compressed texture data not fully supported yet. Skipping...\nFIXME: add support, add option to extract manually\n", ChunkSize);
	//fseek(finput, ChunkSize, SEEK_CUR);
	printf("TPK Child 3 size: %X\n", ChunkSize);
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	unsigned int SavedOffset = 0;
	unsigned char *InBuffer, *OutBuffer, *InfoBuffer;
	unsigned int NumberOfCompBlocks = 0;
	unsigned int CompBlockLocations[0xFFFF];
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
				CompBlockLocations[NumberOfCompBlocks] = ((unsigned int)InBuffer) + i;
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

		CompressedChild4and5Reader(InfoBuffer, TextureDataCount);
		TPKChildType3DDSOutputter(OutBuffer, TextureDataCount);

		free(CompressFile2);
		free(OutBuffer);
		free(InfoBuffer);
		free(InBuffer);
		TextureDataCount++;
	} while (ftell(finput) < RelativeEnd);

	return 1;
}
#else
int TPKChildType3Reader(FILE *finput, unsigned int ChunkSize)
{
	printf("TPK Child 3 size: %X\nSorry, decompression was disabled.\nTo enable, compile with TPKTOOL_DECOMPRESSION and provide DecompressionCode.h.\nSkipping...\n", ChunkSize);
	fseek(finput, ChunkSize, SEEK_CUR);
	return 1;
}

#endif
// Code for compressed data end

int TPKChildType2Writer(FILE *fout, unsigned int ChunkSize)
{
	printf("%s Writing TPK child 2 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	unsigned int TempZero = 0;
	WriteChunkTypeAndSize(fout, TPK_CHILD2_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i <= TextureCategoryHashCount - 1; i++)
	{
		fwrite(&texture[i].Child4.Hash, sizeof(int), 1, fout);
		fwrite(&TempZero, 4, 1, fout);
	}
	return 1;
}

int TPKChildType2Reader(FILE *finput, unsigned int ChunkSize)
{
	unsigned int RelativeEnd = ftell(finput) + ChunkSize;
	printf("TPK Child 2 size: %X\n", ChunkSize);
	while (ftell(finput) < RelativeEnd)
	{
		fread(&TextureCategoryHashArray[TextureCategoryHashCount], 4, 1, finput);
		fseek(finput, 4, SEEK_CUR);
		TextureCategoryHashCount++;
	}
	printf("Number of texture hashes in array: %d\n", TextureCategoryHashCount);
	for (unsigned int i = 0; i <= TextureCategoryHashCount - 1; i++)
		printf("Texture %d hash: %X\n", i, TextureCategoryHashArray[i]);
	return 1;
}

int TPKChildType1Writer(FILE *fout, unsigned int ChunkSize)
{
	printf("%s Writing TPK child 1 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	unsigned int somethingidunno = 0x8;
	WriteChunkTypeAndSize(fout, TPK_CHILD1_CHUNKID, ChunkSize);
	fwrite(&somethingidunno, 4, 1, fout);
	fwrite(&TPKTypeName, sizeof(char), 0x1C, fout);
	fwrite(&TPKPath, sizeof(char), 0x40, fout);
	fwrite(&HashArray, sizeof(int), 7, fout);
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

int TPKChunkWriter(FILE *fout, unsigned int ChunkSize)
{
	printf("%s Writing TPK chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_CHUNKID, ChunkSize);
	TPKChildType1Writer(fout, TPKChild1Size);
	TPKChildType2Writer(fout, TPKChild2Size);
	TPKChildType4Writer(fout, TPKChild4Size);
	TPKChildType5Writer(fout, TPKChild5Size);
	if (AnimCounter)
		TPKExtraChunkWriter(fout, TPKExtraCapsuleSize);
	return 1;
}

int TPKChunkReader(FILE *finput, unsigned int ChunkSize)
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
		case TPK_EXTRACAPSULE_CHUNKID:
			TPKExtraChunkReader(finput, Size);
			break;
		default:
			printf("Skipping chunk type %X size %X\n", Magic, Size);
			fseek(finput, Size, SEEK_CUR);
			break;

		}
	}
	return 1;
}

int MasterChunkWriter(const char* OutFileName, unsigned int ChunkSize)
{
	FILE *fout = fopen(OutFileName, "wb");
	if (!fout)
		return -1;

	unsigned int RelativeStart = 0;
	WriteChunkTypeAndSize(fout, TPKCAPSULE_CHUNKID, ChunkSize);
	printf("%s Writing the master TPK capsule: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	RelativeStart = ftell(fout);
	ZeroChunkWriter(fout, 0x30);
	TPKChunkWriter(fout, TPKChunkSize);
	ZeroChunkWriter(fout, TPKChunkAlignSize);
	TPKDataChunkWriter(fout, TPKDataChunkSize);
	fclose(fout);
	return 1;
}

int MasterChunkReader(const char* FileName)
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
			TPKChunkReader(finput, Size2);
			break;
		case TPKDATA_CHUNKID:
			TPKDataChunkReader(finput, Size2);
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

int PrecalculateTotalSizes()
{
	unsigned int TPKChunkEndOffset = 0;
	printf("%s Precalculating chunk sizes...\n", PRINTTYPE_INFO);
	TPKDataChunkSize = (TPKDataChild1Size + 8) + (TPKDataChild2Size + 8) + 0x58;
	printf("%s Data chunk child 1 size: %X\n", PRINTTYPE_INFO, TPKDataChild1Size);
	printf("%s Data chunk child 2 size: %X\n", PRINTTYPE_INFO, TPKDataChild2Size);
	printf("%s Total data chunk size: %X\n", PRINTTYPE_INFO, TPKDataChunkSize);
	for (unsigned int i = 0; i <= TextureCategoryHashCount - 1; i++)
	{
		TPKChild5Size += 0x18;
		TPKChild4Size += 0x59 + strlen(texture[i].TexName) + 1;
		TPKChild2Size += 0x8;
	}
	if (AnimCounter)
	{
		for (unsigned int i = 0; i < AnimCounter; i++)
		{
			TPKAnimChild1Size[i] += 0x2C;
			for (unsigned j = 0; j < TPKAnim[i].Frames; j++)
				TPKAnimChild2Size[i] += 0xC;
			printf("%s TPK animation %d chunk child 1 size: %X\n", PRINTTYPE_INFO, i, TPKAnimChild1Size[i]);
			printf("%s TPK animation %d chunk child 2 size: %X\n", PRINTTYPE_INFO, i, TPKAnimChild2Size[i]);
			printf("%s TPK animation %d chunk size: %X\n", PRINTTYPE_INFO, i, TPKAnimChunkSize[i]);
			TPKAnimChunkSize[i] = (TPKAnimChild1Size[i] + 8) + (TPKAnimChild2Size[i] + 8);
			TPKExtraCapsuleSize += (TPKAnimChunkSize[i] + 8);
		}
		TPKChunkSize = (TPKChild1Size + 8) + (TPKChild2Size + 8) + (TPKChild4Size + 8) + (TPKChild5Size + 8) + (TPKExtraCapsuleSize + 8);
		printf("%s TPK extra chunk size: %X\n", PRINTTYPE_INFO, TPKExtraCapsuleSize);
	}
	else
		TPKChunkSize = (TPKChild1Size + 8) + (TPKChild2Size + 8) + (TPKChild4Size + 8) + (TPKChild5Size + 8);
	printf("%s TPK chunk child 1 size: %X\n", PRINTTYPE_INFO, TPKChild1Size);
	printf("%s TPK chunk child 2 size: %X\n", PRINTTYPE_INFO, TPKChild2Size);
	printf("%s TPK chunk child 4 size: %X\n", PRINTTYPE_INFO, TPKChild4Size);
	printf("%s TPK chunk child 5 size: %X\n", PRINTTYPE_INFO, TPKChild5Size);
	printf("%s Total TPK chunk size: %X\n", PRINTTYPE_INFO, TPKChunkSize);

	TPKChunkEndOffset = TPKChunkSize + 0x8 + 0x40;
	TPKChunkAlignSize = (TPKChunkEndOffset - (TPKChunkEndOffset % 0x80)) + 0x100;
	//TPKChunkAlignSize = (TPKChunkEndOffset - (TPKChunkEndOffset & 0xFF)) + 0x100;
	TPKChunkAlignSize -= TPKChunkEndOffset + 8;

	TPKCapsuleSize = (TPKChunkSize + 0x8 + (TPKChunkAlignSize + 0x8)) + (TPKDataChunkSize + 0x8) + 0x38; // TPK chunk has a zero chunk, sized 0x8, totalling in 0x10, TPK capsule has a zero chunk, sized 0x30, totalling 0x38
	printf("%s Total TPK capsule size: %X\n", PRINTTYPE_INFO, TPKCapsuleSize);
	TotalSize = TPKCapsuleSize + 8;
	printf("%s Total file size: %X\n", PRINTTYPE_INFO, TotalSize);
	return 1;
}

int SpitIndexSettingsFile(const char* OutFileName, const char* SettingFileName, unsigned int IndexNumber)
{
	FILE *fout = fopen(OutFileName ,"r");
	if (fout == NULL)
	{
		fout = fopen(OutFileName, "w");
		fprintf(fout, "[TPKIndex]\n");
	}
	else
		freopen(OutFileName, "a+", fout);
	fprintf(fout, "%d = %s\n", IndexNumber, SettingFileName);
	fclose(fout);
	return 1;
}

int SpitSettingsFile(const char* OutFileName)
{
	FILE *fout = fopen(OutFileName, "w");
	fprintf(fout, "[TPK]\n");
	fprintf(fout, "TypeName = %s\n", TPKTypeName);
	fprintf(fout, "Path = %s\n", TPKPath);
	fprintf(fout, "Hash = %X\n", HashArray[0]);
	fprintf(fout, "Animations = %d\n", AnimCounter);

	for (unsigned int i = 0; i < AnimCounter ; i++)
	{
		fprintf(fout, "\n[Anim%d]\n", i);
		fprintf(fout, "Name = %s\n", TPKAnim[i].Name);
		fprintf(fout, "Hash = %X\n", TPKAnim[i].Hash);
		fprintf(fout, "Frames = %d\n", TPKAnim[i].Frames);
		fprintf(fout, "Framerate = %d\n", TPKAnim[i].Framerate);
		fprintf(fout, "Unknown1 = %X\n", TPKAnim[i].Unknown1);
		fprintf(fout, "Unknown2 = %X\n", TPKAnim[i].Unknown2);
		fprintf(fout, "Unknown3 = %X\n", TPKAnim[i].Unknown3);
		fprintf(fout, "Unknown4 = %X\n", TPKAnim[i].Unknown4);
		fprintf(fout, "Unknown5 = %X\n", TPKAnim[i].Unknown5);
		fprintf(fout, "Unknown6 = %X\n", TPKAnim[i].Unknown6);
		for (unsigned int j = 0; j <= TPKAnim[i].Frames - 1; j++)
			fprintf(fout, "Frame%d = %X\n", j, AnimFrameHashArray[i][j]);
	}

	for (unsigned int i = 0; i <= TextureDataCount - 1; i++)
	{
		fprintf(fout, "\n[%X]\n", texture[i].Child4.Hash);
		fprintf(fout, "File = %s\n", texture[i].FilesystemPath);
		fprintf(fout, "Name = %s\n", texture[i].TexName);
		fprintf(fout, "Hash2 = %X\n", texture[i].Child4.Hash2);
		fprintf(fout, "TextureFlags = %X\n", texture[i].Child4.TexFlags);
		fprintf(fout, "Unknown1 = %X\n", texture[i].Child4.Unknown1);
		fprintf(fout, "Scaler = %X\n", texture[i].Child4.Scaler);
		fprintf(fout, "Unknown3 = %X\n", texture[i].Child4.Unknown3);
		fprintf(fout, "Unknown4 = %X\n", texture[i].Child4.Unknown4);
		fprintf(fout, "Unknown5 = %X\n", texture[i].Child4.Unknown5);
		fprintf(fout, "Unknown6 = %X\n", texture[i].Child4.Unknown6);
		fprintf(fout, "Unknown7 = %X\n", texture[i].Child4.Unknown7);
		fprintf(fout, "Unknown8 = %X\n", texture[i].Child4.Unknown8);
		fprintf(fout, "Unknown9 = %X\n", texture[i].Child4.Unknown9);
		fprintf(fout, "Unknown10 = %X\n", texture[i].Child4.Unknown10);
		fprintf(fout, "Unknown11 = %X\n", texture[i].Child4.Unknown11);
		fprintf(fout, "Unknown12 = %X\n", texture[i].Child4.Unknown12);
	}
	fclose(fout);
	return 1;
}

int OutputInfoToFile(const char* OutFileName)
{
	FILE *fout = fopen(OutFileName, "w");;
	fprintf(fout, "TPK info:\n");
	fprintf(fout, "TPK type name: %s\n", TPKTypeName);
	fprintf(fout, "TPK path: %s\n", TPKPath);
	for (unsigned int i = 0; i <= 6; i++)
		fprintf(fout, "TPK hash array [%d] = %#08X\n", i, HashArray[i]);
	for (unsigned int i = 0; i <= TextureCategoryHashCount - 1; i++)
		fprintf(fout, "Texture %d hash: %#08X\n", i, TextureCategoryHashArray[i]);
	fprintf(fout, "Total texture hash count: %d\n", TextureCategoryHashCount);
	fprintf(fout, "\n");
	fprintf(fout, "Textures:\n");
	for (unsigned int i = 0; i <= TextureDataCount - 1; i++)
	{
		fprintf(fout, "\nTexture name: %s\n", texture[i].TexName);
		fprintf(fout, "Hash: %#08X\n", texture[i].Child4.Hash);
		fprintf(fout, "Hash2: %#08X\n", texture[i].Child4.Hash2);
		fprintf(fout, "Data offset: %#08X\n", texture[i].Child4.DataOffset);
		fprintf(fout, "Data size: %#08X\n", texture[i].Child4.DataSize);
		fprintf(fout, "Unknown value 1: %#08X\n", texture[i].Child4.Unknown1);
		fprintf(fout, "Scaler: %#08X\n", texture[i].Child4.Scaler);
		fprintf(fout, "Width: %d\n", texture[i].Child4.ResX);
		fprintf(fout, "Height: %d\n", texture[i].Child4.ResY);
		fprintf(fout, "Mipmap count: %d\n", texture[i].Child4.MipmapCount);
		fprintf(fout, "Unknown value 3: %#08X\n", texture[i].Child4.Unknown3);
		fprintf(fout, "Texture flags: %X\n", texture[i].Child4.TexFlags);
		fprintf(fout, "Unknown value 4: %#08X\n", texture[i].Child4.Unknown4);
		fprintf(fout, "Unknown value 5: %#08X\n", texture[i].Child4.Unknown5);
		fprintf(fout, "Unknown value 6: %#08X\n", texture[i].Child4.Unknown6);
		fprintf(fout, "Unknown value 7: %#08X\n", texture[i].Child4.Unknown7);
		fprintf(fout, "Unknown value 8: %#08X\n", texture[i].Child4.Unknown8);
		fprintf(fout, "Unknown value 9: %#08X\n", texture[i].Child4.Unknown9);
		fprintf(fout, "Unknown value 10: %#08X\n", texture[i].Child4.Unknown10);
		fprintf(fout, "Unknown value 11: %#08X\n", texture[i].Child4.Unknown11);
		fprintf(fout, "Unknown value 12: %#08X\n", texture[i].Child4.Unknown12);
		fprintf(fout, "Pixel format data:\n");
		fprintf(fout, "Pixel format: %#08X\n", GamePixelFormat[i].FourCC);
		fprintf(fout, "Unknown pixel format value 1: %#08X\n", GamePixelFormat[i].Unknown1);
		fprintf(fout, "Unknown pixel format value 2: %#08X\n", GamePixelFormat[i].Unknown2);
		/*fprintf(fout, "Unknown pixel format value 3: %#08X\n", texture[i].GamePixelFormat.Unknown3);
		fprintf(fout, "Unknown pixel format value 4: %#08X\n", texture[i].GamePixelFormat.Unknown4);
		fprintf(fout, "Unknown pixel format value 5: %#08X\n", texture[i].GamePixelFormat.Unknown5);*/
	}
	fprintf(fout, "Total texture data entries: %d\n", TextureDataCount);
	fprintf(fout, "\nAnimations:\n");
	fprintf(fout, "Animation count: %d\n", AnimCounter);
	for (unsigned int i = 0; i < AnimCounter; i++)
	{
		fprintf(fout, "\nAnimation %d:\n", i);
		fprintf(fout, "Name: %s\n", TPKAnim[i].Name);
		fprintf(fout, "Hash: %X\n", TPKAnim[i].Hash);
		fprintf(fout, "Frames: %d\n", TPKAnim[i].Frames);
		fprintf(fout, "Framerate: %d FPS\n", TPKAnim[i].Framerate);
		fprintf(fout, "Unknown 1: %X\n", TPKAnim[i].Unknown1);
		fprintf(fout, "Unknown 2: %X\n", TPKAnim[i].Unknown2);
		fprintf(fout, "Unknown 3: %X\n", TPKAnim[i].Unknown3);
		fprintf(fout, "Unknown 4: %X\n", TPKAnim[i].Unknown4);
		fprintf(fout, "Unknown 5: %X\n", TPKAnim[i].Unknown5);
		fprintf(fout, "Unknown 6: %X\n", TPKAnim[i].Unknown6);
		for (unsigned int j = 0; j <= TPKAnim[i].Frames - 1; j++)
			fprintf(fout, "Frame %d hash: %X\n", j, AnimFrameHashArray[i][j]);
	}
	fclose(fout);
	return 1;
}

bool CheckIfValidDDS(const char* InFileName)
{
	FILE *fin = fopen(InFileName, "rb");
	if (fin == NULL)
	{
		perror("ERROR");
		return 0;
	}
	unsigned int ReadMagic = 0;
	fread(&ReadMagic, 4, 1, fin);
	if (ReadMagic != DDS_MAGIC)
	{
		fclose(fin);
		return 0;
	}
	fread(&DDSHeaderStruct, sizeof(DDSHeaderStruct), 1, fin);
	if (DDSHeaderStruct.dwSize != 124 || DDSHeaderStruct.dwFlags < 0x1000 || DDSHeaderStruct.dwCaps < 0x1000)
	{
		fclose(fin);
		return 0;
	}
	fclose(fin);
	return 1;
}

int ReadDDSData(const char* InFileName, int TexNumber)
{
	FILE *fin = fopen(InFileName, "rb");
	if (fin == NULL)
	{
		perror("ERROR");
		return -1;
	}
	fseek(fin, 4, SEEK_SET);
	DDSHeaderStruct.ddspf = DDSPixelFormatStruct;
	fread(&DDSHeaderStruct, sizeof(DDSHeaderStruct), 1, fin);
	texture[TexNumber].Child4.ResY = DDSHeaderStruct.dwHeight;
	texture[TexNumber].Child4.ResX = DDSHeaderStruct.dwWidth;
	texture[TexNumber].Child4.MipmapCount = DDSHeaderStruct.dwMipMapCount;
	if (DDSHeaderStruct.ddspf.dwFlags >= 0x40)
		GamePixelFormat[TexNumber].FourCC = 0x15;
	else
		GamePixelFormat[TexNumber].FourCC = DDSHeaderStruct.ddspf.dwFourCC;
	fseek(fin, 0L, SEEK_END);
	texture[TexNumber].Child4.DataSize = ftell(fin) - 0x80;
	texture[TexNumber].Child4.DataOffset = RelativeDDSDataOffset;
	RelativeDDSDataOffset = RelativeDDSDataOffset + texture[TexNumber].Child4.DataSize;
	fclose(fin);
	return 1;
}

bool SettingsReader(const char* InFileName)
{
	unsigned int ReadNumber = 0;
	unsigned int ReadNumber2 = 0;
	char TempPathBuffer[255];
	printf("Opening %s\n", InFileName);
	FILE *fin = fopen(InFileName, "r");
	if (fin == NULL)
	{
		perror("ERROR");
		return 0;
	}
	if (!(CheckIfValidSettingsFile(fin)))
	{
		printf("%s Please check your settings ini file format.\n", PRINTTYPE_ERROR);
		fclose(fin);
		return 0;
	}
	fscanf(fin, "[TPK]\n");
	fscanf(fin, "TypeName = %s\n", &TPKTypeName);
	fscanf(fin, "Path = %s\n", &TPKPath);
	fscanf(fin, "Hash = %X\n", &HashArray[0]);
	fscanf(fin, "Animations = %d\n", &AnimCounter);

	for (unsigned int i = 0; i < AnimCounter; i++)
	{
		fscanf(fin, "\n[Anim%d]\n", &ReadNumber);
		fscanf(fin, "Name = %s\n", &TPKAnim[ReadNumber].Name);
		fscanf(fin, "Hash = %X\n", &TPKAnim[ReadNumber].Hash);
		fscanf(fin, "Frames = %hhd\n", &TPKAnim[ReadNumber].Frames);
		fscanf(fin, "Framerate = %hhd\n", &TPKAnim[ReadNumber].Framerate);
		fscanf(fin, "Unknown1 = %X\n", &TPKAnim[ReadNumber].Unknown1);
		fscanf(fin, "Unknown2 = %X\n", &TPKAnim[ReadNumber].Unknown2);
		fscanf(fin, "Unknown3 = %hX\n", &TPKAnim[ReadNumber].Unknown3);
		fscanf(fin, "Unknown4 = %X\n", &TPKAnim[ReadNumber].Unknown4);
		fscanf(fin, "Unknown5 = %X\n", &TPKAnim[ReadNumber].Unknown5);
		fscanf(fin, "Unknown6 = %X\n", &TPKAnim[ReadNumber].Unknown6);
		for (unsigned int j = 0; j <= TPKAnim[ReadNumber].Frames - 1; j++)
			fscanf(fin, "Frame%d = %X\n", &ReadNumber2, &AnimFrameHashArray[ReadNumber][j]);
	}

	while (!feof(fin))
	{
		fscanf(fin, "\n[%X]\n", &texture[TextureCategoryHashCount].Child4.Hash);
		TextureCategoryHashArray[TextureCategoryHashCount] = texture[TextureCategoryHashCount].Child4.Hash;
		//texture[TextureCategoryHashCount].Hash2 = 0x1A93CF;
		if (WritingMode > 1)
		{
			fscanf(fin, "File = %s\n", &TempPathBuffer);
			strcpy(texture[TextureCategoryHashCount].FilesystemPath, IndexBasePath);
			strcat(texture[TextureCategoryHashCount].FilesystemPath, TempPathBuffer);
		}
		else
			fscanf(fin, "File = %s\n", &texture[TextureCategoryHashCount].FilesystemPath);
		if (!(CheckIfValidDDS(texture[TextureCategoryHashCount].FilesystemPath)))
		{
			printf("%s File %s invalid, breaking here.\nMake sure all your DDS files are valid first.\n", PRINTTYPE_ERROR, texture[TextureCategoryHashCount].FilesystemPath);
			return 0;
		}
		ReadDDSData(texture[TextureCategoryHashCount].FilesystemPath, TextureCategoryHashCount);
		fscanf(fin, "Name = %s\n", &texture[TextureCategoryHashCount].TexName);
		fscanf(fin, "Hash2 = %X\n", &texture[TextureCategoryHashCount].Child4.Hash2);
		fscanf(fin, "TextureFlags = %X\n", &texture[TextureCategoryHashCount].Child4.TexFlags);
		fscanf(fin, "Unknown1 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown1);
		fscanf(fin, "Scaler = %X\n", &texture[TextureCategoryHashCount].Child4.Scaler);
		fscanf(fin, "Unknown3 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown3);
		fscanf(fin, "Unknown4 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown4);
		fscanf(fin, "Unknown5 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown5);
		fscanf(fin, "Unknown6 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown6);
		fscanf(fin, "Unknown7 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown7);
		fscanf(fin, "Unknown8 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown8);
		fscanf(fin, "Unknown9 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown9);
		fscanf(fin, "Unknown10 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown10);
		fscanf(fin, "Unknown11 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown11);
		fscanf(fin, "Unknown12 = %X\n", &texture[TextureCategoryHashCount].Child4.Unknown12);
		TextureCategoryHashCount++;
	}
	TextureDataCount = TextureCategoryHashCount; // THIS IS NOT TRUE FOR VINYLS!!!
	TPKDataChild2Size = RelativeDDSDataOffset + 0x78;
	fclose(fin);
	return 1;
}

int ReadIndexCountFromFile(const char* InFileName)
{
	int IndexCount = 0;
	FILE *fin = fopen(InFileName, "r");
	int ReadIndexNumber = 0;
	if (fin == NULL)
	{
		perror("ERROR");
		return 0;
	}
	fscanf(fin, "[TPKIndex]\n");
	while (!feof(fin))
	{
		fscanf(fin, "%d = %s\n");
		IndexCount++;
	}
	fclose(fin);
	return IndexCount;
}

bool ReadIndexPathFromFile(const char* InFileName, char* OutPath, int IndexNumber)
{
	FILE *fin = fopen(InFileName, "r");
	int ReadIndexNumber = 0;
	if (fin == NULL)
	{
		perror("ERROR");
		return 0;
	}
	fscanf(fin, "[TPKIndex]\n");
	while (!feof(fin))
	{
		fscanf(fin, "%d = %s\n", &ReadIndexNumber, OutPath);
		if (ReadIndexNumber == IndexNumber)
			break;
	}
	if (ReadIndexNumber != IndexNumber)
	{
		printf("%s Index number %d not found in file %s\n", PRINTTYPE_ERROR, IndexNumber, InFileName);
		fclose(fin);
		return 0;
	}
	fclose(fin);
	return 1;
}

int WriteToBigFile(int IndexNumber, const char* BigFileName, const char* TPKFilename, const char* OutFileName)
{
	FILE *fbig = fopen(BigFileName, "rb");
	FILE *fin = fopen(TPKFilename, "rb");
	FILE *fout = fopen(OutFileName, "wb");
	int ByteBuffer = 0;
	if (fin == NULL || fbig == NULL || fout == NULL)
	{
		perror("ERROR");
		return -1;
	}

	while (ftell(fbig) < TPK[IndexNumber].TPKOffset)
	{
		ByteBuffer = fgetc(fbig);
		fputc(ByteBuffer, fout);
	}
	while (!feof(fin))
	{
		ByteBuffer = fgetc(fin);
		fputc(ByteBuffer, fout);
	}
	fseek(fbig, TPK[IndexNumber].TPKOffset + TPK[IndexNumber].TPKSize, SEEK_SET);
	while (!feof(fbig))
	{
		ByteBuffer = fgetc(fbig);
		fputc(ByteBuffer, fout);
	}

	//fseek(fbig, TPK[IndexNumber].TPKOffset, SEEK_SET);


	fclose(fbig);
	fclose(fin);
	fclose(fout);
	return 1;
}

int WriteTPKByIndex(int IndexNumber, const char* IndexSettingsFileName)
{
	char ReadSettingsPath[255];
	const char* BasePathPointer;
	TextureCategoryHashCount = 0;
	TextureDataCount = 0;
	AnimCounter = 0;
	AnimFrameCounter = 0;
	if (!(ReadIndexPathFromFile(IndexSettingsFileName, ReadSettingsPath, IndexNumber)))
		return 0;
	BasePathPointer = strrchr(IndexSettingsFileName, '\\');
	strncpy(IndexBasePath, IndexSettingsFileName, BasePathPointer - IndexSettingsFileName + 1);
	strcpy(TotalFilePath, IndexBasePath);
	strcat(TotalFilePath, ReadSettingsPath);
	printf("%s Using settings file %s\n", PRINTTYPE_INFO, TotalFilePath);
	if (!SettingsReader(TotalFilePath))
		return -1;
	strcpy(OutputFilePath, Buffer4);
	if (WritingMode == 3)
	{
		TPKCapsuleSize = 0;
		TPKChunkSize = 0;
		TPKChunkAlignSize = 0;
		TPKDataChunkSize = 0;
		TPKDataChunkAlignSize = 0;
		TPKChild1Size = 0x7C;
		TPKChild2Size = 0;
		TPKChild4Size = 0;
		TPKChild5Size = 0;
		TPKExtraCapsuleSize = 0;
		TPKDataChild1Size = 0x18;
		TPKDataChild2Size = 0;

		memset(TPKAnimChunkSize, 0, 0xFFFF);
		memset(TPKAnimChild1Size, 0, 0xFFFF);
		memset(TPKAnimChild2Size, 0, 0xFFFF);

		strcat(OutputFilePath, "\\");
		strncat(OutputFilePath, ReadSettingsPath, strlen(ReadSettingsPath) - 4);
		strcat(OutputFilePath, ".tpk");
	}
	PrecalculateTotalSizes();
	MasterChunkWriter(OutputFilePath, TPKCapsuleSize);
	return 1;
}

int ExtractTPKIndexFromFile(const char* InFileName, const char* OutFileName, int IndexNumber, bool bUnpackingMode)
{
	FILE *finput = fopen(InFileName, "rb");
	FILE *foutput = NULL;
	char PathBuffer[1024];
	char IndexSettingsFileName[32];
	void* FileBuffer = NULL;
	if (finput == NULL)
	{
		perror("ERROR");
		return -1;
	}
	sprintf(TPKHashAndIndexString, "%X_%d.tpk", TPK[IndexNumber].TPKHash, IndexNumber);
	strcpy(PathBuffer, OutFileName);
	strcat(PathBuffer, "\\");
	strcat(PathBuffer, TPKHashAndIndexString);
	foutput = fopen(PathBuffer, "wb");
	if (foutput == NULL)
	{
		perror("ERROR");
		return -1;
	}
	FileBuffer = malloc(TPK[IndexNumber].TPKSize); // this might not be smart
	fseek(finput, TPK[IndexNumber].TPKOffset, SEEK_SET);
	fread(FileBuffer, TPK[IndexNumber].TPKSize, 1, finput);
	fwrite(FileBuffer, TPK[IndexNumber].TPKSize, 1, foutput);
	free(FileBuffer);
	fclose(foutput);
	if (bUnpackingMode)
	{
		TextureCategoryHashCount = 0;
		TextureDataCount = 0;
		AnimCounter = 0;
		AnimFrameCounter = 0;

		sprintf(TPKHashAndIndexString, "%d", IndexNumber);
		strcpy(OutputFilePath, OutFileName);
		strcat(OutputFilePath, "\\");
		strcat(OutputFilePath, TPKHashAndIndexString);

		MasterChunkReader(PathBuffer);

		strcpy(TotalFilePath, OutputFilePath);
		strcat(TotalFilePath, "\\");
		strcat(TotalFilePath, StatFileName);
		printf("%s Outputting statistics to: %s\n", PRINTTYPE_INFO, TotalFilePath);
		OutputInfoToFile(TotalFilePath);

		strcpy(TotalFilePath, OutputFilePath);
		strcat(TotalFilePath, "\\");
		strcat(TotalFilePath, SettingsFileName);
		printf("%s Outputting settings to: %s\n", PRINTTYPE_INFO, TotalFilePath);
		SpitSettingsFile(TotalFilePath);
		
		sprintf(IndexSettingsFileName, "%X_index.ini", TPK[IndexNumber].TPKHash);
		strcpy(TotalFilePath, PathBuffer);
		strcat(TotalFilePath, "\\");
		strcat(TotalFilePath, IndexSettingsFileName);
		printf("%s Updating index settings: %s\n", PRINTTYPE_INFO, TotalFilePath);
		SpitIndexSettingsFile(TotalFilePath, SettingsFileName, IndexNumber);
	}
	fclose(finput);
	return 1;
}

int CreateTPKIndexFromFile(const char* InFileName)
{
	unsigned int MagicNumber = 0;
	bool bFoundSomething = 0;
	printf("%s Opening %s\n", PRINTTYPE_INFO, InFileName);
	FILE *finput = fopen(InFileName, "rb");
	if (finput == NULL)
	{
		perror("ERROR");
		return -1;
	}
	printf("%s Scanning the file...\n", PRINTTYPE_INFO);
	while (!feof(finput))
	{
		if (!(fread(&MagicNumber, sizeof(int), 1, finput)))
			break;
		if (MagicNumber == TPKCAPSULE_CHUNKID)
		{
			bFoundSomething = 1;
			TPK[IndexCounter].TPKOffset = ftell(finput) - sizeof(int);
			fread(&TPK[IndexCounter].TPKSize, sizeof(int), 1, finput);
			TPK[IndexCounter].TPKSize += 0x8;
			fseek(finput, 0xA8, SEEK_CUR);
			fread(&TPK[IndexCounter].TPKHash, sizeof(int), 1, finput);
			printf("%3d: Hash: %X\tSize: %X\tOffset: %X\n", IndexCounter, TPK[IndexCounter].TPKHash, TPK[IndexCounter].TPKSize, TPK[IndexCounter].TPKOffset);
			IndexCounter++;
		}
	}
	fclose(finput);
	if (!bFoundSomething)
	{
		printf("%s Nothing found.\n", PRINTTYPE_WARNING);
		return 0;
	}
	return 1;
}

int isNumeric(const char * s)
{
	if (s == NULL || *s == '\0' || isspace(*s))
		return 0;
	char * p;
	strtod(s, &p);
	return *p == '\0';
}

int IndexMenu(const char* InFileName)
{
	char Command[16];
	char IndexSettingsFile[1024];
	int Index = 0;
	bool bUnpackingMode = 0;
	if (!CreateTPKIndexFromFile(InFileName))
		return 1;
	printf(TPKTOOL_INDEXMESSAGE);
	
	gets_s(Command, 16);
	if (isNumeric(Command))
	{
		sscanf(Command, "%d", &Index);
		if (Index >= IndexCounter)
		{
			printf("%s Please type in a valid index number\n", PRINTTYPE_ERROR); // add a proper while loop to avoid program restarting
			return -1;
		}
		printf("Please type the output path\n>");
		gets_s(OutputFilePath, 1024);
		printf("Extracting TPK %d\n", Index);
		ExtractTPKIndexFromFile(InFileName, OutputFilePath, Index, bUnpackingMode);
	}
	else if (strncmp(Command, "a", 1) == 0)
	{
		Index = -1;
		printf("Please type the output path\n>");
		gets_s(OutputFilePath, 1024);
		printf("Extracting all TPKs\n");
		ExtractTPKIndexFromFile(InFileName, OutputFilePath, Index, bUnpackingMode);
	}
	else if (strncmp(Command, "u", 3) == 0)
	{
		bUnpackingMode = 1;
		printf("Type a number of TPK to unpack.\n>");
		gets_s(Command, 16);
		if (isNumeric(Command))
		{
			sscanf(Command, "%d", &Index);
			if (Index >= IndexCounter)
			{
				printf("%s Please type in a valid index number\n", PRINTTYPE_ERROR); // add a proper while loop to avoid program restarting
				return -1;
			}
			printf("Please type the output path\n>");
			gets_s(OutputFilePath, 1024);
			printf("Extracting and unpacking TPK %d\n", Index);
			ExtractTPKIndexFromFile(InFileName, OutputFilePath, Index, bUnpackingMode);
		}
		else
		{
			printf("%s Please type in a valid index number\n", PRINTTYPE_ERROR); // add a proper while loop to avoid program restarting
			return -1;
		}
	}
	else if (strncmp(Command, "ua", 2) == 0)
	{
		char SafeOutputPath[1024]; // because I screwed up with OutputFilePath
		bUnpackingMode = 1;
		printf("Please type the output path\n>");
		gets_s(SafeOutputPath, 1024);
		printf("Extracting and unpacking all TPKs\n");
		for (unsigned int i = 0; i < IndexCounter; i++)
			ExtractTPKIndexFromFile(InFileName, SafeOutputPath, i, bUnpackingMode);
	}
	else if (strncmp(Command, "p", 1) == 0)
	{
		printf("Type a number of TPK to pack.\n>");
		gets_s(Command, 16);
		if (isNumeric(Command))
		{
			sscanf(Command, "%d", &Index);
			if (Index >= IndexCounter)
			{
				printf("%s Please type in a valid index number\n", PRINTTYPE_ERROR); // add a proper while loop to avoid program restarting
				return -1;
			}
			printf("Please type the path to the index settings file\n>");
			gets_s(IndexSettingsFile, 1024);
			WritingMode = 2;
			printf("Packing TPK %d\n", Index);
			//strncpy(OutputFilePath, IndexSettingsFile, (strrchr(IndexSettingsFile, '\\') - IndexSettingsFile + 1));
			strncpy(Buffer4, IndexSettingsFile, (strrchr(IndexSettingsFile, '\\') - IndexSettingsFile + 1));
			//strcat(OutputFilePath, "TempPack");
			//mkdir(OutputFilePath);
			//strcpy(Buffer4, OutputFilePath);
			strcat(Buffer4, "\\temp.tpk");
			printf("Writing temporary file to %s\n", Buffer4);
			WriteTPKByIndex(Index, IndexSettingsFile);
			printf("Writing %s to big file\n", Buffer4);
			WriteToBigFile(Index, InFileName, Buffer4, "testbig.bin");
			
		}
		else
		{
			printf("%s Please type in a valid index number\n", PRINTTYPE_ERROR); // add a proper while loop to avoid program restarting
			return -1;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
#ifdef TPKTOOL_WIPVER
	printf("Xanvier's NFS TPK Tool version %d !WIP!\nIf you have this version, you're naughty.\n\n", TPKTOOL_VERSION);
#else
	printf("Xanvier's NFS TPK Tool version %d\n\n", TPKTOOL_VERSION);
#endif
	if (argc <= 2)
	{
		if (argv[1] != NULL)
		{
			if (strncmp(argv[1], "-h", 2) == 0 || strncmp(argv[1], "-?", 2) == 0)
			{
				puts(HelpMessage);
				return 0;
			}
		}
		printf("%s Too few / wrong arguments passed.\nUsage: [-w/-i/-h/-?] InFile OutFile\nFor help pass -h or -?\n", PRINTTYPE_ERROR);
		return -1;
	}

	if (strncmp(argv[1], "-w", 2) == 0)
	{
		printf("%s Going into writing mode!\n", PRINTTYPE_INFO);
		if (strncmp(argv[1] + 2, "i", 1) == 0)
		{
			printf("%s Index writing mode!\n", PRINTTYPE_INFO);
			WritingMode = 2;
			if (strncmp(argv[1] + 3, "a", 1) == 0)
			{
				printf("%s Writing all indexes!\n", PRINTTYPE_INFO);
				WritingMode = 3;
			}
		}
		else
			WritingMode = 1;
	}

	if (strncmp(argv[1], "-i", 2) == 0)
	{
		printf("%s Going into interactive indexing mode!\n", PRINTTYPE_INFO);
		bIndexMode = 1;
	}

	switch (WritingMode)
	{
	case 1:
		strcpy(OutputFilePath, argv[argc - 1]);
		if (!SettingsReader(argv[2]))
			return -1;
		PrecalculateTotalSizes();
		MasterChunkWriter(OutputFilePath, TPKCapsuleSize);
		return 0;
	case 2:
		strcpy(Buffer4, argv[argc - 1]);
		if (isNumeric(argv[3]))
			sscanf(argv[3], "%d", &IndexToWrite);
		else
		{
			printf("%s Please type in a index number\n", PRINTTYPE_ERROR);
			return -1;
		}
		printf("%s Packing index %d\n", PRINTTYPE_INFO, IndexToWrite);
		WriteTPKByIndex(IndexToWrite, argv[2]);
		return 0;
	case 3:
		strcpy(Buffer4, argv[argc - 1]);
		IndexCounter = ReadIndexCountFromFile(argv[2]);
		for (unsigned int i = 0; i < IndexCounter; i++)
			WriteTPKByIndex(i, argv[2]);
		return 0;
	}

	if (bIndexMode)
		return IndexMenu(argv[argc - 1]);
	else
	{
		if (!bFileExists(argv[1]))
			return -1;

		if (!(bCheckIfVaildFile(argv[1])))
			return -1;

		strcpy(OutputFilePath, argv[argc - 1]);
		MasterChunkReader(argv[1]);

		strcpy(TotalFilePath, argv[argc - 1]);
		strcat(TotalFilePath, "\\");
		strcat(TotalFilePath, StatFileName);
		printf("%s Outputting statistics to: %s\n", PRINTTYPE_INFO, TotalFilePath);
		OutputInfoToFile(TotalFilePath);

		strcpy(TotalFilePath, argv[argc - 1]);
		strcat(TotalFilePath, "\\");
		strcat(TotalFilePath, SettingsFileName);
		printf("%s Outputting settings to: %s\n", PRINTTYPE_INFO, TotalFilePath);
		SpitSettingsFile(TotalFilePath);

		return 0;
	}
}
