#pragma once
#include <string.h>
#include <stdlib.h>

#define TPKTOOL_VERSION 2
#define TPKTOOL_WIPVER



#define DDS_MAGIC 0x20534444

#define TPKCAPSULE_CHUNKID 0xB3300000
#define TPK_CHUNKID 0xB3310000
#define TPKDATA_CHUNKID 0xB3320000
#define TPK_CHILD1_CHUNKID 0x33310001
#define TPK_CHILD2_CHUNKID 0x33310002
#define TPK_CHILD3_CHUNKID 0x33310003
#define TPK_CHILD4_CHUNKID 0x33310004
#define TPK_CHILD5_CHUNKID 0x33310005

#define TPK_EXTRACAPSULE_CHUNKID 0xB3312000
#define TPK_ANIM_CHUNKID 0xB3312004
#define TPK_ANIMCHILD1_CHUNKID 0x33312001
#define TPK_ANIMCHILD2_CHUNKID 0x33312002

#define TPKDATA_CHILD1_CHUNKID 0x33320001
#define TPKDATA_CHILD2_CHUNKID 0x33320002
#define TPKDATA_CHILD3_CHUNKID 0x33320003

#define TEXTURENUMCHANNELS 4
#define TPK_TYPENAME_SIZE 0x1C
#define TPK_PATHNAME_SIZE 0x40

#define SIZEOF_TPK_CHILD1 0x7C
#define SIZEOF_TPK_DATA_CHILD1 0x18
#define SIZEOF_TPK_CHILD5 0x18
#define SIZEOF_TPK_CHILD5_V2 0x20
#define SIZEOF_TPK_CHILD4 0x59
#define SIZEOF_TPK_CHILD4_V2 0x7C
#define SIZEOF_TPK_CHILD2 0x8

#define TPKTOOL_READINGMODE_V2 2
#define TPKTOOL_WRITINGMODE_V2 2

// min version is TPKv3 for PS3 anyway, so we don't care about TPKv2 for that platform (unless someone magically ports older NFS games to PS3)
#define TPKTOOL_READINGMODE_PLAT_PS3 3 
#define TPKTOOL_WRITINGMODE_PLAT_PS3 3

// 360 supports TPK minver 5 (TPKv2 as I called it here, version detection will be added at a later point)
#define TPKTOOL_READINGMODE_PLAT_V2_360 4
#define TPKTOOL_WRITINGMODE_PLAT_V2_360 4
#define TPKTOOL_READINGMODE_PLAT_360 5
#define TPKTOOL_WRITINGMODE_PLAT_360 5

#define TPKTOOL_HELPMESSAGE "\
Usage: [-w/-h/-?] InFile [OutFile]\n\n\
Default: InFile = TPK file | OutFile = Output folder path\n\
-2     : TPK v2 mode (UG2 & MW), InFile = TPK file | OutFile = Output folder path\n\
-PS3   : PS3 mode (TPK v3 only), InFile = TPK file | OutFile = Output folder path\n\
-360   : 360 mode (TPK v3), InFile = TPK file | OutFile = Output folder path\n\
-360-2 : 360 mode (TPK v2), InFile = TPK file | OutFile = Output folder path\n\
-w     : Writing mode (TPK v3), InFile = TPK settings ini file | OutFile = Output file name\n\
-w2    : Writing mode (TPK v2), InFile = TPK settings ini file | OutFile = Output file name\n\
-wPS3  : Writing mode (PS3 TPK v3), InFile = TPK settings ini file | OutFile = Output file name\n\
-w360  : Writing mode (360 TPK v3), InFile = TPK settings ini file | OutFile = Output file name\n\
-w360-2: Writing mode (360 TPK v2), InFile = TPK settings ini file | OutFile = Output file name\n\
-h/-?  : Show this help message\n\
[OutFile] optional, if not specified, the folder will be created with the filename's base\n\
\nDDS files are extracted to their respective TPK hash directory.\n\
Compressed TPKs are NOT fully supported yet.\n\
All games except World are currently supported!\n\
While this tool can extract data for other platforms, it only fully works with PC! PS3 & 360 support WIP!"

#define PRINTTYPE_ERROR "ERROR:"
#define PRINTTYPE_INFO "INFO:"
#define PRINTTYPE_WARNING "WARNING:"

int WritingMode = 0;

struct GamePixelFormatStruct // temporary storage for unknown stuff
{
	unsigned int FourCC;
	unsigned int Unknown1;
	unsigned int Unknown2;
	unsigned int Unknown3;
	unsigned int Unknown4;
	unsigned int Unknown5;
};

