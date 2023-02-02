#pragma once
#include "stdafx.h"
#include "TPKTool.h"

int WriteChunkTypeAndSize(FILE *fout, unsigned int ChunkMagic, unsigned int ChunkSize)
{
	fwrite(&ChunkMagic, 4, 1, fout);
	fwrite(&ChunkSize, 4, 1, fout);
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

int TPKChildType1Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal)
{
	printf("%s Writing TPK child 1 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	
	WriteChunkTypeAndSize(fout, TPK_CHILD1_CHUNKID, ChunkSize);
	
	fwrite(&(*InTPKToolInternal).TPKTypeValue, sizeof(int), 1, fout);
	fwrite(&(*InTPKToolInternal).TPKTypeName, sizeof(char), 0x1C, fout);
	fwrite(&(*InTPKToolInternal).TPKPathName, sizeof(char), 0x40, fout);
	fwrite(&(*InTPKToolInternal).HashArray, sizeof(int), 7, fout);
	return 1;
}

int TPKChildType2Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct)
{
	printf("%s Writing TPK child 2 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	unsigned int TempZero = 0;
	WriteChunkTypeAndSize(fout, TPK_CHILD2_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i <= (*InTPKToolInternal).TextureCategoryHashCount - 1; i++)
	{
		fwrite(&InTexStruct[i].Child4.NameHash, sizeof(int), 1, fout);
		fwrite(&TempZero, 4, 1, fout);
	}
	return 1;
}

int TPKChildType4Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct)
{
	unsigned char TexNameSize = 0;
	printf("%s Writing TPK child 4 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_CHILD4_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureCategoryHashCount; i++)
	{
		InTexStruct[i].Child4.Unknown = 0;
		InTexStruct[i].Child4.Padding_990[0] = 0;
		InTexStruct[i].Child4.Padding_990[1] = 0;
		InTexStruct[i].Child4.PalettePlacement = 0xFFFFFFFF;
		fwrite(&InTexStruct[i].Child4, sizeof(TextureInfo), 1, fout);
		TexNameSize = strlen(InTexStruct[i].TexName) + 1;
		fwrite(&TexNameSize, sizeof(char), 1, fout);

		fwrite(&InTexStruct[i].TexName, sizeof(char), TexNameSize, fout);
	}
	return 1;
}

int TPK_v2_ChildType4Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct)
{
	// UG2 & MW
	printf("%s Writing TPK child 4 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	unsigned char TexNameSize = 0;
	//TPKChild4Struct_TPKv4 *TPKv4Child4_Bridge = (TPKChild4Struct_TPKv4*)calloc(1, sizeof(TPKChild4Struct_TPKv4));
	OldTextureInfo *TPKv4Child4_Bridge = (OldTextureInfo*)calloc(1, sizeof(OldTextureInfo));

	WriteChunkTypeAndSize(fout, TPK_CHILD4_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureCategoryHashCount; i++)
	{
		strncpy((*TPKv4Child4_Bridge).DebugName, InTexStruct[i].TexName, 0x18);
		(*TPKv4Child4_Bridge).NameHash = InTexStruct[i].Child4.NameHash;
		(*TPKv4Child4_Bridge).ClassNameHash = InTexStruct[i].Child4.ClassNameHash;
		(*TPKv4Child4_Bridge).ImagePlacement = InTexStruct[i].Child4.ImagePlacement;
		(*TPKv4Child4_Bridge).PalettePlacement = InTexStruct[i].Child4.PalettePlacement;
		(*TPKv4Child4_Bridge).ImageSize = InTexStruct[i].Child4.ImageSize;
		(*TPKv4Child4_Bridge).PaletteSize = InTexStruct[i].Child4.PaletteSize;
		(*TPKv4Child4_Bridge).BaseImageSize = InTexStruct[i].Child4.BaseImageSize;
		(*TPKv4Child4_Bridge).Width = InTexStruct[i].Child4.Width;
		(*TPKv4Child4_Bridge).Height = InTexStruct[i].Child4.Height;
		(*TPKv4Child4_Bridge).ShiftWidth = InTexStruct[i].Child4.ShiftWidth;
		(*TPKv4Child4_Bridge).ShiftHeight = InTexStruct[i].Child4.ShiftHeight;
		(*TPKv4Child4_Bridge).ImageCompressionType = InTexStruct[i].Child4.ImageCompressionType;
		(*TPKv4Child4_Bridge).PaletteCompressionType = InTexStruct[i].Child4.PaletteCompressionType;
		(*TPKv4Child4_Bridge).NumPaletteEntries = InTexStruct[i].Child4.NumPaletteEntries;
		(*TPKv4Child4_Bridge).NumMipMapLevels = InTexStruct[i].Child4.NumMipMapLevels;
		(*TPKv4Child4_Bridge).TilableUV = InTexStruct[i].Child4.TilableUV;
		(*TPKv4Child4_Bridge).BiasLevel = InTexStruct[i].Child4.BiasLevel;
		(*TPKv4Child4_Bridge).RenderingOrder = InTexStruct[i].Child4.RenderingOrder;
		(*TPKv4Child4_Bridge).ScrollType = InTexStruct[i].Child4.ScrollType;
		(*TPKv4Child4_Bridge).UsedFlag = InTexStruct[i].Child4.UsedFlag;
		(*TPKv4Child4_Bridge).ApplyAlphaSorting = InTexStruct[i].Child4.ApplyAlphaSorting;
		(*TPKv4Child4_Bridge).AlphaUsageType = InTexStruct[i].Child4.AlphaUsageType;
		(*TPKv4Child4_Bridge).AlphaBlendType = InTexStruct[i].Child4.AlphaBlendType;
		(*TPKv4Child4_Bridge).Flags = InTexStruct[i].Child4.Flags;
		//(*TPKv4Child4_Bridge).MipmapBiasType = InTexStruct[i].Child4.MipmapBiasType;
		//(*TPKv4Child4_Bridge).Unknown3 = InTexStruct[i].Child4.Padding;
		(*TPKv4Child4_Bridge).ScrollTimeStep = InTexStruct[i].Child4.ScrollTimeStep;
		(*TPKv4Child4_Bridge).ScrollSpeedS = InTexStruct[i].Child4.ScrollSpeedS;
		(*TPKv4Child4_Bridge).ScrollSpeedT = InTexStruct[i].Child4.ScrollSpeedT;
		(*TPKv4Child4_Bridge).OffsetS = InTexStruct[i].Child4.OffsetS;
		(*TPKv4Child4_Bridge).OffsetT = InTexStruct[i].Child4.OffsetT;
		(*TPKv4Child4_Bridge).ScaleS = InTexStruct[i].Child4.ScaleS;
		(*TPKv4Child4_Bridge).ScaleT = InTexStruct[i].Child4.ScaleT;

		fwrite(TPKv4Child4_Bridge, sizeof(OldTextureInfo), 1, fout);
	}
	free(TPKv4Child4_Bridge);
	return 1;
}

int TPKChildType5Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, GamePixelFormatStruct *InGamePixelFormat)
{
	printf("%s Writing TPK child 5 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_CHILD5_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureCategoryHashCount; i++)
	{
		for (unsigned int j = 0; j <= 0xB; j++)
			fputc(0, fout);
		fwrite(&InGamePixelFormat[i].FourCC, 4, 1, fout);
		fwrite(&InGamePixelFormat[i].Unknown1, 4, 1, fout);
		fwrite(&InGamePixelFormat[i].Unknown2, 4, 1, fout);
	}
	return 1;
}

int TPK_v2_360_ChildType5Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, GamePixelFormatStruct *InGamePixelFormat)
{
	printf("%s Writing TPK child 5 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);

	TPKChild5Struct_v2_360* GamePixelFormatBridge = (TPKChild5Struct_v2_360*)calloc(1, sizeof(TPKChild5Struct_v2_360));

	(*GamePixelFormatBridge).PixelFormatVal1 = 1;
	(*GamePixelFormatBridge).SomeVal1 = 6;
	(*GamePixelFormatBridge).SomeVal1 = 1; // NOT ALWAYS TRUE

	WriteChunkTypeAndSize(fout, TPK_CHILD5_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureCategoryHashCount; i++)
	{
		switch (InGamePixelFormat[i].FourCC)
		{
		case FOURCC_ARGB:
			(*GamePixelFormatBridge).D3DFormat = D3DFMT_LIN_A8R8G8B8;
			break;
		case FOURCC_DXT1:
			(*GamePixelFormatBridge).D3DFormat = D3DFMT_LIN_DXT1;
			break;
		case FOURCC_DXT3:
			(*GamePixelFormatBridge).D3DFormat = D3DFMT_LIN_DXT3;
			break;
		case FOURCC_DXT5:
			(*GamePixelFormatBridge).D3DFormat = D3DFMT_LIN_DXT5;
			break;
		case FOURCC_ATI2:
			(*GamePixelFormatBridge).D3DFormat = D3DFMT_LIN_DXN;
			break;
		default:
			(*GamePixelFormatBridge).D3DFormat = D3DFMT_LIN_A8R8G8B8;
			break;
		}

		fwrite(GamePixelFormatBridge, sizeof(TPKChild5Struct_v2_360), 1, fout);
	}

	free(GamePixelFormatBridge);

	return 1;
}

int TPK_Xbox_ChildType5Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, GamePixelFormatStruct *InGamePixelFormat)
{
	printf("%s Writing TPK child 5 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);

	TPKChild5Struct_Xbox* GamePixelFormatBridge = (TPKChild5Struct_Xbox*)calloc(1, sizeof(TPKChild5Struct_Xbox));

	(*GamePixelFormatBridge).unk1 = 1;
	(*GamePixelFormatBridge).unk2 = 0x302;
	(*GamePixelFormatBridge).unk3 = 0x303; // NOT ALWAYS TRUE

	WriteChunkTypeAndSize(fout, TPK_CHILD5_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureCategoryHashCount; i++)
	{
		//(*GamePixelFormatBridge).bSwizzled = false;
		switch (InGamePixelFormat[i].FourCC)
		{
		case FOURCC_ARGB:
			(*GamePixelFormatBridge).PixelFormat = 0x6;
			break;
		case FOURCC_DXT1:
			(*GamePixelFormatBridge).PixelFormat = 0xC;
			break;
		case FOURCC_DXT3:
			(*GamePixelFormatBridge).PixelFormat = 0xE;
			break;
		case FOURCC_DXT5:
			(*GamePixelFormatBridge).PixelFormat = 0xF;
			break;
		default:
			(*GamePixelFormatBridge).PixelFormat = 0x6;
			break;
		}

		fwrite(GamePixelFormatBridge, sizeof(TPKChild5Struct_Xbox), 1, fout);
	}

	free(GamePixelFormatBridge);

	return 1;
}

int TPK_PS3_ChildType5Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, GamePixelFormatStruct *InGamePixelFormat)
{
	printf("%s Writing TPK child 5 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);

	TPKChild5Struct_PS3* GamePixelFormatBridge = (TPKChild5Struct_PS3*)calloc(1, sizeof(TPKChild5Struct_PS3));

	(*GamePixelFormatBridge).SomeVal1 = 2;
	(*GamePixelFormatBridge).SomeVal2 = 3;

	WriteChunkTypeAndSize(fout, TPK_CHILD5_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureCategoryHashCount; i++)
	{
		switch (InGamePixelFormat[i].FourCC)
		{
		case FOURCC_DXT1:
			(*GamePixelFormatBridge).PixelFormatVal1 = 0;
			(*GamePixelFormatBridge).PixelFormatVal2 = 3;
			(*GamePixelFormatBridge).PixelFormatVal3 = 3;
			break;
		case FOURCC_DXT5:
			(*GamePixelFormatBridge).PixelFormatVal1 = 1;
			(*GamePixelFormatBridge).PixelFormatVal2 = 3;
			(*GamePixelFormatBridge).PixelFormatVal3 = 3;
			break;
		case FOURCC_ARGB:
			(*GamePixelFormatBridge).PixelFormatVal1 = 1;
			(*GamePixelFormatBridge).PixelFormatVal2 = 1;
			(*GamePixelFormatBridge).PixelFormatVal3 = 0;
			break;
		default:
			(*GamePixelFormatBridge).PixelFormatVal1 = 1;
			(*GamePixelFormatBridge).PixelFormatVal2 = 1;
			(*GamePixelFormatBridge).PixelFormatVal3 = 0;
			break;
		}

		fwrite(GamePixelFormatBridge, sizeof(TPKChild5Struct_PS3), 1, fout);
	}

	free(GamePixelFormatBridge);

	return 1;
}

int TPK_v2_ChildType5Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, GamePixelFormatStruct *InGamePixelFormat)
{
	printf("%s Writing TPK child 5 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);

	GamePixelFormatStruct_v2* GamePixelFormatBridge = (GamePixelFormatStruct_v2*)calloc(1, sizeof(GamePixelFormatStruct_v2));

	WriteChunkTypeAndSize(fout, TPK_CHILD5_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureCategoryHashCount; i++)
	{
		(*GamePixelFormatBridge).FourCC = InGamePixelFormat[i].FourCC;
		(*GamePixelFormatBridge).Unknown3 = InGamePixelFormat[i].Unknown1;
		(*GamePixelFormatBridge).Unknown4 = InGamePixelFormat[i].Unknown2;
		(*GamePixelFormatBridge).Unknown5 = InGamePixelFormat[i].Unknown3;

		fwrite(GamePixelFormatBridge, sizeof(GamePixelFormatStruct_v2), 1, fout);

		/*for (unsigned int j = 0; j <= 0xB; j++)
			fputc(0, fout);
		fwrite(&InGamePixelFormat[i].FourCC, 4, 1, fout);
		fwrite(&InGamePixelFormat[i].Unknown1, 4, 1, fout);
		fwrite(&InGamePixelFormat[i].Unknown2, 4, 1, fout);*/
	}

	free(GamePixelFormatBridge);

	return 1;
}

int TPKAnimChildType2Writer(FILE *fout, unsigned int ChunkSize, unsigned int AnimToWrite, TPKToolInternalStruct *InTPKToolInternal, TPKAnimStruct *InTPKAnim)
{
	printf("%s Writing TPK animation %d child 2 chunk: %X bytes\n", PRINTTYPE_INFO, AnimToWrite, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_ANIMCHILD2_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < InTPKAnim[AnimToWrite].Frames; i++)
	{
		fwrite(&(*InTPKToolInternal).AnimFrameHashArray[AnimToWrite][i], sizeof(int), 1, fout);
		for (unsigned int j = 0; j < 0x8; j++)
			fputc(0, fout);
	}
	return 1;
}

int TPKAnimChildType1Writer(FILE *fout, unsigned int ChunkSize, unsigned int AnimToWrite, TPKAnimStruct *InTPKAnim)
{
	printf("%s Writing TPK animation %d child 1 chunk: %X bytes\n", PRINTTYPE_INFO, AnimToWrite, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_ANIMCHILD1_CHUNKID, ChunkSize);
	fwrite(&InTPKAnim[AnimToWrite], sizeof(TPKAnimStruct), 1, fout);
	return 1;
}


int TPKAnimChunkWriter(FILE *fout, unsigned int ChunkSize, unsigned int AnimToWrite, TPKToolInternalStruct *InTPKToolInternal, TPKAnimStruct *InTPKAnim)
{
	printf("%s Writing TPK animation %d chunk: %X bytes\n", PRINTTYPE_INFO, AnimToWrite, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_ANIM_CHUNKID, ChunkSize);
	TPKAnimChildType1Writer(fout, (*InTPKToolInternal).TPKAnimChild1Size[AnimToWrite], AnimToWrite, InTPKAnim);
	TPKAnimChildType2Writer(fout, (*InTPKToolInternal).TPKAnimChild2Size[AnimToWrite], AnimToWrite, InTPKToolInternal, InTPKAnim);
	return 1;
}

int TPKExtraChunkWriter(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TPKAnimStruct *InTPKAnim)
{
	printf("%s Writing TPK extra chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_EXTRACAPSULE_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < (*InTPKToolInternal).AnimCounter; i++)
		TPKAnimChunkWriter(fout, (*InTPKToolInternal).TPKAnimChunkSize[i], i, InTPKToolInternal, InTPKAnim);
	return 1;
}

int TPKChunkWriter(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormat, TPKAnimStruct *InTPKAnim)
{
	printf("%s Writing TPK chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPK_CHUNKID, ChunkSize);
	TPKChildType1Writer(fout, (*InTPKToolInternal).TPKChild1Size, InTPKToolInternal);
	TPKChildType2Writer(fout, (*InTPKToolInternal).TPKChild2Size, InTPKToolInternal, InTexStruct);
	//TPKChildType4Writer(fout, (*InTPKToolInternal).TPKChild4Size, InTPKToolInternal, InTexStruct);

	switch(WritingMode)
	{
	case TPKTOOL_WRITINGMODE_V2:
		TPK_v2_ChildType4Writer(fout, (*InTPKToolInternal).TPKChild4Size, InTPKToolInternal, InTexStruct);
		TPK_v2_ChildType5Writer(fout, (*InTPKToolInternal).TPKChild5Size, InTPKToolInternal, InGamePixelFormat);
		break;
	case TPKTOOL_WRITINGMODE_PLAT_XBOX:
		TPKChildType4Writer(fout, (*InTPKToolInternal).TPKChild4Size, InTPKToolInternal, InTexStruct);
		TPK_Xbox_ChildType5Writer(fout, (*InTPKToolInternal).TPKChild5Size, InTPKToolInternal, InGamePixelFormat);
		break;
	case TPKTOOL_WRITINGMODE_PLAT_V2_360:
		TPK_v2_ChildType4Writer(fout, (*InTPKToolInternal).TPKChild4Size, InTPKToolInternal, InTexStruct);
		TPK_v2_360_ChildType5Writer(fout, (*InTPKToolInternal).TPKChild5Size, InTPKToolInternal, InGamePixelFormat);
		break;
	case TPKTOOL_WRITINGMODE_PLAT_PS3:
		TPKChildType4Writer(fout, (*InTPKToolInternal).TPKChild4Size, InTPKToolInternal, InTexStruct);
		TPK_PS3_ChildType5Writer(fout, (*InTPKToolInternal).TPKChild5Size, InTPKToolInternal, InGamePixelFormat);
		break;
	default:
		TPKChildType4Writer(fout, (*InTPKToolInternal).TPKChild4Size, InTPKToolInternal, InTexStruct);
		TPKChildType5Writer(fout, (*InTPKToolInternal).TPKChild5Size, InTPKToolInternal, InGamePixelFormat);
		break;
	}

	if ((*InTPKToolInternal).AnimCounter)
		TPKExtraChunkWriter(fout, (*InTPKToolInternal).TPKExtraCapsuleSize, InTPKToolInternal, InTPKAnim);
	return 1;
}

int TPKDataChildType2Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct)
{
	printf("%s Writing TPK data child 2 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPKDATA_CHILD2_CHUNKID, ChunkSize);

	for (unsigned int i = 0; i <= 0x77; i++) // THIS NEEDS WORK FOR 360!!
		fputc(0x11, fout);
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureCategoryHashCount; i++)
		WriteDDSDataToFile(InTexStruct[i].FilesystemPath, fout);
	return 1;
}

int TPKDataChildType1Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal)
{
	TPKLinkStruct *WriterTPKLink = (TPKLinkStruct*)calloc(1, sizeof(TPKLinkStruct)); // allocing, because this is useless to us atm anyway we don't care about it later, and also to avoid stack overflow

	printf("%s Writing TPK data child 1 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPKDATA_CHILD1_CHUNKID, ChunkSize);
	(*WriterTPKLink).Unknown1 = 0;
	(*WriterTPKLink).Unknown2 = 0;
	(*WriterTPKLink).NumberOfTPKs = 1;
	(*WriterTPKLink).TPKHash[0] = (*InTPKToolInternal).HashArray[0];
	(*WriterTPKLink).Unknown3 = 0;
	(*WriterTPKLink).Unknown4 = 0;

	fwrite(&(*WriterTPKLink).Unknown1, 4, 1, fout);
	fwrite(&(*WriterTPKLink).Unknown2, 4, 1, fout);
	fwrite(&(*WriterTPKLink).NumberOfTPKs, 4, 1, fout);
	fwrite(&(*WriterTPKLink).TPKHash[0], 4, 1, fout);
	fwrite(&(*WriterTPKLink).Unknown3, 4, 1, fout);
	fwrite(&(*WriterTPKLink).Unknown4, 4, 1, fout);

	free(WriterTPKLink);

	return 1;
}


int TPKDataChunkWriter(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct)
{
	printf("%s Writing TPK data chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPKDATA_CHUNKID, ChunkSize);
	TPKDataChildType1Writer(fout, (*InTPKToolInternal).TPKDataChild1Size, InTPKToolInternal);
	ZeroChunkWriter(fout, 0x50);
	TPKDataChildType2Writer(fout, (*InTPKToolInternal).TPKDataChild2Size, InTPKToolInternal, InTexStruct);
	return 1;
}

int MasterChunkWriter(const char* OutFileName, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct, GamePixelFormatStruct *InGamePixelFormat, TPKAnimStruct *InTPKAnim)
{
	FILE *fout = fopen(OutFileName, "wb");
	if (!fout)
		return -1;

	unsigned int RelativeStart = 0;
	printf("%s Writing the master TPK capsule: %X bytes\n", PRINTTYPE_INFO, (*InTPKToolInternal).TPKCapsuleSize);
	WriteChunkTypeAndSize(fout, TPKCAPSULE_CHUNKID, (*InTPKToolInternal).TPKCapsuleSize);
	RelativeStart = ftell(fout);
	ZeroChunkWriter(fout, 0x30);
	TPKChunkWriter(fout, (*InTPKToolInternal).TPKChunkSize, InTPKToolInternal, InTexStruct, InGamePixelFormat, InTPKAnim);
	ZeroChunkWriter(fout, (*InTPKToolInternal).TPKChunkAlignSize);
	TPKDataChunkWriter(fout, (*InTPKToolInternal).TPKDataChunkSize, InTPKToolInternal, InTexStruct);
	fclose(fout);
	return 1;
}
