// XNFSTPKTool
// NFS TPK extractor & repacker
// TODO: fix 64MB memory leak
// TODO: finish TPK v4- support
// 08/2018 - v2: scrapped indexing mode, too ambicious for a simple commandline tool, will rework into a separate library so another tool can do it
// 02/2023 - NOTE: build only with "XDKLibs", currently 360 stuff is broken...
// 02/2023 - trying to deprecate printfs, callocs, etc. in favor of C++ equivalents
// 02/2023 - implementing mINI - TODO - implement error checks for critical parts of TPK and implement dummy data for non-critical parts

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
#define MINI_CASE_SENSITIVE
#include "includes\mINI\src\mini\ini.h"
#include <iostream>

using namespace std;

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
	char AnimSection[32];
	char AnimFrameKey[32];
	int hc = 0;
	cout << PRINTTYPE_INFO << " Opening: " << InFileName << "\n";

	mINI::INIFile inifile(InFileName);
	mINI::INIStructure ini;

	if (!inifile.read(ini))
	{
		cout << "Can't open file for reading: " << InFileName << "\n";
		return false;
	}
	if (!ini.has("TPK"))
	{
		cout << PRINTTYPE_ERROR << " Missing TPK section. Please check your settings ini file format.\n";
		return false;
	}

	strcpy_s((*InTPKInternal).TPKTypeName, ini["TPK"]["Name"].c_str());
	(*InTPKInternal).TPKTypeValue = (uint32_t)stoul(ini["TPK"]["Version"]);
	strcpy_s((*InTPKInternal).TPKPathName, ini["TPK"]["Filename"].c_str());
	(*InTPKInternal).HashArray[0] = (uint32_t)stoul(ini["TPK"]["FilenameHash"], nullptr, 0);
	(*InTPKInternal).AnimCounter = (uint32_t)stoul(ini["TPK"]["Animations"]);

	for (unsigned int i = 0; i < (*InTPKInternal).AnimCounter; i++)
	{
		sprintf_s(AnimSection, "Anim%d", i);
		strcpy_s(InTPKAnim[i].Name, ini[AnimSection]["Name"].c_str());
		InTPKAnim[i].Hash = (uint32_t)stoul(ini[AnimSection]["Hash"], nullptr, 0);
		InTPKAnim[i].Frames = (uint8_t)(stoul(ini[AnimSection]["Frames"]) & 0xFF);
		InTPKAnim[i].Framerate = (uint8_t)(stoul(ini[AnimSection]["Framerate"]) & 0xFF);
		InTPKAnim[i].Unknown1 = (uint32_t)stoul(ini[AnimSection]["Unknown1"], nullptr, 0);
		InTPKAnim[i].Unknown2 = (uint32_t)stoul(ini[AnimSection]["Unknown2"], nullptr, 0);
		InTPKAnim[i].Unknown3 = (uint16_t)(stoul(ini[AnimSection]["Unknown3"], nullptr, 0) & 0xFFFF);
		InTPKAnim[i].Unknown4 = (uint32_t)stoul(ini[AnimSection]["Unknown4"], nullptr, 0);
		InTPKAnim[i].Unknown5 = (uint32_t)stoul(ini[AnimSection]["Unknown5"], nullptr, 0);
		InTPKAnim[i].Unknown6 = (uint32_t)stoul(ini[AnimSection]["Unknown6"], nullptr, 0);
		for (unsigned int j = 0; j < InTPKAnim[i].Frames; j++)
		{
			sprintf_s(AnimFrameKey, "Frame%d", j);
			(*InTPKInternal).AnimFrameHashArray[i][j] = (uint32_t)stoul(ini[AnimSection][AnimFrameKey], nullptr, 0);
		}
	}

	// iterate through all sections in the ini
	for (auto const& it : ini)
	{
		auto const& section = it.first;
		// ignore any non-hex sections!
		bool bHexSection = all_of(section.begin(), section.end(), ::isxdigit);
		if (bHexSection)
		{
			auto& collection = ini[section];
			TextureInfo* ti = &InTexStruct[hc].Child4;

			(*ti).NameHash = (uint32_t)stoul(section, nullptr, 16);
			(*InTPKInternal).TextureCategoryHashArray[hc] = (*ti).NameHash;
			strcpy_s(InTexStruct[hc].FilesystemPath, collection["File"].c_str());
			if (!(CheckIfValidDDS(InTexStruct[hc].FilesystemPath)))
			{
				cout << PRINTTYPE_ERROR << " File " << collection["File"] << " invalid, breaking here.\nMake sure all your DDS files are valid first.\n";
				return false;
			}

			// apply some defaults for non-critical stuff
			(*ti).ClassNameHash = 0x1A93CF;
			(*ti).ImageCompressionType = 0x24; // TODO: add the enums to the code!
			(*ti).NumPaletteEntries = 0;
			(*ti).TilableUV = 0;
			(*ti).BiasLevel = 0;
			(*ti).RenderingOrder = 5;
			(*ti).ScrollType = 0;
			(*ti).UsedFlag = 0;
			(*ti).ApplyAlphaSorting = 0;
			(*ti).AlphaUsageType = 2;
			(*ti).AlphaBlendType = 1;
			(*ti).Flags = 0;
			(*ti).ScrollTimeStep = 0;
			(*ti).ScrollSpeedS = 0;
			(*ti).ScrollSpeedT = 0;
			(*ti).OffsetS = 0;
			(*ti).OffsetT = 256;
			(*ti).ScaleS = 256;
			(*ti).ScaleT = 0;
			if (WritingMode == TPKTOOL_WRITINGMODE_V2)
			{
				InGamePixelFormat[hc].Unknown1 = 1;
				InGamePixelFormat[hc].Unknown2 = 5;
				InGamePixelFormat[hc].Unknown3 = 6;
			}

			if (!collection.has("Name"))
			{
				cout << PRINTTYPE_ERROR << " Missing Name key of texture: 0x" << std::uppercase << std::hex << (*ti).NameHash << '\n';
				return false;
			}
			strcpy_s(InTexStruct[hc].TexName, collection["Name"].c_str());
			if (collection.has("ClassNameHash"))
				(*ti).ClassNameHash = (uint32_t)stoul(collection["ClassNameHash"], nullptr, 0);
			if (collection.has("ImageCompressionType"))
				(*ti).ImageCompressionType = (uint8_t)(stoul(collection["ImageCompressionType"], nullptr, 0) & 0xFF);
			if (collection.has("PaletteCompressionType"))
				(*ti).PaletteCompressionType = (uint8_t)(stoul(collection["PaletteCompressionType"], nullptr, 0) & 0xFF);
			if (collection.has("NumPaletteEntries"))
				(*ti).NumPaletteEntries = (uint16_t)(stoul(collection["NumPaletteEntries"]) & 0xFFFF);
			if (collection.has("TilableUV"))
				(*ti).TilableUV = (uint8_t)(stoul(collection["TilableUV"]) & 0xFF);
			if (collection.has("BiasLevel"))
				(*ti).BiasLevel = (uint8_t)(stoul(collection["BiasLevel"]) & 0xFF);
			if (collection.has("RenderingOrder"))
				(*ti).RenderingOrder = (uint8_t)(stoul(collection["RenderingOrder"]) & 0xFF);
			if (collection.has("ScrollType"))
				(*ti).ScrollType = (uint8_t)(stoul(collection["ScrollType"]) & 0xFF);
			if (collection.has("UsedFlag"))
				(*ti).UsedFlag = (uint8_t)(stoul(collection["UsedFlag"]) & 0xFF);
			if (collection.has("ApplyAlphaSorting"))
				(*ti).ApplyAlphaSorting = (uint8_t)(stoul(collection["ApplyAlphaSorting"]) & 0xFF);
			if (collection.has("AlphaUsageType"))
				(*ti).AlphaUsageType = (uint8_t)(stoul(collection["AlphaUsageType"]) & 0xFF);
			if (collection.has("AlphaBlendType"))
				(*ti).AlphaBlendType = (uint8_t)(stoul(collection["AlphaBlendType"]) & 0xFF);
			if (collection.has("Flags"))
				(*ti).Flags = (uint8_t)(stoul(collection["Flags"], nullptr, 0) & 0xFF);
			if (collection.has("ScrollTimeStep"))
				(*ti).ScrollTimeStep = (uint16_t)(stoul(collection["ScrollTimeStep"]) & 0xFFFF);
			if (collection.has("ScrollSpeedS"))
				(*ti).ScrollSpeedS = (uint16_t)(stoul(collection["ScrollSpeedS"]) & 0xFFFF);
			if (collection.has("ScrollSpeedT"))
				(*ti).ScrollSpeedT = (uint16_t)(stoul(collection["ScrollSpeedT"]) & 0xFFFF);
			if (collection.has("OffsetS"))
				(*ti).OffsetS = (uint16_t)(stoul(collection["OffsetS"]) & 0xFFFF);
			if (collection.has("OffsetT"))
				(*ti).OffsetT = (uint16_t)(stoul(collection["OffsetT"]) & 0xFFFF);
			if (collection.has("ScaleS"))
				(*ti).ScaleS = (uint16_t)(stoul(collection["ScaleS"]) & 0xFFFF);
			if (collection.has("ScaleT"))
				(*ti).ScaleT = (uint16_t)(stoul(collection["ScaleT"]) & 0xFFFF);

			if (WritingMode == TPKTOOL_WRITINGMODE_V2)
			{
				if (collection.has("PixelFormatUnk1"))
					InGamePixelFormat[hc].Unknown1 = (uint32_t)stoul(collection["PixelFormatUnk1"], nullptr, 0);
				if (collection.has("PixelFormatUnk2"))
					InGamePixelFormat[hc].Unknown2 = (uint32_t)stoul(collection["PixelFormatUnk2"], nullptr, 0);
				if (collection.has("PixelFormatUnk3"))
					InGamePixelFormat[hc].Unknown3 = (uint32_t)stoul(collection["PixelFormatUnk3"], nullptr, 0);
			}

			hc++;
		}
	}

	(*InTPKInternal).TextureCategoryHashCount = hc;



	if ((*InTPKInternal).TextureCategoryHashCount > 1)
		SortTexturesByHash(InTPKInternal, InTexStruct, InGamePixelFormat);

	for (unsigned int i = 0; i < (*InTPKInternal).TextureCategoryHashCount; i++)
	{
		ReadDDSData(InTexStruct, InGamePixelFormat, &(*InTPKInternal).RelativeDDSDataOffset, i);

		if (InGamePixelFormat[i].FourCC == FOURCC_ARGB)
			InTexStruct[i].Child4.BaseImageSize = ((InTexStruct[i].Child4.Width) * (InTexStruct[i].Child4.Height)) * TEXTURENUMCHANNELS;
		else
			InTexStruct[i].Child4.BaseImageSize = flp2(InTexStruct[i].Child4.ImageSize);
	}

	(*InTPKInternal).TextureDataCount = (*InTPKInternal).TextureCategoryHashCount; // THIS IS NOT TRUE FOR VINYLS!!!
	(*InTPKInternal).TPKDataChild2Size = (*InTPKInternal).RelativeDDSDataOffset + 0x78;

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

	printf("Speed TexturePack Tool\n\n");

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
			printf("%s Going into TPK version 4- mode!\n", PRINTTYPE_INFO);
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
			printf("%s Going into 360 TPKv4 mode!\n", PRINTTYPE_INFO);
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
			printf("%s Going into PS2 TPKv4 mode!\n", PRINTTYPE_INFO);
			WritingMode = TPKTOOL_WRITINGMODE_PLAT_V2_PS2;
			printf("%s Unimplemented... Code coming soon.\n", PRINTTYPE_INFO);
			return 0;
		}
		else if (strncmp(argv[1] + 2, "XBX", 3) == 0)
		{
			printf("%s Going into Xbox mode!\n", PRINTTYPE_INFO);
			WritingMode = TPKTOOL_WRITINGMODE_PLAT_XBOX;
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
		printf("%s Going into TPK version 4- mode!\n", PRINTTYPE_INFO);
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
		printf("%s Going into 360 TPKv4 mode!\n", PRINTTYPE_INFO);
		ReadingMode = TPKTOOL_READINGMODE_PLAT_V2_360;
	}

	else if (strncmp(argv[1], "-PS2", 6) == 0)
	{
		printf("%s Going into PS2 mode!\n", PRINTTYPE_INFO);
		ReadingMode = TPKTOOL_READINGMODE_PLAT_PS2;

		//unimplemented... temp code.
		//printf("%s Unimplemented... Code coming soon. Use PC modes to get some data out for now.\n", PRINTTYPE_INFO);
		//return 0;
	}

	else if (strncmp(argv[1], "-PS2-2", 6) == 0)
	{
		printf("%s Going into PS2 TPKv4 mode!\n", PRINTTYPE_INFO);
		ReadingMode = TPKTOOL_READINGMODE_PLAT_V2_PS2;

		//unimplemented... temp code.
		//printf("%s Unimplemented... Code coming soon. Use PC modes to get some data out for now.\n", PRINTTYPE_INFO);
		//return 0;
	}

	else if (strncmp(argv[1], "-XBX", 6) == 0)
	{
		printf("%s Going into Xbox mode!\n", PRINTTYPE_INFO);
		ReadingMode = TPKTOOL_READINGMODE_PLAT_XBOX;

		//unimplemented... temp code.
		//printf("%s Unimplemented... Code coming soon. Use PC modes to get some data out for now.\n", PRINTTYPE_INFO);
		//return 0;
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
			sprintf((*TPKToolStuff).TotalFilePath, "md \"%s\"\0", (*TPKToolStuff).OutputPath);
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
			sprintf((*TPKToolStuff).TotalFilePath, "md \"%s\"\0", (*TPKToolStuff).OutputPath);
			system((*TPKToolStuff).TotalFilePath);
		}

		strcpy((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).OutputPath);
		MasterChunkReader(argv[1], (*TPKToolStuff).OutputPath, TPKToolStuff, texture, GamePixelFormat, TPKAnim, &TPKLink);
	}

	strcpy((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).OutputPath);
	strcat((*TPKToolStuff).TotalFilePath, "\\");
	strcat((*TPKToolStuff).TotalFilePath, (*TPKToolStuff).SettingsFileName);
	printf("%s Outputting settings to: %s\n", PRINTTYPE_INFO, (*TPKToolStuff).TotalFilePath);
	SpitSettingsFile((*TPKToolStuff).TotalFilePath, texture, TPKToolStuff, TPKAnim, GamePixelFormat);

	if (ReadingMode == TPKTOOL_READINGMODE_PLAT_V2_PS2)
	{
		string CTEininame = argv[2];
		CTEininame += "_CTE.ini";

		std::cout << "Writing Console Texture Explorer ini to: " << CTEininame << '\n';

		WriteConsoleTexExplorerIni_PS2(CTEininame.c_str(), texture, TPKToolStuff);
	}

	free(TPKToolStuff);

	return 0;
}
