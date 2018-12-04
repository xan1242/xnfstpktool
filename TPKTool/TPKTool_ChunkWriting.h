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
	//unsigned int somethingidunno = 0x8;
	WriteChunkTypeAndSize(fout, TPK_CHILD1_CHUNKID, ChunkSize);
	//fwrite(&somethingidunno, 4, 1, fout);
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
		fwrite(&InTexStruct[i].Child4.Hash, sizeof(int), 1, fout);
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
		InTexStruct[i].Child4.Unknown13[0] = 0;
		InTexStruct[i].Child4.Unknown13[1] = 0;
		InTexStruct[i].Child4.Unknown13[2] = 0;
		InTexStruct[i].Child4.Unknown14 = 0xFFFFFFFF;
		fwrite(&InTexStruct[i].Child4, sizeof(TPKChild4Struct), 1, fout);
		TexNameSize = strlen(InTexStruct[i].TexName) + 1;
		fwrite(&TexNameSize, sizeof(char), 1, fout);

	/*	TexNameSize = strlen(InTexStruct[i].TexName) + 1;
		for (unsigned int j = 0; j <= 0xB; j++)
			fputc(0, fout);
		fwrite(&InTexStruct[i].Child4.Hash, sizeof(int), 1, fout);
		fwrite(&InTexStruct[i].Child4.Hash2, sizeof(int), 1, fout);
		fwrite(&InTexStruct[i].Child4.DataOffset, sizeof(int), 1, fout);
		for (unsigned int j = 0; j <= 0x3; j++)
			fputc(0xFF, fout);
		fwrite(&InTexStruct[i].Child4.DataSize, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.Unknown1, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.Scaler, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.ResX, 2, 1, fout);
		fwrite(&InTexStruct[i].Child4.ResY, 2, 1, fout);
		fwrite(&InTexStruct[i].Child4.MipmapCount, 1, 1, fout);
		fwrite(&InTexStruct[i].Child4.MipmapCount, 1, 1, fout);
		fwrite(&InTexStruct[i].Child4.Unknown3, 2, 1, fout);
		fwrite(&InTexStruct[i].Child4.TexFlags, 4, 1, fout);

		fwrite(&InTexStruct[i].Child4.Unknown4, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.Unknown5, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.Unknown6, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.Unknown7, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.Unknown8, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.Unknown9, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.Unknown10, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.Unknown11, 4, 1, fout);
		fwrite(&InTexStruct[i].Child4.Unknown12, 4, 1, fout);

		fwrite(&TexNameSize, sizeof(char), 1, fout);*/
		fwrite(&InTexStruct[i].TexName, sizeof(char), TexNameSize, fout);
	}
	return 1;
}