struct TPKChild5Struct_PS3
{
	unsigned char Unknown1[8];
	unsigned int PixelFormatVal1; // DXT1 = 0, DXT5 = 1, RGBA = 1, DXT3 = unknown, P8 = unknown
	unsigned char SomeVal1; // 2
	unsigned char SomeVal2; // 3
	unsigned short int Unknown2; // 0
	unsigned char PixelFormatVal2; // DXT1 = 3, DXT5 = 3, RGBA = 1, DXT3 = unknown, P8 = unknown
	unsigned char PixelFormatVal3; // DXT1 = 3, DXT5 = 3, RGBA = 0, DXT3 = unknown, P8 = unknown
	unsigned char Unknown3[0x1A]; // all = 0
};

// also used on Carbon it seems...
struct TPKChild5Struct_v2_360
{
	unsigned char Unknown1[8];
	unsigned int PixelFormatVal1; // 1
	unsigned int SomeVal1; // 6 = DXT5
	unsigned int SomeVal2; // 1 = DXT5, ARGB, DXT3, 7 = ARGB or some other RGB?
	unsigned char SomeVal3;
	bool bSwizzled;
	unsigned short SomeVal4;
	//unsigned int SomeVal3; // 0x1A200154 = DXT5, 0x18280186 = RGBA, 0x1A200152 = DXT1, 0x1A200153 = DXT3, maybe not 32 bits, simplified for the sake of testing
	unsigned char Unknown2[8];
};

struct GamePixelFormatStruct_v2 // temporary storage for unknown stuff
{
	unsigned int Unknown1;
	unsigned int Unknown2;
	unsigned int Unknown3; // sometimes 1
	unsigned int Unknown4; // mostly 5
	unsigned int Unknown5; // mostly 6, sometimes 2
	unsigned int FourCC;
	unsigned int Unknown6;
	unsigned int Unknown7;
};

struct TPKLinkStruct
{
	unsigned int NumberOfTPKs;
	unsigned int TPKHash[255]; // can go up to int max but decided against it since this feature is unused
	unsigned int Unknown1;
	unsigned int Unknown2;
	unsigned int Unknown3;
	unsigned int Unknown4;
};

struct TPKAnimStruct
{
	unsigned int Unknown1;
	unsigned int Unknown2;
	char Name[0x10];
	unsigned int Hash;
	unsigned char Frames;
	unsigned char Framerate;
	unsigned short int Unknown3;
	unsigned int Unknown4;
	unsigned int Unknown5;
	unsigned int Unknown6;
};

struct TPKChild3Struct
{
	unsigned int TextureHash;
	unsigned int AbsoluteOffset;
	unsigned int Size;
	unsigned int OutSize;
	unsigned int FromEndToHeaderOffset;
	unsigned int unk;
	//unsigned int Unknown[3];
};

struct CompressBlockHead
{
	unsigned int CompressBlockMagic; // = 0x55441122
	unsigned int OutSize; // =0x8000
	unsigned int TotalBlockSize;
	unsigned int Unknown2; // += OutSize
	unsigned int Unknown3; // += TotalBlockSize
	unsigned int Unknown4;
};

struct TPK_v4_Child4Struct // World
{
	unsigned int Unknown13[3];
	unsigned int Hash;
	unsigned int Hash2;
	unsigned int Hash3;
	unsigned int DataOffset;
	unsigned int Unknown14;
	unsigned int DataSize;
	unsigned int Unknown1;
	unsigned int Scaler;
	unsigned short int ResX;
	unsigned short int ResY;
	unsigned char UnkByteVal1;
	unsigned char UnkByteVal2;
	unsigned short int Unknown3;
	unsigned char Unknown17;
	unsigned char Unknown18;
	unsigned char MipmapCount;
	unsigned char UnkByteVal3;
	unsigned int Unknown4;
	unsigned int Unknown5; // affects color blending modes
	unsigned int Unknown6;
	unsigned int Unknown7;
	unsigned int Unknown8;
	unsigned int Unknown9;
	unsigned int Unknown10;
	unsigned int Unknown11;
	unsigned int Unknown12;
};

struct TPKChild4Struct
{
	unsigned int Unknown13[3];
	unsigned int Hash;
	unsigned int Hash2;
	unsigned int DataOffset;
	unsigned int Unknown14;
	unsigned int DataSize;
	unsigned int Unknown1;
	unsigned int Scaler;
	unsigned short int ResX;
	unsigned short int ResY;
	unsigned char UnkByteVal1;
	unsigned char UnkByteVal2;
	unsigned short int Unknown3;
	unsigned char Unknown17;
	unsigned char Unknown18;
	unsigned char MipmapCount;
	unsigned char UnkByteVal3;
	unsigned int Unknown4;
	unsigned int Unknown5; // affects color blending modes
	unsigned int Unknown6;
	unsigned int Unknown7;
	unsigned int Unknown8;
	unsigned int Unknown9;
	unsigned int Unknown10;
	unsigned int Unknown11;
	unsigned int Unknown12;
};

