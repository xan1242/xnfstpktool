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
			if (InTexStruct[j].Child4.NameHash == (*InTPKInternal).TextureCategoryHashArray[i])
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
	fscanf(fin, "Name = %s\n", &(*InTPKInternal).TPKTypeName);
	fscanf(fin, "Version = %d\n", &(*InTPKInternal).TPKTypeValue);
	fscanf(fin, "Filename = %s\n", &(*InTPKInternal).TPKPathName);
	fscanf(fin, "FilenameHash = %X\n", &(*InTPKInternal).HashArray[0]);
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
		fscanf(fin, "\n[%X]\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.NameHash);
		(*InTPKInternal).TextureCategoryHashArray[(*InTPKInternal).TextureCategoryHashCount] = InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.NameHash;

		fscanf(fin, "File = %s\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].FilesystemPath);
		if (!(CheckIfValidDDS(InTexStruct[(*InTPKInternal).TextureCategoryHashCount].FilesystemPath)))
		{
			printf("%s File %s invalid, breaking here.\nMake sure all your DDS files are valid first.\n", PRINTTYPE_ERROR, InTexStruct[(*InTPKInternal).TextureCategoryHashCount].FilesystemPath);
			return 0;
		}

		fscanf(fin, "Name = %s\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].TexName);
		fscanf(fin, "ClassNameHash = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ClassNameHash);
		fscanf(fin, "ShiftWidth = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ShiftWidth);
		fscanf(fin, "ShiftHeight = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ShiftHeight);
		fscanf(fin, "ImageCompressionType = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ImageCompressionType);
		fscanf(fin, "PaletteCompressionType = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.PaletteCompressionType);
		fscanf(fin, "NumPaletteEntries = %hX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.NumPaletteEntries);
		fscanf(fin, "TilableUV = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.TilableUV);
		fscanf(fin, "BiasLevel = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.BiasLevel);
		fscanf(fin, "RenderingOrder = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.RenderingOrder);
		fscanf(fin, "ScrollType = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ScrollType);
		fscanf(fin, "UsedFlag = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.UsedFlag);
		fscanf(fin, "ApplyAlphaSorting = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ApplyAlphaSorting);
		fscanf(fin, "AlphaUsageType = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.AlphaUsageType);
		fscanf(fin, "AlphaBlendType = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.AlphaBlendType);
		fscanf(fin, "Flags = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Flags);
		fscanf(fin, "MipmapBiasType = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.MipmapBiasType);
		fscanf(fin, "ScrollTimeStep = %hX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ScrollTimeStep);
		fscanf(fin, "ScrollSpeedS = %hX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ScrollSpeedS);
		fscanf(fin, "ScrollSpeedT = %hX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ScrollSpeedT);
		fscanf(fin, "OffsetS = %hX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.OffsetS);
		fscanf(fin, "OffsetT = %hX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.OffsetT);
		fscanf(fin, "ScaleS = %hX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ScaleS);
		fscanf(fin, "ScaleT = %hX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.ScaleT);
		fscanf(fin, "Unknown1 = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Padding);

		if (WritingMode == TPKTOOL_WRITINGMODE_V2)
		{
			fscanf(fin, "PixelFormatUnk1 = %X\n", &InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown1);
			fscanf(fin, "PixelFormatUnk2 = %X\n", &InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown2);
			fscanf(fin, "PixelFormatUnk3 = %X\n", &InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown3);

			if (!InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown2)
			{
				// using mostcommon values
				//InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown1 = 1;
				InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown2 = 5;
				InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown3 = 6;
			}
		}
		else
		{
			fscanf(fin, "PixelFormatUnk1 = %X\n", &ReadNumber);
			fscanf(fin, "PixelFormatUnk2 = %X\n", &ReadNumber);
			fscanf(fin, "PixelFormatUnk3 = %X\n", &ReadNumber);
		}
		// REPURPOSING CHILD4's VALUES! IF CARBON IS USED, THESE ARE ZEROED OUT! HACK!
		//if (WritingMode == TPKTOOL_WRITINGMODE_V2)
		//{
		//	fscanf(fin, "Unknown10 = %X\n", &InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown1);
		//	fscanf(fin, "Unknown11 = %X\n", &InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown2);
		//	fscanf(fin, "Unknown12 = %X\n", &InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown3);
		//	// TEMPORARY HACK
		//	//InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown1 = 1;
		//	//InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown2 = 5;
		//	//InGamePixelFormat[(*InTPKInternal).TextureCategoryHashCount].Unknown3 = 6;
		//}
		//else
		//{
		//	fscanf(fin, "Unknown10 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown10);
		//	fscanf(fin, "Unknown11 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown11);
		//	fscanf(fin, "Unknown12 = %X\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown12);
		//}
		//fscanf(fin, "Unknown17 = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown17);
		//fscanf(fin, "Unknown18 = %hhX\n", &InTexStruct[(*InTPKInternal).TextureCategoryHashCount].Child4.Unknown18);

		(*InTPKInternal).TextureCategoryHashCount++;
	}

	if ((*InTPKInternal).TextureCategoryHashCount > 1)
		SortTexturesByHash(InTPKInternal, InTexStruct, InGamePixelFormat);

	for (unsigned int i = 0; i < (*InTPKInternal).TextureCategoryHashCount; i++)
	{
		ReadDDSData(InTexStruct, InGamePixelFormat, &(*InTPKInternal).RelativeDDSDataOffset, i);

		if (InGamePixelFormat[i].FourCC == 0x15)
			InTexStruct[i].Child4.BaseImageSize = ((InTexStruct[i].Child4.Width) * (InTexStruct[i].Child4.Height)) * TEXTURENUMCHANNELS;
		else
			InTexStruct[i].Child4.BaseImageSize = flp2(InTexStruct[i].Child4.ImageSize);
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
	struct stat st = { 0 }; // filestat for folder existence

#ifdef TPKTOOL_WIPVER
	printf("Xanvier's NFS TPK Tool version %d !WIP!\nIf you have this version, you're naughty.\n\n", TPKTOOL_VERSION);
#else
	printf("Xanvier's NFS TPK Tool version %d\n\n", TPKTOOL_VERSION);
#endif

	if (argc <= 1)
	{
		printf("%s Too few / wrong arguments passed.\nUsage: [-w/-h/-?] InFile [OutFile]\nFor help pass -h or -?\n", PRINTTYPE_ERROR);
		return -1;
	}

	if (strncmp(argv[1], "-h", 2) == 0 || strncmp(argv[1], "-?", 2) == 0)
	{
		puts(TPKTOOL_HELPMESSAGE);
		return 0;
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
		else if (strncmp(argv[1] + 2, "360", 3) == 0)
		{
			printf("%s Going into 360 mode!\n", PRINTTYPE_INFO);
			WritingMode = TPKTOOL_WRITINGMODE_PLAT_360;
		}
		else if (strncmp(argv[1] + 2, "360-2", 5) == 0)
		{
			printf("%s Going into 360 TPKv2 mode!\n", PRINTTYPE_INFO);
			WritingMode = TPKTOOL_WRITINGMODE_PLAT_V2_360;
		}
		else if (strncmp(argv[1] + 2, "PS2", 3) == 0)
		{
			printf("%s Going into PS2 mode!\n", PRINTTYPE_INFO);
			WritingMode = TPKTOOL_WRITINGMODE_PLAT_PS2;
			printf("%s Unimplemented... Code coming soon.\n", PRINTTYPE_INFO);
			return 0;
		}
		else if (strncmp(argv[1] + 2, "PS2-2", 5) == 0)
		{
			printf("%s Going into PS2 TPKv2 mode!\n", PRINTTYPE_INFO);
			WritingMode = TPKTOOL_WRITINGMODE_PLAT_V2_PS2;
			printf("%s Unimplemented... Code coming soon.\n", PRINTTYPE_INFO);
			return 0;
		}
	}

	if (WritingMode)
	{
		//strcpy(OutputFilePath, argv[argc - 1]);
		if (!SettingsReader(argv[2], TPKToolStuff, texture, GamePixelFormat, TPKAnim))
			return -1;
		PrecalculateTotalSizes(TPKToolStuff, texture, TPKAnim);
		MasterChunkWriter(argv[argc - 1], TPKToolStuff, texture, GamePixelFormat, TPKAnim);
		free(TPKToolStuff);
		return 0;
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

	else if (strncmp(argv[1], "-360", 6) == 0)
	{
		printf("%s Going into 360 mode!\n", PRINTTYPE_INFO);
		ReadingMode = TPKTOOL_READINGMODE_PLAT_360;
	}

	else if (strncmp(argv[1], "-360-2", 6) == 0)
	{
		printf("%s Going into 360 TPKv2 mode!\n", PRINTTYPE_INFO);
		ReadingMode = TPKTOOL_READINGMODE_PLAT_V2_360;
	}

	else if (strncmp(argv[1], "-PS2", 6) == 0)
	{
		printf("%s Going into PS2 mode!\n", PRINTTYPE_INFO);
		ReadingMode = TPKTOOL_READINGMODE_PLAT_PS2;

		//unimplemented... temp code.
		printf("%s Unimplemented... Code coming soon. Use PC modes to get some data out for now.\n", PRINTTYPE_INFO);
		return 0;
	}

	else if (strncmp(argv[1], "-PS2-2", 6) == 0)
	{
		printf("%s Going into PS2 TPKv2 mode!\n", PRINTTYPE_INFO);
		ReadingMode = TPKTOOL_READINGMODE_PLAT_V2_PS2;

		//unimplemented... temp code.
		printf("%s Unimplemented... Code coming soon. Use PC modes to get some data out for now.\n", PRINTTYPE_INFO);
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

	//strcpy((*TPKToolStuff).TotalFilePath, argv[argc - 1]);
	
	//strcpy(OutputFilePath, argv[argc - 1]);

	if (ReadingMode)
	{
		if (argc == 3)
		{
			strncpy((*TPKToolStuff).OutputPath, argv[2], strrchr(argv[2], '.') - argv[2]);
		}
		else 
		{
			strcpy((*TPKToolStuff).OutputPath, argv[argc - 1]);
		}

		if (stat((*TPKToolStuff).OutputPath, &st) == -1)
		{
			// not cross-platform compatible, i know...
			printf("Making directory %s\n", (*TPKToolStuff).OutputPath);
			sprintf((*TPKToolStuff).TotalFilePath, "md %s\0", (*TPKToolStuff).OutputPath);
			system((*TPKToolStuff).TotalFilePath);
			//_mkdir(InputFilePath);
		}

		strcpy((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).OutputPath);
		MasterChunkReader(argv[2], (*TPKToolStuff).OutputPath, TPKToolStuff, texture, GamePixelFormat, TPKAnim, &TPKLink);
	}
	else
	{
		if (argc == 2)
		{
			strncpy((*TPKToolStuff).OutputPath, argv[1], strrchr(argv[1], '.') - argv[1]);
		}
		else
		{
			strcpy((*TPKToolStuff).OutputPath, argv[argc - 1]);
		}

		if (stat((*TPKToolStuff).OutputPath, &st) == -1)
		{
			// not cross-platform compatible, i know...
			printf("Making directory %s\n", (*TPKToolStuff).OutputPath);
			sprintf((*TPKToolStuff).TotalFilePath, "md %s\0", (*TPKToolStuff).OutputPath);
			system((*TPKToolStuff).TotalFilePath);
		}

		strcpy((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).OutputPath);
		MasterChunkReader(argv[1], (*TPKToolStuff).OutputPath, TPKToolStuff, texture, GamePixelFormat, TPKAnim, &TPKLink);
	}
	// phasing out stat file in favor of more detailed ini
	strcpy((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).OutputPath);
	strcat((*TPKToolStuff).TotalFilePath, "\\");
	strcat((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).StatFileName);
	printf("%s Outputting statistics to: %s\n", PRINTTYPE_INFO, (*TPKToolStuff).TotalFilePath);
	OutputInfoToFile((*TPKToolStuff).TotalFilePath, texture, TPKToolStuff, GamePixelFormat, TPKAnim);

	strcpy((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).OutputPath);
	strcat((*TPKToolStuff).TotalFilePath, "\\");
	strcat((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).SettingsFileName);
	printf("%s Outputting settings to: %s\n", PRINTTYPE_INFO, (*TPKToolStuff).TotalFilePath);
	SpitSettingsFile((*TPKToolStuff).TotalFilePath, texture, TPKToolStuff, TPKAnim, GamePixelFormat);

	free(TPKToolStuff);

	return 0;
}
