// XNFSTPKTool
// NFS TPK extractor & repacker
// TODO: fix 64MB memory leak
// TODO: finish TPK v2 support
// 08/2018 - v2: scrapped indexing mode, too ambicious for a simple commandline tool, will rework into a separate library so another tool can do it

#include "stdafx.h"
#include "DDS.h"
#include "TPKTool.h"
#include "TPKTool_DDSReading.h"
#include "TPKTool_ChunkReading.h"
#include "TPKTool_ChunkWriting.h"
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <ctype.h>

GamePixelFormatStruct GamePixelFormat[0xFFFF];
TPKLinkStruct TPKLink;
TPKAnimStruct TPKAnim[0xFFFF];
TexStruct texture[0xFFFF];

// this will need cleanup due to stupid structs I put
void SortTexturesByHash(TPKToolInternalStruct *InTPKInternal, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormat)
{
	GamePixelFormatStruct *GamePixelFormat_sorted;
	TexStruct *texture_sorted;
	unsigned int j = 0;

	printf("%s Sorting textures by their hash...\n", PRINTTYPE_INFO);

	// allocating memory for a sorted copy
	GamePixelFormat_sorted = (GamePixelFormatStruct*)calloc((*InTPKInternal).TextureCategoryHashCount, sizeof(GamePixelFormatStruct));
	texture_sorted = (TexStruct*)calloc((*InTPKInternal).TextureCategoryHashCount, sizeof(TexStruct));

	// using the hash array as a reference for sorting
	bubbleSort((*InTPKInternal).TextureCategoryHashArray, (*InTPKInternal).TextureCategoryHashCount);

	for (unsigned int i = 0; i < (*InTPKInternal).TextureCategoryHashCount; i++)
	{
		// first we search for the texture
		for (j = 0; j < (*InTPKInternal).TextureCategoryHashCount; j++)
		{
			if (InTexStruct[j].Child4.Hash == (*InTPKInternal).TextureCategoryHashArray[i])
				break;
		}
		// then we use its index to sort things out (copy to the temp buffer)
		texture_sorted[i] = InTexStruct[j];
		GamePixelFormat_sorted[i] = InGamePixelFormat[j];
	}

	// once we're done, we copy stuff back to the memory
	memcpy(InTexStruct, texture_sorted, sizeof(TexStruct)*(*InTPKInternal).TextureCategoryHashCount);
	memcpy(InGamePixelFormat, GamePixelFormat_sorted, sizeof(GamePixelFormatStruct)*(*InTPKInternal).TextureCategoryHashCount);

	// then free the memory
	free(texture_sorted);
	free(GamePixelFormat_sorted);
}