struct TPKChild4Struct_TPKv2
{
	unsigned int Unknown13[3];
	char TexName[0x18];
	unsigned int Hash;
	unsigned int Hash2;
	unsigned int unk1;
	unsigned int DataOffset;
	unsigned int Unknown14;
	unsigned int DataSize;
	unsigned int Unknown1;
	unsigned int Scaler;
	unsigned short int ResX;
	unsigned short int ResY;
	unsigned char UnkByteVal1;
	unsigned char UnkByteVal2;
	unsigned short int Unknown3;
	unsigned char Unknown17;
	unsigned char Unknown18;
	unsigned char MipmapCount;
	unsigned char UnkByteVal3;
	unsigned int Unknown4;
	unsigned int Unknown5;
	unsigned int Unknown6;
	unsigned int Unknown7;
	unsigned int Unknown8;
	unsigned int Unknown9;
	unsigned int Unknown10; // zeroes from here
	unsigned int Unknown11;
	unsigned int Unknown12;
	unsigned int Unknown15;
	unsigned int Unknown16;
};

struct TexStruct_TPKv2
{
	char TexName[255];
	TPKChild4Struct_TPKv2 Child4;
	char FilesystemPath[255];
};

struct TexStruct
{
	TPKChild4Struct Child4;
	bool bSwizzled;
	char TexName[255];
	char FilesystemPath[255];
};

// a structure to encompass internally used variables to reduce text clutter during argument passing
// MEMORY LEAK! very hacky ATM, needs fixing ASAP
struct TPKToolInternalStruct
{
	unsigned int TPKTypeValue;
	char TPKTypeName[TPK_TYPENAME_SIZE];
	char TPKPathName[TPK_PATHNAME_SIZE];
	char StatFileName[32];
	char SettingsFileName[32];
	char OutputPath[1024];
	char TotalFilePath[1124];
	unsigned int HashArray[7];
	unsigned int TextureCategoryHashArray[0xFFFF];
	unsigned int TextureCategoryHashCount;
	unsigned int TextureDataCount;
	unsigned int AnimFrameHashArray[0xFFFF][255];
	unsigned int AnimFrameCounter;
	unsigned int AnimCounter;
	void* DDSDataBuffer;
	unsigned int TPKDataChunkSize;
	unsigned int TPKDataChunkAlignSize;
	unsigned int TPKDataChild1Size;
	unsigned int TPKDataChild2Size;
	unsigned int RelativeDDSDataOffset;
	unsigned int TPKChild1Size;
	unsigned int TPKChild2Size;
	unsigned int TPKChild4Size;
	unsigned int TPKChild5Size;
	unsigned int TotalSize;
	unsigned int TPKCapsuleSize;
	unsigned int TPKChunkSize;
	unsigned int TPKChunkAlignSize;
	unsigned int TPKExtraCapsuleSize;
	unsigned int TPKAnimChunkSize[0xFFFF];
	unsigned int TPKAnimChild1Size[0xFFFF];
	unsigned int TPKAnimChild2Size[0xFFFF];
};

unsigned int flp2(unsigned int x)
{
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	return x - (x >> 1);
}

void swap(unsigned int *xp, unsigned int *yp)
{
	unsigned int temp = *xp;
	*xp = *yp;
	*yp = temp;
}

// A function to implement bubble sort 
void bubbleSort(unsigned int arr[], unsigned int n)
{
	unsigned int i, j;
	for (i = 0; i < n - 1; i++)

		// Last i elements are already in place    
		for (j = 0; j < n - i - 1; j++)
			if (arr[j] > arr[j + 1])
				swap(&arr[j], &arr[j + 1]);
}

bool bFileExists(const char* filename)
{
	FILE* fin = fopen(filename, "rb");
	if (fin)
	{
		fclose(fin);
		return true;
	}
	//perror("ERROR");
	return false;
}

char* DuplicateFileName(const char* filename)
{
	unsigned int Counter = 1;
	char DissectedPath[260];
	char* fileext = (char*)strrchr(filename, '.');
	do
	{
		strncpy(DissectedPath, filename, fileext - filename);
		DissectedPath[fileext - filename] = '\0';
		sprintf(DissectedPath + strlen(DissectedPath), "(%d)%s", Counter, fileext);
		Counter++;
	} while (bFileExists(DissectedPath));
	return DissectedPath;
}