int TPK_v2_ChildType4Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct)
{
	// UG2 & MW
	printf("%s Writing TPK child 4 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	unsigned char TexNameSize = 0;
	TPKChild4Struct_TPKv2 *TPKv2Child4_Bridge = (TPKChild4Struct_TPKv2*)calloc(1, sizeof(TPKChild4Struct_TPKv2));

	WriteChunkTypeAndSize(fout, TPK_CHILD4_CHUNKID, ChunkSize);
	for (unsigned int i = 0; i < (*InTPKToolInternal).TextureCategoryHashCount; i++)
	{
		strncpy((*TPKv2Child4_Bridge).TexName, InTexStruct[i].TexName, 0x18);
		(*TPKv2Child4_Bridge).Hash = InTexStruct[i].Child4.Hash;
		(*TPKv2Child4_Bridge).Hash2 = InTexStruct[i].Child4.Hash2;
		(*TPKv2Child4_Bridge).DataOffset = InTexStruct[i].Child4.DataOffset;
		(*TPKv2Child4_Bridge).Unknown14 = InTexStruct[i].Child4.Unknown14;
		(*TPKv2Child4_Bridge).DataSize = InTexStruct[i].Child4.DataSize;
		(*TPKv2Child4_Bridge).Unknown1 = InTexStruct[i].Child4.Unknown1;
		(*TPKv2Child4_Bridge).Scaler = InTexStruct[i].Child4.Scaler;
		(*TPKv2Child4_Bridge).ResX = InTexStruct[i].Child4.ResX;
		(*TPKv2Child4_Bridge).ResY = InTexStruct[i].Child4.ResY;
		(*TPKv2Child4_Bridge).MipmapCount = InTexStruct[i].Child4.MipmapCount;
	//	(*TPKv2Child4_Bridge).MipmapCount2 = InTexStruct[i].Child4.MipmapCount;
		(*TPKv2Child4_Bridge).Unknown3 = InTexStruct[i].Child4.Unknown3;
		(*TPKv2Child4_Bridge).UnkByteVal1 = InTexStruct[i].Child4.UnkByteVal1;
		(*TPKv2Child4_Bridge).UnkByteVal2 = InTexStruct[i].Child4.UnkByteVal2;
		(*TPKv2Child4_Bridge).UnkByteVal3 = InTexStruct[i].Child4.UnkByteVal3;
		(*TPKv2Child4_Bridge).Unknown17 = InTexStruct[i].Child4.Unknown17;
		(*TPKv2Child4_Bridge).Unknown18 = InTexStruct[i].Child4.Unknown18;

		//(*TPKv2Child4_Bridge).TexFlags = InTexStruct[i].Child4.TexFlags;
		(*TPKv2Child4_Bridge).Unknown4 = InTexStruct[i].Child4.Unknown4;
		(*TPKv2Child4_Bridge).Unknown5 = InTexStruct[i].Child4.Unknown5;
		(*TPKv2Child4_Bridge).Unknown6 = InTexStruct[i].Child4.Unknown6;
		(*TPKv2Child4_Bridge).Unknown7 = InTexStruct[i].Child4.Unknown7;
		(*TPKv2Child4_Bridge).Unknown8 = InTexStruct[i].Child4.Unknown8;
		(*TPKv2Child4_Bridge).Unknown9 = InTexStruct[i].Child4.Unknown9;

		fwrite(TPKv2Child4_Bridge, sizeof(TPKChild4Struct_TPKv2), 1, fout);
	}
	free(TPKv2Child4_Bridge);
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
		case 0x31545844:
			(*GamePixelFormatBridge).PixelFormatVal1 = 0;
			(*GamePixelFormatBridge).PixelFormatVal2 = 3;
			(*GamePixelFormatBridge).PixelFormatVal3 = 3;
			break;
		case 0x35545844:
			(*GamePixelFormatBridge).PixelFormatVal1 = 1;
			(*GamePixelFormatBridge).PixelFormatVal2 = 3;
			(*GamePixelFormatBridge).PixelFormatVal3 = 3;
			break;
		case 0x15:
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

		/*for (unsigned int j = 0; j <= 0xB; j++)
		fputc(0, fout);
		fwrite(&InGamePixelFormat[i].FourCC, 4, 1, fout);
		fwrite(&InGamePixelFormat[i].Unknown1, 4, 1, fout);
		fwrite(&InGamePixelFormat[i].Unknown2, 4, 1, fout);*/
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

	if (WritingMode == TPKTOOL_WRITINGMODE_V2)
	{
		TPK_v2_ChildType4Writer(fout, (*InTPKToolInternal).TPKChild4Size, InTPKToolInternal, InTexStruct);
		TPK_v2_ChildType5Writer(fout, (*InTPKToolInternal).TPKChild5Size, InTPKToolInternal, InGamePixelFormat);
	}
	else if (WritingMode == TPKTOOL_WRITINGMODE_PLAT_PS3)
	{
		TPKChildType4Writer(fout, (*InTPKToolInternal).TPKChild4Size, InTPKToolInternal, InTexStruct);
		TPK_PS3_ChildType5Writer(fout, (*InTPKToolInternal).TPKChild5Size, InTPKToolInternal, InGamePixelFormat);
	}
	else
	{
		TPKChildType4Writer(fout, (*InTPKToolInternal).TPKChild4Size, InTPKToolInternal, InTexStruct);
		TPKChildType5Writer(fout, (*InTPKToolInternal).TPKChild5Size, InTPKToolInternal, InGamePixelFormat);
	}

	if ((*InTPKToolInternal).AnimCounter)
		TPKExtraChunkWriter(fout, (*InTPKToolInternal).TPKExtraCapsuleSize, InTPKToolInternal, InTPKAnim);
	return 1;
}

int TPKDataChildType2Writer(FILE *fout, unsigned int ChunkSize, TPKToolInternalStruct *InTPKToolInternal, TexStruct *InTexStruct)
{
	printf("%s Writing TPK data child 2 chunk: %X bytes\n", PRINTTYPE_INFO, ChunkSize);
	WriteChunkTypeAndSize(fout, TPKDATA_CHILD2_CHUNKID, ChunkSize);

	for (unsigned int i = 0; i <= 0x77; i++)
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