bool SettingsReader(const char* InFileName, TPKToolInternalStruct *InTPKInternal, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormat, TPKAnimStruct *InTPKAnim)
{
	unsigned int ReadNumber = 0;
	unsigned int ReadNumber2 = 0;
//	char TempPathBuffer[255];
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
	fscanf(fin, "TypeName = %s\n", &(*InTPKInternal).TPKTypeName);
	fscanf(fin, "TypeVal = %d\n", &(*InTPKInternal).TPKTypeValue);
	fscanf(fin, "Path = %s\n", &(*InTPKInternal).TPKPathName);
	fscanf(fin, "Hash = %X\n", &(*InTPKInternal).HashArray[0]);
	fscanf(fin, "Animations = %d\n", &(*InTPKInternal).AnimCounter);

	for (unsigned int i = 0; i < (*InTPKInternal).AnimCounter; i++)
	{
		fscanf(fin, "\n[Anim%d]\n", &ReadNumber);
		fscanf(fin, "Name = %s\n", &InTPKAnim[ReadNumber].Name);
		fscanf(fin, "Hash = %X\n", &InTPKAnim[ReadNumber].Hash);
		fscanf(fin, "Frames = %hhd\n", &InTPKAnim[ReadNumber].Frames);
		fscanf(fin, "Framerate = %hhd\n", &InTPKAnim[ReadNumber].Framerate);
		fscanf(fin, "Unknown1 = %X\n", &InTPKAnim[ReadNumber].Unknown1);
		fscanf(fin, "Unknown2 = %X\n", &InTPKAnim[ReadNumber].Unknown2);
		fscanf(fin, "Unknown3 = %hX\n", &InTPKAnim[ReadNumber].Unknown3);
		fscanf(fin, "Unknown4 = %X\n", &InTPKAnim[ReadNumber].Unknown4);
		fscanf(fin, "Unknown5 = %X\n", &InTPKAnim[ReadNumber].Unknown5);
		fscanf(fin, "Unknown6 = %X\n", &InTPKAnim[ReadNumber].Unknown6);
		for (unsigned int j = 0; j < InTPKAnim[ReadNumber].Frames; j++)
			fscanf(fin, "Frame%d = %X\n", &ReadNumber2, &(*InTPKInternal).AnimFrameHashArray[ReadNumber][j]);
	}

	while (!feof(fin))
	{
		fscanf(fin, "\n[%X]\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Hash);
		(*InTPKInternal).TextureCategoryHashArray[(*InTPKInternal).TextureCategoryHashCount] = InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Hash;
		//texture[TextureCategoryHashCount].Hash2 = 0x1A93CF;
		fscanf(fin, "File = %s\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].FilesystemPath);
		if (!(CheckIfValidDDS(InTexStruct[(*InTPKInternal).TextureCategoryHashCount].FilesystemPath)))
		{
			printf("%s File %s invalid, breaking here.\nMake sure all your DDS files are valid first.\n", PRINTTYPE_ERROR, InTexStruct[(*InTPKInternal).TextureCategoryHashCount].FilesystemPath);
			return 0;
		}
//		ReadDDSData(InTexStruct, InGamePixelFormat, &(*InTPKInternal).RelativeDDSDataOffset, (*InTPKInternal).TextureCategoryHashCount);
		fscanf(fin, "Name = %s\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].TexName);
		fscanf(fin, "Hash2 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Hash2);
		//fscanf(fin, "TextureFlags = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.TexFlags);
		fscanf(fin, "UnkByte1 = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.UnkByteVal1);
		fscanf(fin, "UnkByte2 = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.UnkByteVal2);
		fscanf(fin, "UnkByte3 = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.UnkByteVal3);
		fscanf(fin, "Unknown1 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown1);
		//fscanf(fin, "Scaler = %X\n", &texture[TextureCategoryHashCount].Child4.Scaler);

//		if (InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].FourCC == 0x15)
//			InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Scaler = ((InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ResX) * (InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ResY)) * TEXTURENUMCHANNELS;
//		else
//			InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Scaler = flp2(InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.DataSize);

		fscanf(fin, "Unknown3 = %hX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown3);
		fscanf(fin, "Unknown4 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown4);
		fscanf(fin, "Unknown5 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown5);
		fscanf(fin, "Unknown6 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown6);
		fscanf(fin, "Unknown7 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown7);
		fscanf(fin, "Unknown8 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown8);
		fscanf(fin, "Unknown9 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown9);
		

		// REPURPOSING CHILD4's VALUES! IF CARBON IS USED, THESE ARE ZEROED OUT! HACK!
		if (WritingMode == TPKTOOL_WRITINGMODE_V2)
		{
			fscanf(fin, "Unknown10 = %X\n", &InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown1);
			fscanf(fin, "Unknown11 = %X\n", &InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown2);
			fscanf(fin, "Unknown12 = %X\n", &InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown3);
		}
		else
		{
			fscanf(fin, "Unknown10 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown10);
			fscanf(fin, "Unknown11 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown11);
			fscanf(fin, "Unknown12 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown12);
		}
		fscanf(fin, "Unknown17 = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown17);
		fscanf(fin, "Unknown18 = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown18);

		(*InTPKInternal).TextureCategoryHashCount++;
	}

	SortTexturesByHash(InTPKInternal, InTexStruct, InGamePixelFormat);

	for (unsigned int i = 0; i < (*InTPKInternal).TextureCategoryHashCount; i++)
	{
		ReadDDSData(InTexStruct, InGamePixelFormat, &(*InTPKInternal).RelativeDDSDataOffset, i);

		if (InGamePixelFormat[i].FourCC == 0x15)
			InTexStruct[i].Child4.Scaler = ((InTexStruct[i].Child4.ResX) * (InTexStruct[i].Child4.ResY)) * TEXTURENUMCHANNELS;
		else
			InTexStruct[i].Child4.Scaler = flp2(InTexStruct[i].Child4.DataSize);
	}

	(*InTPKInternal).TextureDataCount = (*InTPKInternal).TextureCategoryHashCount; // THIS IS NOT TRUE FOR VINYLS!!!
	(*InTPKInternal).TPKDataChild2Size = (*InTPKInternal).RelativeDDSDataOffset + 0x78;

	fclose(fin);
	return 1;
}

bool bDoFileChecks(const char* Filename)
{
	if (!bFileExists(Filename))
		return false;

	if (!(bCheckIfVaildFile(Filename)))
		return false;
	return true;
}

int main(int argc, char *argv[])
{
	TPKToolInternalStruct *TPKToolStuff = (TPKToolInternalStruct*)calloc(1, sizeof(TPKToolInternalStruct));

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
				puts(TPKTOOL_HELPMESSAGE);
				return 0;
			}
		}
		printf("%s Too few / wrong arguments passed.\nUsage: [-w/-h/-?] InFile OutFile\nFor help pass -h or -?\n", PRINTTYPE_ERROR);
		return -1;
	}

	if (strncmp(argv[1], "-w", 2) == 0)
	{
		printf("%s Going into writing mode!\n", PRINTTYPE_INFO);
		WritingMode = 1;
		if (strncmp(argv[1] + 2, "2", 1) == 0)
		{
			printf("%s Going into TPK version 2 mode!\n", PRINTTYPE_INFO);
			WritingMode = TPKTOOL_WRITINGMODE_V2;
		}
		else if (strncmp(argv[1] + 2, "PS3", 3) == 0)
		{
			printf("%s Going into PS3 mode!\n", PRINTTYPE_INFO);
			WritingMode = TPKTOOL_WRITINGMODE_PLAT_PS3;
		}
	}

	if (strncmp(argv[1], "-2", 2) == 0)
	{
		printf("%s Going into TPK version 2 mode!\n", PRINTTYPE_INFO);
		ReadingMode = TPKTOOL_READINGMODE_V2;
	}

	else if (strncmp(argv[1], "-PS3", 4) == 0)
	{
		printf("%s Going into PS3 mode!\n", PRINTTYPE_INFO);
		ReadingMode = TPKTOOL_READINGMODE_PLAT_PS3;
	}

	if (WritingMode)
	{
		//strcpy(OutputFilePath, argv[argc - 1]);
		if (!SettingsReader(argv[2], TPKToolStuff, texture, GamePixelFormat, TPKAnim))
			return -1;
	//	SortTexturesByHash(TPKToolStuff, texture, GamePixelFormat);
		PrecalculateTotalSizes(TPKToolStuff, texture, TPKAnim);
		MasterChunkWriter(argv[argc - 1], TPKToolStuff, texture, GamePixelFormat, TPKAnim);
		free(TPKToolStuff);
		return 0;
	}

	if (ReadingMode)
	{
		if (!bDoFileChecks(argv[2]))
			return -1;
	}
	else
	{
		if (!bDoFileChecks(argv[1]))
			return -1;
	}

	strcpy((*TPKToolStuff).TotalFilePath, argv[argc - 1]);
	//strcpy(OutputFilePath, argv[argc - 1]);
	if (ReadingMode)
		MasterChunkReader(argv[2], argv[argc - 1], TPKToolStuff, texture, GamePixelFormat, TPKAnim, &TPKLink);
	else
		MasterChunkReader(argv[1], argv[argc - 1], TPKToolStuff, texture, GamePixelFormat, TPKAnim, &TPKLink);

	strcpy((*TPKToolStuff).TotalFilePath, argv[argc - 1]);
	strcat((*TPKToolStuff).TotalFilePath, "\\");
	strcat((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).StatFileName);
	printf("%s Outputting statistics to: %s\n", PRINTTYPE_INFO, (*TPKToolStuff).TotalFilePath);
	OutputInfoToFile((*TPKToolStuff).TotalFilePath, texture, TPKToolStuff, GamePixelFormat, TPKAnim);

	strcpy((*TPKToolStuff).TotalFilePath, argv[argc - 1]);
	strcat((*TPKToolStuff).TotalFilePath, "\\");
	strcat((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).SettingsFileName);
	printf("%s Outputting settings to: %s\n", PRINTTYPE_INFO, (*TPKToolStuff).TotalFilePath);
	SpitSettingsFile((*TPKToolStuff).TotalFilePath, texture, TPKToolStuff, TPKAnim);

	free(TPKToolStuff);

	return 0;
}
