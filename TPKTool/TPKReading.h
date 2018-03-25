#pragma once
#include <stdio.h>

int ReadChunkTypeAndSize(FILE *finput, unsigned int &ChunkMagic, unsigned int &ChunkSize);
int ZeroChunkReader(FILE *finput, unsigned int ChunkSize);
bool CheckIfVaildFile(FILE *finput);
int OutputDDS(const char* OutFilePath, unsigned int TexNumber, unsigned int RelativeStart);
int TPKDataChildType2Reader(FILE *finput, unsigned int ChunkSize);
int TPKDataChildType1Reader(FILE *finput, unsigned int ChunkSize);
int TPKDataChunkReader(FILE *finput, unsigned int ChunkSize);
int TPKChildType5Reader(FILE *finput, unsigned int ChunkSize);
int TPKChildType4Reader(FILE *finput, unsigned int ChunkSize);
int TPKChildType3Reader(FILE *finput, unsigned int ChunkSize);
int TPKChildType1Reader(FILE *finput, unsigned int ChunkSize);
int TPKChunkReader(FILE *finput, unsigned int ChunkSize);
int MasterChunkReader(FILE *finput);