bool CheckIfValidSettingsFile(FILE *finput)
{
	unsigned long int oldoffset = ftell(finput);
	char TempBuffer[32];
	fgets(TempBuffer, 32, finput);
	if (strncmp(TempBuffer, "[TPK]\n", 7) == 0)
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

	fread(&Magic, 4, 1, finput);
	fread(&Size, 4, 1, finput);

	fclose(finput);
	if (Magic != TPKCAPSULE_CHUNKID || Size == 0)
	{
		printf("%s Invalid file.\n", PRINTTYPE_ERROR);
		return false;
	}
	return true;
}

int SpitSettingsFile(const char* OutFileName, TexStruct *InTexStruct, TPKToolInternalStruct *InTPKToolInternal, TPKAnimStruct *InTPKAnim)
{
	FILE *fout = fopen(OutFileName, "w");

	fprintf(fout, "[TPK]\n");
	fprintf(fout, "TypeName = %s\n", (*InTPKToolInternal).TPKTypeName);
	fprintf(fout, "TypeVal = %d\n", (*InTPKToolInternal).TPKTypeValue);
	fprintf(fout, "Path = %s\n", (*InTPKToolInternal).TPKPathName);
	fprintf(fout, "Hash = %X\n", (*InTPKToolInternal).HashArray[0]);
	fprintf(fout, "Animations = %d\n", (*InTPKToolInternal).AnimCounter);

	for (unsigned int i = 0; i < (*InTPKToolInternal).AnimCounter; i++)
	{
		fprintf(fout, "\n[Anim%d]\n", i);
		fprintf(fout, "Name = %s\n", InTPKAnim[i].Name);
		fprintf(fout, "Hash = %X\n", InTPKAnim[i].Hash);
		fprintf(fout, "Frames = %d\n", InTPKAnim[i].Frames);
		fprintf(fout, "Framerate = %d\n", InTPKAnim[i].Framerate);
		fprintf(fout, "Unknown1 = %X\n", InTPKAnim[i].Unknown1);
		fprintf(fout, "Unknown2 = %X\n", InTPKAnim[i].Unknown2);
		fprintf(fout, "Unknown3 = %X\n", InTPKAnim[i].Unknown3);
		fprintf(fout, "Unknown4 = %X\n", InTPKAnim[i].Unknown4);
		fprintf(fout, "Unknown5 = %X\n", InTPKAnim[i].Unknown5);
		fprintf(fout, "Unknown6 = %X\n", InTPKAnim[i].Unknown6);
		for (unsigned int j = 0; j < InTPKAnim[i].Frames; j++)
			fprintf(fout, "Frame%d = %X\n", j, (*InTPKToolInternal).AnimFrameHashArray[i][j]);
	}

	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureDataCount; i++)
	{
		fprintf(fout, "\n[%X]\n", InTexStruct[i].Child4.Hash);
		fprintf(fout, "File = %s\n", InTexStruct[i].FilesystemPath);
		fprintf(fout, "Name = %s\n", InTexStruct[i].TexName);
		fprintf(fout, "Hash2 = %X\n", InTexStruct[i].Child4.Hash2);
		//fprintf(fout, "TextureFlags = %X\n", InTexStruct[i].Child4.TexFlags);
		fprintf(fout, "UnkByte1 = %X\n", InTexStruct[i].Child4.UnkByteVal1);
		fprintf(fout, "UnkByte2 = %X\n", InTexStruct[i].Child4.UnkByteVal2);
		fprintf(fout, "UnkByte3 = %X\n", InTexStruct[i].Child4.UnkByteVal3);
		fprintf(fout, "Unknown1 = %X\n", InTexStruct[i].Child4.Unknown1);
		fprintf(fout, "Unknown3 = %X\n", InTexStruct[i].Child4.Unknown3);
		fprintf(fout, "Unknown4 = %X\n", InTexStruct[i].Child4.Unknown4);
		fprintf(fout, "Unknown5 = %X\n", InTexStruct[i].Child4.Unknown5);
		fprintf(fout, "Unknown6 = %X\n", InTexStruct[i].Child4.Unknown6);
		fprintf(fout, "Unknown7 = %X\n", InTexStruct[i].Child4.Unknown7);
		fprintf(fout, "Unknown8 = %X\n", InTexStruct[i].Child4.Unknown8);
		fprintf(fout, "Unknown9 = %X\n", InTexStruct[i].Child4.Unknown9);
		fprintf(fout, "Unknown10 = %X\n", InTexStruct[i].Child4.Unknown10);
		fprintf(fout, "Unknown11 = %X\n", InTexStruct[i].Child4.Unknown11);
		fprintf(fout, "Unknown12 = %X\n", InTexStruct[i].Child4.Unknown12);
		fprintf(fout, "Unknown17 = %X\n", InTexStruct[i].Child4.Unknown17);
		fprintf(fout, "Unknown18 = %X\n", InTexStruct[i].Child4.Unknown18);
	}
	fclose(fout);
	return 1;
}

