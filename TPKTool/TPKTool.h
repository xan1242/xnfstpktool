#pragma once

#define TPKTOOL_VERSION 1
//#define TPKTOOL_WIPVER

// uncomment this to enable decompression code, requires DecompressionCode.h which contains propriatery game code
// place DecompressionCode.h in the project folder (also named TPKTool)
//#define TPKTOOL_DECOMPRESSION

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

#define TPKTOOL_HELPMESSAGE "\
Usage: [-w[i[a]]/-i/-h/-?] InFile [indexnumber] OutFile\n\n\
Default mode extracts a single TPK.\n\
Default: InFile = TPK file | OutFile = Output folder path\n\
-w     : Writing mode, InFile = TPK settings ini file | OutFile = Output file name\n\
-wi    : Writing mode for specific index, InFile = Index settings ini file | OutFile = Output file name | indexnumber required\n\
-wia   : Writing mode for all indexes, InFile = Index settings ini file | OutFile = Output path\n\
-i     : Interactive indexing mode, InFile = TPK file | OutFile = nothing (yet)\n\
-h/-?  : Show this help message\n\
\nDDS files are extracted to their respective TPK hash directory. In case of interactive mode, the index is appended.\n\
For files with multiple TPKs in the archive, use the interactive indexing mode.\n\
Compressed TPKs are NOT fully supported yet.\n\
Carbon and ProStreet are currently only supported!"

#define TPKTOOL_INDEXMESSAGE "Type a number of which TPK you want to extract, type 'a' for all, 'u' to unpack a specific index, 'ua' to unpack all.\n\
Type 'p' to pack a specific index, 'pa' to pack all indexes.\nType anything else to quit.\n>"

#define PRINTTYPE_ERROR "ERROR:"
#define PRINTTYPE_INFO "INFO:"
#define PRINTTYPE_WARNING "WARNING:"

struct GamePixelFormatStruct // temporary storage for unknown stuff
{
	unsigned int FourCC;
	unsigned int Unknown1;
	unsigned int Unknown2;
	unsigned int Unknown3;
	unsigned int Unknown4;
	unsigned int Unknown5;
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
	unsigned int Unknown[3];
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

struct TPKChild4Struct
{
	unsigned int Unknown13[3];
	unsigned int Hash;
	unsigned int Hash2;
	unsigned int DataOffset;
	unsigned int Unknown14;
	unsigned int DataSize;
	unsigned int Unknown1;
	unsigned int Scaler; // actually DATA SIZE AGAIN HOLY FUCK HOW DID I MISS THIS
	unsigned short int ResX;
	unsigned short int ResY;
	unsigned char MipmapCount;
	unsigned char Unknown3[3];
	unsigned int TexFlags;
	unsigned int Unknown4;
	unsigned int Unknown5;
	unsigned int Unknown6;
	unsigned int Unknown7;
	unsigned int Unknown8;
	unsigned int Unknown9;
	unsigned int Unknown10;
	unsigned int Unknown11;
	unsigned int Unknown12;
};

struct TexStruct
{
	char TexName[255];
	TPKChild4Struct Child4;
	/*unsigned int Hash;
	unsigned int Hash2;
	unsigned int DataOffset;
	unsigned int DataSize;
	unsigned int ResY;
	unsigned int ResX;
	unsigned int MipmapCount;
	unsigned int TexFlags;
	unsigned int Unknown1; // not using arrays on purpose, don't worry, I'm not a too big of an idiot
	unsigned int Scaler;
	unsigned int Unknown3;
	unsigned int Unknown4;
	unsigned int Unknown5;
	unsigned int Unknown6;
	unsigned int Unknown7;
	unsigned int Unknown8;
	unsigned int Unknown9;
	unsigned int Unknown10;
	unsigned int Unknown11;
	unsigned int Unknown12;*/
	char FilesystemPath[255];
	//DirectX::DDS_PIXELFORMAT PixelFormat;
};

struct Index
{
	unsigned long int TPKSize;
	unsigned int TPKHash;
	unsigned long int TPKOffset;
};