int OutputInfoToFile(const char* OutFileName, TexStruct *InTexStruct, TPKToolInternalStruct *InTPKToolInternal, GamePixelFormatStruct *InGamePixelFormat, TPKAnimStruct *InTPKAnim)
{
	FILE *fout = fopen(OutFileName, "w");;
	fprintf(fout, "TPK info:\n");
	fprintf(fout, "TPK type name: %s\n", (*InTPKToolInternal).TPKTypeName);
	fprintf(fout, "TPK type value: %d\n", (*InTPKToolInternal).TPKTypeValue);
	fprintf(fout, "TPK path: %s\n", (*InTPKToolInternal).TPKPathName);
	for (unsigned int i = 0; i <= 6; i++)
		fprintf(fout, "TPK hash array [%d] = %#08X\n", i, (*InTPKToolInternal).HashArray[i]);
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureCategoryHashCount; i++)
		fprintf(fout, "Texture %d hash: %#08X\n", i, (*InTPKToolInternal).TextureCategoryHashArray[i]);
	fprintf(fout, "Total texture hash count: %d\n", (*InTPKToolInternal).TextureCategoryHashCount);
	fprintf(fout, "\n");
	fprintf(fout, "Textures:\n");
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureDataCount; i++)
	{
		fprintf(fout, "\nTexture name: %s\n", InTexStruct[i].TexName);
		fprintf(fout, "Hash: %#08X\n", InTexStruct[i].Child4.Hash);
		fprintf(fout, "Hash2: %#08X\n", InTexStruct[i].Child4.Hash2);
		fprintf(fout, "Data offset: %#08X\n", InTexStruct[i].Child4.DataOffset);
		fprintf(fout, "Data size: %#08X\n", InTexStruct[i].Child4.DataSize);
		fprintf(fout, "Unknown value 1: %#08X\n", InTexStruct[i].Child4.Unknown1);
		fprintf(fout, "Scaler: %#08X\n", InTexStruct[i].Child4.Scaler);
		fprintf(fout, "Width: %d\n", InTexStruct[i].Child4.ResX);
		fprintf(fout, "Height: %d\n", InTexStruct[i].Child4.ResY);
		fprintf(fout, "Mipmap count: %d\n", InTexStruct[i].Child4.MipmapCount);
		fprintf(fout, "Unknown value 3: %X\n", InTexStruct[i].Child4.Unknown3);
		//fprintf(fout, "Texture flags: %X\n", InTexStruct[i].Child4.TexFlags);
		fprintf(fout, "Unknown value 4: %#08X\n", InTexStruct[i].Child4.Unknown4);
		fprintf(fout, "Blending mode: %#08X\n", InTexStruct[i].Child4.Unknown5);
		fprintf(fout, "Unknown value 6: %#08X\n", InTexStruct[i].Child4.Unknown6);
		fprintf(fout, "Unknown value 7: %#08X\n", InTexStruct[i].Child4.Unknown7);
		fprintf(fout, "Unknown value 8: %#08X\n", InTexStruct[i].Child4.Unknown8);
		fprintf(fout, "Unknown value 9: %#08X\n", InTexStruct[i].Child4.Unknown9);
		fprintf(fout, "Unknown value 10: %#08X\n", InTexStruct[i].Child4.Unknown10);
		fprintf(fout, "Unknown value 11: %#08X\n", InTexStruct[i].Child4.Unknown11);
		fprintf(fout, "Unknown value 12: %#08X\n", InTexStruct[i].Child4.Unknown12);
		fprintf(fout, "Pixel format data:\n");
		fprintf(fout, "Pixel format: %#08X\n", InGamePixelFormat[i].FourCC);
		fprintf(fout, "Unknown pixel format value 1: %#08X\n", InGamePixelFormat[i].Unknown1);
		fprintf(fout, "Unknown pixel format value 2: %#08X\n", InGamePixelFormat[i].Unknown2);
		/*fprintf(fout, "Unknown pixel format value 3: %#08X\n", texture[i].GamePixelFormat.Unknown3);
		fprintf(fout, "Unknown pixel format value 4: %#08X\n", texture[i].GamePixelFormat.Unknown4);
		fprintf(fout, "Unknown pixel format value 5: %#08X\n", texture[i].GamePixelFormat.Unknown5);*/
	}
	fprintf(fout, "Total texture data entries: %d\n", (*InTPKToolInternal).TextureDataCount);
	fprintf(fout, "\nAnimations:\n");
	fprintf(fout, "Animation count: %d\n", (*InTPKToolInternal).AnimCounter);
	for (unsigned int i = 0; i < (*InTPKToolInternal).AnimCounter; i++)
	{
		fprintf(fout, "\nAnimation %d:\n", i);
		fprintf(fout, "Name: %s\n", InTPKAnim[i].Name);
		fprintf(fout, "Hash: %X\n", InTPKAnim[i].Hash);
		fprintf(fout, "Frames: %d\n", InTPKAnim[i].Frames);
		fprintf(fout, "Framerate: %d FPS\n", InTPKAnim[i].Framerate);
		fprintf(fout, "Unknown 1: %X\n", InTPKAnim[i].Unknown1);
		fprintf(fout, "Unknown 2: %X\n", InTPKAnim[i].Unknown2);
		fprintf(fout, "Unknown 3: %X\n", InTPKAnim[i].Unknown3);
		fprintf(fout, "Unknown 4: %X\n", InTPKAnim[i].Unknown4);
		fprintf(fout, "Unknown 5: %X\n", InTPKAnim[i].Unknown5);
		fprintf(fout, "Unknown 6: %X\n", InTPKAnim[i].Unknown6);
		for (unsigned int j = 0; j < InTPKAnim[i].Frames; j++)
			fprintf(fout, "Frame %d hash: %X\n", j, (*InTPKToolInternal).AnimFrameHashArray[i][j]);
	}
	fclose(fout);
	return 1;
}


int PrecalculateTotalSizes(TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct, TPKAnimStruct *InTPKAnim)
{
	(*InTPKToolInternal).TPKChild1Size = SIZEOF_TPK_CHILD1;
	(*InTPKToolInternal).TPKDataChild1Size = SIZEOF_TPK_DATA_CHILD1;
	unsigned int TPKChunkEndOffset = 0;
	printf("%s Precalculating chunk sizes...\n", PRINTTYPE_INFO);
	(*InTPKToolInternal).TPKDataChunkSize = ((*InTPKToolInternal).TPKDataChild1Size + 8) + ((*InTPKToolInternal).TPKDataChild2Size + 8) + 0x58;
	printf("%s Data chunk child 1 size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKDataChild1Size);
	printf("%s Data chunk child 2 size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKDataChild2Size);
	printf("%s Total data chunk size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKDataChunkSize);
	for (unsigned int i = 0; i <= (*InTPKToolInternal).TextureCategoryHashCount - 1; i++)
	{
		switch (WritingMode)
		{
		case TPKTOOL_WRITINGMODE_V2:
			(*InTPKToolInternal).TPKChild5Size += sizeof(GamePixelFormatStruct_v2);
			(*InTPKToolInternal).TPKChild4Size += sizeof(TPKChild4Struct_TPKv2);
			break;
		case TPKTOOL_WRITINGMODE_PLAT_V2_360:
			(*InTPKToolInternal).TPKChild5Size += sizeof(TPKChild5Struct_v2_360);
			(*InTPKToolInternal).TPKChild4Size += sizeof(TPKChild4Struct_TPKv2);
			break;
		case TPKTOOL_WRITINGMODE_PLAT_PS3:
			(*InTPKToolInternal).TPKChild5Size += sizeof(TPKChild5Struct_PS3);
			(*InTPKToolInternal).TPKChild4Size += sizeof(TPKChild4Struct) + strlen(InTexStruct[i].TexName) + 1;
			break;
		case TPKTOOL_WRITINGMODE_PLAT_360: // TODO
			(*InTPKToolInternal).TPKChild5Size += sizeof(GamePixelFormatStruct);
			(*InTPKToolInternal).TPKChild4Size += sizeof(TPKChild4Struct) + strlen(InTexStruct[i].TexName) + 1;
			break;
		default:
			(*InTPKToolInternal).TPKChild5Size += sizeof(GamePixelFormatStruct);
			(*InTPKToolInternal).TPKChild4Size += sizeof(TPKChild4Struct) + strlen(InTexStruct[i].TexName) + 1;
			break;
		}

		(*InTPKToolInternal).TPKChild2Size += SIZEOF_TPK_CHILD2;
	}
	if ((*InTPKToolInternal).AnimCounter)
	{
		for (unsigned int i = 0; i < (*InTPKToolInternal).AnimCounter; i++)
		{
			(*InTPKToolInternal).TPKAnimChild1Size[i] += 0x2C;
			for (unsigned j = 0; j < InTPKAnim[i].Frames; j++)
				(*InTPKToolInternal).TPKAnimChild2Size[i] += 0xC;
			printf("%s TPK animation %d chunk child 1 size: %X\n", PRINTTYPE_INFO, i, (*InTPKToolInternal).TPKAnimChild1Size[i]);
			printf("%s TPK animation %d chunk child 2 size: %X\n", PRINTTYPE_INFO, i, (*InTPKToolInternal).TPKAnimChild2Size[i]);
			printf("%s TPK animation %d chunk size: %X\n", PRINTTYPE_INFO, i, (*InTPKToolInternal).TPKAnimChunkSize[i]);
			(*InTPKToolInternal).TPKAnimChunkSize[i] = ((*InTPKToolInternal).TPKAnimChild1Size[i] + 8) + ((*InTPKToolInternal).TPKAnimChild2Size[i] + 8);
			(*InTPKToolInternal).TPKExtraCapsuleSize += ((*InTPKToolInternal).TPKAnimChunkSize[i] + 8);
		}
		(*InTPKToolInternal).TPKChunkSize = ((*InTPKToolInternal).TPKChild1Size + 8) + ((*InTPKToolInternal).TPKChild2Size + 8) + ((*InTPKToolInternal).TPKChild4Size + 8) + ((*InTPKToolInternal).TPKChild5Size + 8) + ((*InTPKToolInternal).TPKExtraCapsuleSize + 8);
		printf("%s TPK extra chunk size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKExtraCapsuleSize);
	}
	else
		(*InTPKToolInternal).TPKChunkSize = ((*InTPKToolInternal).TPKChild1Size + 8) + ((*InTPKToolInternal).TPKChild2Size + 8) + ((*InTPKToolInternal).TPKChild4Size + 8) + ((*InTPKToolInternal).TPKChild5Size + 8);
	printf("%s TPK chunk child 1 size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKChild1Size);
	printf("%s TPK chunk child 2 size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKChild2Size);
	printf("%s TPK chunk child 4 size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKChild4Size);
	printf("%s TPK chunk child 5 size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKChild5Size);
	printf("%s Total TPK chunk size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKChunkSize);

	TPKChunkEndOffset = (*InTPKToolInternal).TPKChunkSize + 0x8 + 0x40;
	(*InTPKToolInternal).TPKChunkAlignSize = (TPKChunkEndOffset - (TPKChunkEndOffset % 0x80)) + 0x100;
	//TPKChunkAlignSize = (TPKChunkEndOffset - (TPKChunkEndOffset & 0xFF)) + 0x100;
	(*InTPKToolInternal).TPKChunkAlignSize -= TPKChunkEndOffset + 8;

	(*InTPKToolInternal).TPKCapsuleSize = ((*InTPKToolInternal).TPKChunkSize + 0x8 + ((*InTPKToolInternal).TPKChunkAlignSize + 0x8)) + ((*InTPKToolInternal).TPKDataChunkSize + 0x8) + 0x38; // TPK chunk has a zero chunk, sized 0x8, totalling in 0x10, TPK capsule has a zero chunk, sized 0x30, totalling 0x38
	printf("%s Total TPK capsule size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKCapsuleSize);
	(*InTPKToolInternal).TotalSize = (*InTPKToolInternal).TPKCapsuleSize + 8;
	printf("%s Total file size: %X\n", PRINTTYPE_INFO, (*InTPKToolInternal).TotalSize);
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
	fseek(fin, 0, SEEK_END); // WRONG WRONG WRONG WRONG WRONG WRONG WRONG WRONG, rewrite to filesystem stat!
	Filesize = ftell(fin) - 0x80;
	fseek(fin, 0x80, SEEK_SET);
	void* InputFileBuffer = malloc(sizeof(char) * Filesize);
	fread(InputFileBuffer, sizeof(char), Filesize, fin);
	fwrite(InputFileBuffer, sizeof(char), Filesize, fout);
	free(InputFileBuffer);
	fclose(fin);
	return 1;
}

int ByteSwapBuffer_Short(void* buffer, unsigned int size)
{
	unsigned int CurrentPos = (unsigned int)buffer;
	unsigned short CurrentValue;

	while (CurrentPos < ((unsigned int)buffer + size))
	{
		CurrentValue = _byteswap_ushort(*(unsigned short*)(CurrentPos));
		*(unsigned short*)(CurrentPos) = CurrentValue;
		CurrentPos += sizeof(short);
	}

	return 0;
}

// 360 deswizzling stuff
// source from: https://github.com/emoose/FtexTool (thanks!)
// converted to C / C++ by me
// will add swizzlers later on...
int Align(int ptr, int alignment)
{
	return ((ptr + alignment - 1) & ~(alignment - 1));
}

int appLog2(int n)
{
	int r;
	int n2 = n;
	for (r = -1; n2 != 0; n2 >>= 1, r++)
	{ /*empty*/
	}
	return r;
}

int GetTiledOffset(int x, int y, int width, int logBpb)
{
	int alignedWidth = Align(width, 32);
	// top bits of coordinates
	int macro = ((x >> 5) + (y >> 5) * (alignedWidth >> 5)) << (logBpb + 7);
	// lower bits of coordinates (result is 6-bit value)
	int micro = ((x & 7) + ((y & 0xE) << 2)) << logBpb;
	// mix micro/macro + add few remaining x/y bits
	int offset = macro + ((micro & ~0xF) << 1) + (micro & 0xF) + ((y & 1) << 4);
	// mix bits again
	return (((offset & ~0x1FF) << 3) +					// upper bits (offset bits [*-9])
		((y & 16) << 7) +							// next 1 bit
		((offset & 0x1C0) << 2) +					// next 3 bits (offset bits [8-6])
		(((((y & 8) >> 2) + (x >> 3)) & 3) << 6) +	// next 2 bits
		(offset & 0x3F)								// lower 6 bits (offset bits [5-0])
		) >> logBpb;
}

void* UntileCompressedX360Texture(void* data, unsigned int datasize, int tiledWidth, int originalWidth, int height, int blockSizeX, int blockSizeY, int bytesPerBlock)
{
	void* OutputBuffer = malloc(datasize);

	int blockWidth = tiledWidth / blockSizeX;
	int originalBlockWidth = originalWidth / blockSizeX;
	int blockHeight = height / blockSizeY;
	int logBpp = appLog2(bytesPerBlock);

	for (int y = 0; y < blockHeight; y++)
	{
		for (int x = 0; x < originalBlockWidth; x++)
		{
			int addr = GetTiledOffset(x, y, blockWidth, logBpp);

			int sy = addr / blockWidth;
			int sx = addr % blockWidth;

			int dstAddr = (y * originalBlockWidth + x) * bytesPerBlock;
			int srcAddr = (sy * blockWidth + sx) * bytesPerBlock;

			memcpy((void*)((int)OutputBuffer + dstAddr), (void*)((int)data + srcAddr), bytesPerBlock);
		}
	}
	return OutputBuffer;
}

void* Deswizzle(void* data, int width, int height, int numMipMaps, unsigned int FourCC)
{
	bool found = false;

	int BlockSizeX;
	int BlockSizeY;
	int bytesPerBlock;
	int X360AlignX;
	int X360AlignY;


	switch (FourCC)
	{
	case 0x31545844:
		BlockSizeX = 4;
		BlockSizeY = 4;
		X360AlignX = 128;
		X360AlignY = 128;
		bytesPerBlock = 8;
		found = true;
		break;
	case 0x33545844:
	case 0x35545844:
		BlockSizeX = 4;
		BlockSizeY = 4;
		X360AlignX = 128;
		X360AlignY = 128;
		bytesPerBlock = 16;
		found = true;
		break;
	default:
		// using RGBA8 here...
		BlockSizeX = 1;
		BlockSizeY = 1;
		X360AlignX = 32;
		X360AlignY = 32;
		bytesPerBlock = 4;
		break;
	}

	int curAddr = 0;
	for (int i = 0; i < numMipMaps; i++)
	{
		int width1 = Align(width, X360AlignX);
		int height1 = Align(height, X360AlignY);

		int size = (width1 / BlockSizeX) * (height1 / BlockSizeY) * bytesPerBlock;

		void* mipMapData = malloc(size);
		memcpy(mipMapData, (void*)((unsigned int)data + curAddr), size);
		//Array.Copy(data, curAddr, mipMapData, 0, size);
		mipMapData = UntileCompressedX360Texture(mipMapData, size, width1, width, height1, BlockSizeX, BlockSizeY, bytesPerBlock);
		memcpy((void*)((unsigned int)data + curAddr), mipMapData, size);
		//Array.Copy(mipMapData, 0, data, curAddr, size);

		curAddr += size;
		width /= 2;
		height /= 2;
	}

	return data;
}

int Deswizzle_RecalculateSize(int width, int height, unsigned int FourCC)
{

	int BlockSizeX;
	int BlockSizeY;
	int bytesPerBlock;
	int X360AlignX;
	int X360AlignY;

	switch (FourCC)
	{
	case 0x31545844:
		BlockSizeX = 4;
		BlockSizeY = 4;
		X360AlignX = 128;
		X360AlignY = 128;
		bytesPerBlock = 8;
		break;
	case 0x33545844:
	case 0x35545844:
		BlockSizeX = 4;
		BlockSizeY = 4;
		X360AlignX = 128;
		X360AlignY = 128;
		bytesPerBlock = 16;
		break;
	default:
		// using RGBA8 here...
		BlockSizeX = 1;
		BlockSizeY = 1;
		X360AlignX = 32;
		X360AlignY = 32;
		bytesPerBlock = 4;
		break;
	}
	int size = (width / BlockSizeX) * (height / BlockSizeY) * bytesPerBlock;

	return size;
}