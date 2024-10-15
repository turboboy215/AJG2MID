/*Alberto José González (GB/GBC) to MIDI converter*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt;
long bank;
long offset;
long tablePtrLoc;
long tableOffset;
int i, j;
char outfile[1000000];
int songNum;
long seqPtrs[3];
long songPtr;
int chanMask;
long bankAmt;
int foundTable = 0;
int format = 0;
int astNum = 0;
int tempPtr = 0;
unsigned long seqList[500];

unsigned static char* romData;

const int PopUpPtrs[2] = { 0x59A6, 0x5E43 };
const int AstPtrs[13] = { 0x5712, 0x585F, 0x5D42, 0x5E3C, 0x60B8, 0x61FC, 0x654B, 0x66AF, 0x676B, 0x6A95, 0x6E62, 0x76CB, 0x7C65 };
const int PinoPtrs[4] = { 0x4C77, 0x5859, 0x5BF4, 0x6117};

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long ptr);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("Alberto Jose Gonzalez (GB/GBC) to TXT converter\n");
	if (args != 4)
	{
		printf("Usage: AJG2TXT <rom> <bank>\n");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			bank = strtol(argv[2], NULL, 16);
			offset = strtol(argv[3], NULL, 16);
			if (bank != 1)
			{
				bankAmt = bankSize;
			}
			else
			{
				bankAmt = 0;
			}
			fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize);
			fread(romData, 1, bankSize, rom);

			/*Get the song locations from the offset*/
			i = offset - bankAmt;
			songNum = 1;
			if (romData[i] == 0xC3)
			{
				if (bank == 0x02 && offset == 0x60F2)
				{
					format = 2;
				}
				while (romData[i] == 0xC3)
				{
					songPtr = ReadLE16(&romData[i + 1]);
					if (songPtr == 0x4925 && offset == 0x4018)
					{
						break;
					}
					if ((songPtr == 0x491D || songPtr == 0x6F8F || songPtr == 0x6888) && offset == 0x4062)
					{
						break;
					}
					if ((songPtr == 0x5B30 || songPtr == 0x5634) && offset == 0x406A)
					{
						break;
					}
					if (songPtr == 0x491D && offset == 0x4060)
					{
						break;
					}
					if ((songPtr == 0x4919 || songPtr == 0x4937) && offset == 0x4065)
					{
						break;
					}
					if (songPtr == 0x5BCD && offset == 0x408C)
					{
						break;
					}
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					song2txt(songNum, songPtr);
					i += 3;
					songNum++;

				}
			}
			/*Alternate method for Pop Up*/
			else if (bank == 0x02 && offset == 0x4000)
			{
				format = 6;
				songPtr = PopUpPtrs[0];
				printf("Song %i: 0x%04X\n", songNum, songPtr);
				song2txt(songNum, songPtr);
				i += 2;
				songNum++;
				songPtr = PopUpPtrs[1];
				printf("Song %i: 0x%04X\n", songNum, songPtr);
				song2txt(songNum, songPtr);
			}

			/*Alternate method for Asterix*/
			else if (bank == 0x04 && offset == 0x4000)
			{
				format = 3;
				for (astNum = 0; astNum < 13; astNum++)
				{
					songPtr = AstPtrs[astNum];
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					song2txt(songNum, songPtr);
					i += 2;
					songNum++;
				}
			}

			/*Alternate method for Pinocchio*/
			else if (bank == 0x03 && offset == 0x4000)
			{
				format = 3;
				for (astNum = 0; astNum < 4; astNum++)
				{
					songPtr = PinoPtrs[astNum];
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					song2txt(songNum, songPtr);
					i += 2;
					songNum++;
				}
			}

			else if (bank == 0x14 && offset == 0x4086)
			{
				format = 3;
				tempPtr = ReadLE16(&romData[offset - bankAmt]) - bankAmt;
				for (astNum = 0; astNum < 11; astNum++)
				{
					songPtr = ReadLE16(&romData[tempPtr]);
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					song2txt(songNum, songPtr);
					tempPtr += 2;
					songNum++;
				}
			}

			else if (bank == 0x40 && offset == 0x4087)
			{
				format = 3;
				tempPtr = romData[offset + 1 - bankAmt] + (romData[offset + 4 - bankAmt] * 0x100);
				for (astNum = 0; astNum < 10; astNum++)
				{
					songPtr = ReadLE16(&romData[tempPtr - bankAmt]);
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					song2txt(songNum, songPtr);
					tempPtr += 2;
					songNum++;
				}
			}

			else if (bank == 0x40 && offset == 0x4093)
			{
				format = 3;
				tempPtr = romData[offset + 1 - bankAmt] + (romData[offset + 4 - bankAmt] * 0x100);
				for (astNum = 0; astNum < 9; astNum++)
				{
					songPtr = ReadLE16(&romData[tempPtr - bankAmt]);
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					song2txt(songNum, songPtr);
					tempPtr += 2;
					songNum++;
				}
			}

			else if ((bank == 0x31 || bank == 0x2C) && offset == 0x4087)
			{
				format = 3;
				tempPtr = romData[offset + 1 - bankAmt] + (romData[offset + 4 - bankAmt] * 0x100);
				for (astNum = 0; astNum < 12; astNum++)
				{
					songPtr = ReadLE16(&romData[tempPtr - bankAmt]);
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					song2txt(songNum, songPtr);
					tempPtr += 2;
					songNum++;
				}
			}

			/*Add "missing" jingles to the Smurfs 1*/
			if (offset == 0x4018 && bank == 7 && romData[0x01] == 0x90)
			{
				songPtr = 0x7DF3;
				printf("Song %i: 0x%04X\n", songNum, songPtr);
				song2txt(songNum, songPtr);
				songNum++;
				songPtr = 0x7ECD;
				printf("Song %i: 0x%04X\n", songNum, songPtr);
				song2txt(songNum, songPtr);
			}

			/*Add additional songs to Bomb Jack*/
			if (offset == 0x60F2 && bank == 2)
			{
				songPtr = 0x60FE;
				printf("Song %i: 0x%04X\n", songNum, songPtr);
				song2txt(songNum, songPtr);
				songNum++;
				songPtr = 0x743E;
				printf("Song %i: 0x%04X\n", songNum, songPtr);
				song2txt(songNum, songPtr);
			}


			printf("The operation was successfully completed!\n");
			exit(0);

		}
	}
}

void song2txt(int songNum, long ptr)
{
	int curTrack = 0;
	int seqEnd = 0;
	int curPat = 0;
	int totalPats = 0;
	long curSeq = 0;
	long patLoop = 0;
	int curSeqNum = 0;
	int isInSeqPos = 0;
	int k = 0;
	int curNote = 0;
	int curChanSpeed = 0;
	long seqPos = 0;
	unsigned char command[4];
	int transpose = 0;
	int globalTranspose = 0;
	int c4Speed = 0;
	long c4Offset = 0;
	long jumpPos = 0;
	int repeat = 0;
	long loopPt = 0;
	sprintf(outfile, "song%i.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file song%i.txt!\n", songNum);
		exit(2);
	}
	else
	{
		/*Look for the "real" start of the channels in the code*/
		j = ptr - bankAmt;
		while (romData[j] != 0x11)
		{
			j++;
		}
		seqPtrs[0] = ReadLE16(&romData[j + 1]);
		if (ReadLE16(&romData[j + 11]) < ReadLE16(&romData[j + 16]))
		{
			seqPtrs[1] = ReadLE16(&romData[j + 7]);
			seqPtrs[2] = ReadLE16(&romData[j + 13]);
		}
		else
		{
			seqPtrs[1] = ReadLE16(&romData[j + 13]);
			seqPtrs[2] = ReadLE16(&romData[j + 7]);
		}

		/*Clear the sequence array*/
		for (j = 0; j < 500; j++)
		{
			seqList[j] = 0;
		}
	}

	for (curTrack == 0; curTrack < 3; curTrack++)
	{
		isInSeqPos = 0;
		curPat = 1;
		fprintf(txt, "Channel %i: 0x%04X\n", (curTrack + 1), seqPtrs[curTrack]);
		j = seqPtrs[curTrack];
		if (curTrack == 0)
		{
			while (j != seqPtrs[1] && j != seqPtrs[2] && ReadLE16(&romData[j - bankAmt]) < (bankSize * 2) && ReadLE16(&romData[j - bankAmt]) > bankAmt && ReadLE16(&romData[j - bankAmt]) != 0x0000 && isInSeqPos == 0)
			{
				for (k = 0; k < 500; k++)
				{
					if (j == seqList[k])
					{
						isInSeqPos = 1;
					}
				}
				if (isInSeqPos == 0)
				{
					curSeq = ReadLE16(&romData[j - bankAmt]);
					fprintf(txt, "Sequence %i: 0x%04X\n", curPat, curSeq);
					seqList[curSeqNum] = curSeq;
					j += 2;
					curSeqNum++;
					curPat++;
				}

			}
			if (ReadLE16(&romData[j - bankAmt]) == 0x0000)
			{
				patLoop = ReadLE16(&romData[j + 2 - bankAmt]);
				fprintf(txt, "Pattern loop: 0x%04X\n", patLoop);
			}
			fprintf(txt, "\n");
		}
		else if (curTrack == 1)
		{
			while (j != seqPtrs[0] && j != seqPtrs[2] && ReadLE16(&romData[j - bankAmt]) < (bankSize * 2) && ReadLE16(&romData[j - bankAmt]) > bankAmt && ReadLE16(&romData[j - bankAmt]) != 0x0000 && isInSeqPos == 0)
			{
				for (k = 0; k < 500; k++)
				{
					if (j == seqList[k])
					{
						isInSeqPos = 1;
					}
				}
				if (isInSeqPos == 0)
				{
					curSeq = ReadLE16(&romData[j - bankAmt]);
					fprintf(txt, "Sequence %i: 0x%04X\n", curPat, curSeq);
					seqList[curSeqNum] = curSeq;
					j += 2;
					curSeqNum++;
					curPat++;
				}
			}
			if (ReadLE16(&romData[j - bankAmt]) == 0x0000)
			{
				patLoop = ReadLE16(&romData[j + 2 - bankAmt]);
				fprintf(txt, "Pattern loop: 0x%04X\n", patLoop);
			}
			fprintf(txt, "\n");
		}
		else if (curTrack == 2)
		{
			while (j != seqPtrs[0] && j != seqPtrs[1] && ReadLE16(&romData[j - bankAmt]) < (bankSize * 2) && ReadLE16(&romData[j - bankAmt]) > bankAmt && ReadLE16(&romData[j - bankAmt]) != 0x0000 && isInSeqPos == 0)
			{
				for (k = 0; k < 500; k++)
				{
					if (j == seqList[k])
					{
						isInSeqPos = 1;
					}
				}
				if (isInSeqPos == 0)
				{
					curSeq = ReadLE16(&romData[j - bankAmt]);
					fprintf(txt, "Sequence %i: 0x%04X\n", curPat, curSeq);
					seqList[curSeqNum] = curSeq;
					j += 2;
					curSeqNum++;
					curPat++;
				}
			}
			if (ReadLE16(&romData[j - bankAmt]) == 0x0000)
			{
				patLoop = ReadLE16(&romData[j + 2 - bankAmt]);
				fprintf(txt, "Pattern loop: 0x%04X\n", patLoop);
			}
			fprintf(txt, "\n");
		}

	}

	/*Convert the sequence data to TXT*/
	for (curSeqNum = 0; curSeqNum < 500; curSeqNum++)
	{
		if (seqList[curSeqNum] == 0)
		{
			break;
		}
		fprintf(txt, "Sequence 0x%04X:\n", seqList[curSeqNum]);
		seqPos = seqList[curSeqNum];
		seqEnd = 0;

		while (seqEnd == 0)
		{
			/*Version 0*/
			if (format == 0 || format == 3)
			{
				command[0] = romData[seqPos - bankAmt];
				command[1] = romData[seqPos - bankAmt + 1];
				command[2] = romData[seqPos - bankAmt + 2];
				command[3] = romData[seqPos - bankAmt + 3];
				if (command[0] < 0x60)
				{
					curNote = command[0];
					fprintf(txt, "Play note: %01X\n", curNote);
					seqPos++;
				}

				else if (command[0] == 0x60)
				{
					transpose = (signed char)command[1];
					fprintf(txt, "Enable 'tom tom' mode, transpose: %i\n", transpose);
					seqPos += 2;


				}
				else if (command[0] == 0x61)
				{
					fprintf(txt, "Disable 'tom tom' mode?\n");
					seqPos++;
				}

				else if (command[0] == 0x62)
				{
					fprintf(txt, "Rest for a bar\n");
					seqPos++;
				}

				else if (command[0] == 0x63)
				{
					fprintf(txt, "Set channel parameters: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] == 0x64)
				{
					fprintf(txt, "Half rest?\n");
					seqPos++;
				}

				else if (command[0] == 0x65)
				{
					fprintf(txt, "Hold note for a bar\n");
					seqPos++;
				}

				else if (command[0] == 0x66)
				{
					fprintf(txt, "No note/rest for a bar\n");
					seqPos++;
				}

				else if (command[0] == 0x68)
				{
					fprintf(txt, "End of final note\n");
					seqEnd = 1;
				}

				else if (command[0] == 0x69)
				{
					globalTranspose = (signed char)command[1];
					fprintf(txt, "Transpose (global): %i\n", globalTranspose);
					seqPos += 2;
				}

				else if (command[0] == 0x6A)
				{
					fprintf(txt, "End of sequence\n");
					seqEnd = 1;
				}

				else if (command[0] == 0x6B)
				{
					fprintf(txt, "Rest for amount of time: %01X\n", command[1]);
					seqPos += 2;
				}

				else if (command[0] == 0x6C)
				{
					c4Speed = command[1];
					c4Offset = ReadLE16(&romData[seqPos + 2 - bankAmt]);
					fprintf(txt, "Set speed control of channel 4: %01X, offset: %04X\n", c4Speed, c4Offset);
					seqPos += 4;
				}

				else if (command[0] == 0x6D)
				{
					fprintf(txt, "End channel without loop after 68 value\n");
					seqPos++;
				}

				else if (command[0] == 0x6E)
				{
					fprintf(txt, "Effect/decay option 1\n");
					seqPos++;
				}

				else if (command[0] == 0x6F)
				{
					fprintf(txt, "Effect/decay option 2\n");
					seqPos++;
				}

				else if (command[0] == 0x70)
				{
					fprintf(txt, "Effect/decay option 3\n");
					seqPos++;
				}

				else if (command[0] == 0x71)
				{
					fprintf(txt, "Effect/decay option 4\n");
					seqPos++;
				}

				else if (command[0] == 0x72)
				{
					fprintf(txt, "Effect/decay option 5\n");
					seqPos++;
				}

				else if (command[0] == 0x73)
				{
					fprintf(txt, "Effect/decay option 6\n");
					seqPos++;
				}

				else if (command[0] == 0x74)
				{
					if (command[2] >= 0x01)
					{
						fprintf(txt, "Set channel note length/others: %01X, %01X, %01X\n", command[1], command[2], command[3]);
						seqPos += 4;
					}
					else if (command[2] == 0x00)
					{
						fprintf(txt, "Set channel note length/others: %01X, %01X\n", command[1], command[2]);
						seqPos += 3;
					}

				}

				else if (command[0] == 0x75)
				{
					fprintf(txt, "Set channel note size?: %01X\n", command[1]);
					seqPos += 2;
				}

				else if (command[0] == 0x76)
				{
					fprintf(txt, "Channel 3 setting 76: %01X\n", command[1]);
					seqPos += 2;
				}

				else if (command[0] == 0x77)
				{
					fprintf(txt, "Fade in effect (all channels): %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] == 0x78)
				{
					fprintf(txt, "Command 78\n");
					seqPos ++;
				}

				else if (command[0] == 0x79)
				{
					fprintf(txt, "Switch waveform: %04X\n", ReadLE16(&romData[seqPos + 1 - bankAmt]));
					seqPos += 3;
				}

				else if (command[0] == 0x7A)
				{
					transpose = (signed char)command[1];
					fprintf(txt, "Transpose (current channel): %i\n", transpose);
					seqPos += 2;
				}

				else if (command[0] == 0x7B)
				{
					if (command[1] == 0x00)
					{
						fprintf(txt, "Set channel envelope without echo: %01X\n", command[1]);
						seqPos += 2;
					}
					else if (command[1] >= 0x01)
					{
						fprintf(txt, "Set channel envelope/echo: %01X, %01X\n", command[1], command[2]);
						seqPos += 3;
					}
				}

				else if (command[0] == 0x7C)
				{
					fprintf(txt, "Turn off effect\n");
					seqPos++;
				}

				else if (command[0] == 0x7D)
				{
					loopPt = ReadLE16(&romData[seqPos + 1 - bankAmt]);
					fprintf(txt, "Go to loop point: %04X\n", loopPt);
					seqEnd = 1;

				}

				else if (command[0] == 0x7E)
				{
					jumpPos = ReadLE16(&romData[seqPos + 1 - bankAmt]);
					fprintf(txt, "Jump to position: %04X\n", jumpPos);
					seqPos += 3;

				}

				else if (command[0] == 0x7F)
				{
					jumpPos = ReadLE16(&romData[seqPos + 1 - bankAmt]);
					fprintf(txt, "Load routine: %04X\n", jumpPos);
					seqPos += 3;
				}

				else if (command[0] == 0x80)
				{
					fprintf(txt, "End of repeat point\n");
					seqPos++;
				}

				else if (command[0] == 0x81)
				{
					fprintf(txt, "Command 81\n");
					seqPos++;
				}

				else if (command[0] == 0x82)
				{
					fprintf(txt, "Command 82\n");
					seqPos++;
				}

				else if (command[0] == 0x83)
				{
					fprintf(txt, "Command 83: %01X\n", command[1]);
					seqPos += 2;
				}

				else if (command[0] == 0x84)
				{
					fprintf(txt, "Switch to instrument type 1\n");
					seqPos++;
				}

				else if (command[0] == 0x85)
				{
					fprintf(txt, "Switch to instrument type 2\n");
					seqPos++;
				}

				else if (command[0] == 0x86)
				{
					fprintf(txt, "Switch to instrument type 3\n");
					seqPos++;
				}

				else if (command[0] == 0x87)
				{
					fprintf(txt, "Switch to instrument type 4\n");
					seqPos++;
				}

				else if (command[0] == 0x88)
				{
					fprintf(txt, "Switch to instrument type 5\n");
					seqPos++;
				}

				else if (command[0] == 0x89)
				{
					fprintf(txt, "Switch to instrument type 6\n");
					seqPos++;
				}

				else if (command[0] == 0x8A)
				{
					fprintf(txt, "Switch to instrument type 7\n");
					seqPos++;
				}

				else if (command[0] == 0x8B)
				{
					fprintf(txt, "Switch to instrument type 8\n");
					seqPos++;
				}

				else if (command[0] == 0x8C)
				{
					fprintf(txt, "Switch to instrument type 9\n");
					seqPos++;
				}

				else if (command[0] == 0x8D)
				{
					fprintf(txt, "Switch to instrument type 10\n");
					seqPos++;
				}

				else if (command[0] == 0x8E)
				{
					fprintf(txt, "Switch to instrument type 11\n");
					seqPos++;
				}

				else if (command[0] == 0x8F)
				{
					fprintf(txt, "Switch to instrument type 12\n");
					seqPos++;
				}

				else if (command[0] >= 0x90 && command[0] < 0xA0)
				{
					fprintf(txt, "Effect: %01X\n", command[0]);
					seqPos++;
				}
				else if (command[0] >= 0xA0 && command[0] < 0xA9)
				{
					fprintf(txt, "Effect: %01X\n", command[0]);
					seqPos++;
				}

				else if (command[0] == 0xA9)
				{
					fprintf(txt, "Command A9: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] >= 0xAA && command[0] < 0xAC)
				{
					fprintf(txt, "Effect: %01X\n", command[0]);
					seqPos++;
				}

				else if (command[0] >= 0xAC && command[0] <= 0xB9)
				{
					fprintf(txt, "Set up waveform/volume envelope: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] >= 0xC0 && command[0] <= 0xCF && format == 3)
				{
					fprintf(txt, "Set up waveform/volume envelope: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] > 0xB9 && command[0] < 0xE9)
				{
					curChanSpeed = command[0];
					fprintf(txt, "Change channel speed: %01X\n", curChanSpeed);
					seqPos++;
				}

				else if (command[0] >= 0xE9)
				{
					repeat = command[0] - 0xE7;
					fprintf(txt, "Repeat: %i\n", repeat);
					seqPos++;
				}

				else
				{
					fprintf(txt, "Unknown command %01X\n", command[0]);
					seqPos++;
				}
			}

			/*Format 2 = Bomb Jack*/
			else if (format == 2)
			{
				command[0] = romData[seqPos - bankAmt];
				command[1] = romData[seqPos - bankAmt + 1];
				command[2] = romData[seqPos - bankAmt + 2];
				command[3] = romData[seqPos - bankAmt + 3];
				if (command[0] < 0x60)
				{
					curNote = command[0];
					fprintf(txt, "Play note: %01X\n", curNote);
					seqPos++;
				}

				else if (command[0] == 0x60)
				{
					transpose = (signed char)command[1];
					fprintf(txt, "Enable 'tom tom' mode, transpose: %i\n", transpose);
					seqPos += 2;
				}

				else if (command[0] == 0x63)
				{
					fprintf(txt, "Hold note for a bar\n");
					seqPos++;
				}

				else if (command[0] == 0x64)
				{
					fprintf(txt, "No note/rest for a bar\n");
					seqPos++;
				}

				else if (command[0] == 0x68)
				{
					fprintf(txt, "End of sequence\n");
					seqEnd = 1;
				}

				else if (command[0] == 0x6A)
				{
					fprintf(txt, "Turn on channel?\n");
					seqPos++;
				}

				else if (command[0] == 0x6B)
				{
					c4Speed = command[1];
					c4Offset = ReadLE16(&romData[seqPos + 2 - bankAmt]);
					fprintf(txt, "Set speed control of channel 4: %01X, offset: %04X\n", c4Speed, c4Offset);
					seqPos += 4;
				}

				else if (command[0] == 0x70)
				{
					fprintf(txt, "Command 70\n");
					seqPos++;
				}

				else if (command[0] == 0x72)
				{
					fprintf(txt, "Set channel parameters?: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] == 0x75)
				{
					fprintf(txt, "Command 75: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] == 0x77)
				{
					fprintf(txt, "Set channel input on/off?: %01X\n", command[1]);
					seqPos += 2;
				}

				else if (command[0] == 0x7A)
				{
					fprintf(txt, "Switch waveform: %04X\n", ReadLE16(&romData[seqPos + 1 - bankAmt]));
					seqPos += 3;
				}

				else if (command[0] == 0x7F)
				{
					fprintf(txt, "Play tom tom note\n");
					seqPos++;
				}

				else if (command[0] == 0x89)
				{
					fprintf(txt, "Switch to instrument type 1\n");
					seqPos++;
				}

				else if (command[0] == 0x8A)
				{
					fprintf(txt, "Switch to instrument type 2\n");
					seqPos++;
				}

				else if (command[0] == 0x8B)
				{
					fprintf(txt, "Switch to instrument type 3\n");
					seqPos++;
				}

				else if (command[0] == 0x8C)
				{
					fprintf(txt, "Switch to instrument type 4\n");
					seqPos++;
				}

				else if (command[0] == 0x8D)
				{
					fprintf(txt, "Switch to instrument type 5\n");
					seqPos++;
				}

				else if (command[0] == 0x8E)
				{
					fprintf(txt, "Switch to instrument type 6\n");
					seqPos++;
				}

				else if (command[0] == 0x8F)
				{
					fprintf(txt, "Switch to instrument type 7\n");
					seqPos++;
				}

				else if (command[0] >= 0x90 && command[0] < 0xA0)
				{
					fprintf(txt, "Effect: %01X\n", command[0]);
					seqPos++;
				}

				else if (command[0] == 0xC6)
				{
					fprintf(txt, "Sweep parameters: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] == 0xC4)
				{
					fprintf(txt, "Command C4\n");
					seqPos++;
				}

				else if (command[0] > 0xC6)
				{
					curChanSpeed = command[0];
					fprintf(txt, "Change channel speed: %01X\n", curChanSpeed);
					seqPos++;
				}

				else
				{
					fprintf(txt, "Unknown command %01X\n", command[0]);
					seqPos++;
				}


			}

			/*Format 3 = Asterix*/
			else if (format == 3)
			{
				command[0] = romData[seqPos - bankAmt];
				command[1] = romData[seqPos - bankAmt + 1];
				command[2] = romData[seqPos - bankAmt + 2];
				command[3] = romData[seqPos - bankAmt + 3];
				if (command[0] < 0x60)
				{
					curNote = command[0];
					fprintf(txt, "Play note: %01X\n", curNote);
					seqPos++;
				}

				else if (command[0] == 0x63)
				{
					fprintf(txt, "Set channel parameters: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] == 0x64)
				{
					fprintf(txt, "Half rest?\n");
					seqPos++;
				}

				else if (command[0] == 0x65)
				{
					fprintf(txt, "Hold note for a bar\n");
					seqPos++;
				}

				else if (command[0] == 0x66)
				{
					fprintf(txt, "No note/rest for a bar\n");
					seqPos++;
				}

				else if (command[0] == 0x68)
				{
					fprintf(txt, "End of song\n");
					seqEnd = 1;
				}

				else if (command[0] == 0x69)
				{
					globalTranspose = (signed char)command[1];
					fprintf(txt, "Transpose (global): %i\n", globalTranspose);
					seqPos += 2;
				}

				else if (command[0] == 0x6A)
				{
					fprintf(txt, "End of sequence\n");
					seqEnd = 1;
				}

				else if (command[0] == 0x6C)
				{
					if (command[1] >= 0x9E)
					{
						fprintf(txt, "Rest for amount of time: %01X\n", command[1]);
						seqPos += 2;
					}
					else if (command[1] < 0x9E)
					{
						fprintf(txt, "Rest for amount of time: %01X, %01X\n", command[1], command[2]);
						seqPos += 3;
					}

				}

				else if (command[0] == 0x6D)
				{
					c4Speed = command[1];
					c4Offset = ReadLE16(&romData[seqPos + 2 - bankAmt]);
					fprintf(txt, "Set speed control of channel 4: %01X, offset: %04X\n", c4Speed, c4Offset);
					seqPos += 4;
				}

				else if (command[0] == 0x6E)
				{
					fprintf(txt, "Stop current drum pattern?\n");
					seqPos++;
				}

				else if (command[0] == 0x6F)
				{
					fprintf(txt, "Effect/decay option 1\n");
					seqPos++;
				}

				else if (command[0] == 0x70)
				{
					fprintf(txt, "Effect/decay option 2\n");
					seqPos++;
				}

				else if (command[0] == 0x71)
				{
					fprintf(txt, "Effect/decay option 3\n");
					seqPos++;
				}

				else if (command[0] == 0x72)
				{
					fprintf(txt, "Effect/decay option 4\n");
					seqPos++;
				}

				else if (command[0] == 0x73)
				{
					fprintf(txt, "Effect/decay option 5\n");
					seqPos++;
				}

				else if (command[0] == 0x74)
				{
					fprintf(txt, "Effect/decay option 6\n");
					seqPos++;
				}

				else if (command[0] == 0x75)
				{
					if (command[2] > 0x00)
					{
						fprintf(txt, "Set up channel parameters: %01X, %01X, %01X\n", command[1], command[2], command[3]);
						seqPos += 4;
					}
					else if (command[2] == 0x00)
					{
						fprintf(txt, "Set up channel parameters: %01X, %01X, %01X, %01X\n", command[1], command[2], command[3], romData[seqPos + 4 - bankAmt]);
						seqPos += 5;
					}
				}

				else if (command[0] == 0x76)
				{
					fprintf(txt, "Channel 3 setting 76: %01X\n", command[1]);
					seqPos += 2;
				}

				else if (command[0] == 0x77)
				{
					fprintf(txt, "Modify duty: %01X\n", command[1]);
					seqPos += 2;
				}

				else if (command[0] == 0x7A)
				{
					fprintf(txt, "Switch waveform: %04X\n", ReadLE16(&romData[seqPos + 1 - bankAmt]));
					seqPos += 3;
				}

				else if (command[0] == 0x7D)
				{
					transpose = (signed char)command[1];
					fprintf(txt, "Transpose (current channel): %i\n", transpose);
					seqPos += 2;
				}

				else if (command[0] == 0x7E)
				{
					jumpPos = ReadLE16(&romData[seqPos + 1 - bankAmt]);
					fprintf(txt, "Load routine: %04X\n", jumpPos);
					seqPos += 3;
				}

				else if (command[0] == 0x7F)
				{
					fprintf(txt, "Command 7F\n");
					seqPos++;
				}

				else if (command[0] == 0x80)
				{
					loopPt = ReadLE16(&romData[seqPos + 1 - bankAmt]);
					fprintf(txt, "Go to loop point: %04X\n", loopPt);
					seqEnd = 1;
				}

				else if (command[0] == 0x81)
				{
					jumpPos = ReadLE16(&romData[seqPos + 1 - bankAmt]);
					fprintf(txt, "Jump to position: %04X\n", jumpPos);
					seqPos += 3;
				}

				else if (command[0] == 0x82)
				{
					jumpPos = ReadLE16(&romData[seqPos + 1 - bankAmt]);
					fprintf(txt, "Load routine (v2): %04X\n", jumpPos);
					seqPos += 3;
				}

				else if (command[0] == 0x84)
				{
					fprintf(txt, "Command 84: %01X\n", command[1]);
					seqPos += 2;
				}

				else if (command[0] == 0x85)
				{
					fprintf(txt, "Switch to instrument type 1\n");
					seqPos++;
				}

				else if (command[0] == 0x86)
				{
					fprintf(txt, "Switch to instrument type 2\n");
					seqPos++;
				}

				else if (command[0] == 0x87)
				{
					fprintf(txt, "Switch to instrument type 3\n");
					seqPos++;
				}

				else if (command[0] == 0x88)
				{
					fprintf(txt, "Switch to instrument type 4\n");
					seqPos++;
				}

				else if (command[0] == 0x89)
				{
					fprintf(txt, "Switch to instrument type 5\n");
					seqPos++;
				}

				else if (command[0] == 0x8A)
				{
					fprintf(txt, "Switch to instrument type 6\n");
					seqPos++;
				}

				else if (command[0] == 0x8B)
				{
					fprintf(txt, "Switch to instrument type 7\n");
					seqPos++;
				}

				else if (command[0] == 0x8B)
				{
					fprintf(txt, "Switch to instrument type 8\n");
					seqPos++;
				}

				else if (command[0] == 0x8C)
				{
					fprintf(txt, "Switch to instrument type 9\n");
					seqPos++;
				}

				else if (command[0] == 0x8D)
				{
					fprintf(txt, "Switch to instrument type 10\n");
					seqPos++;
				}

				else if (command[0] == 0x8E)
				{
					fprintf(txt, "Switch to instrument type 11\n");
					seqPos++;
				}

				else if (command[0] == 0x9E)
				{
					fprintf(txt, "Command 9E\n");
					seqPos++;
				}

				else if (command[0] >= 0xA0 && command[0] < 0xB0)
				{
					fprintf(txt, "Effect: %01X\n", command[0]);
					seqPos++;
				}

				else if (command[0] == 0xBD)
				{
					fprintf(txt, "Command BD: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] >= 0xC0 && command[0] <= 0xCC)
				{
					fprintf(txt, "Set up waveform/volume envelope: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] > 0xCC && command[0] < 0xE9)
				{
					curChanSpeed = command[0];
					fprintf(txt, "Change channel speed: %01X\n", curChanSpeed);
					seqPos++;
				}

				else if (command[0] == 0xEA)
				{
					fprintf(txt, "Final note?\n");
					seqPos++;
				}

				else
				{
					fprintf(txt, "Unknown command %01X\n", command[0]);
					seqPos++;
				}
			}

			/*Format 6 = Pop Up*/
			else if (format == 6)
			{
				command[0] = romData[seqPos - bankAmt];
				command[1] = romData[seqPos - bankAmt + 1];
				command[2] = romData[seqPos - bankAmt + 2];
				command[3] = romData[seqPos - bankAmt + 3];
				if (command[0] < 0x60)
				{
					curNote = command[0];
					fprintf(txt, "Play note: %01X\n", curNote);
					seqPos++;
				}

				else if (command[0] == 0x6B)
				{
					fprintf(txt, "Rest for a bar\n");
					seqPos++;
				}

				else if (command[0] == 0x72)
				{
					globalTranspose = (signed char)command[1];
					fprintf(txt, "Transpose (global): %i\n", globalTranspose);
					seqPos += 2;
				}

				else if (command[0] == 0x73)
				{
					fprintf(txt, "End of sequence\n");
					seqEnd = 1;
				}

				else if (command[0] == 0x76)
				{
					fprintf(txt, "Rest for amount of time: %01X\n", command[1]);
					seqPos += 2;
				}

				else if (command[0] == 0x77)
				{
					fprintf(txt, "Hold note for a bar\n");
					seqPos++;
				}

				else if (command[0] == 0x79)
				{
					fprintf(txt, "Command 79\n");
					seqPos++;
				}

				else if (command[0] == 0x7B)
				{
					fprintf(txt, "Command 7B\n");
					seqPos++;
				}

				else if (command[0] == 0x82)
				{
					transpose = (signed char)command[1];
					fprintf(txt, "Enable 'tom tom' mode (v2)\n");
					seqPos++;
				}

				else if (command[0] == 0x83)
				{
					transpose = (signed char)command[1];
					fprintf(txt, "Enable 'tom tom' mode\n");
					seqPos++;
				}

				else if (command[0] == 0x85)
				{
					fprintf(txt, "Volume/envelope setting: %01X\n", command[0]);
					seqPos++;
				}

				else if (command[0] == 0x8F)
				{
					fprintf(txt, "Volume/envelope setting: %01X\n", command[0]);
					seqPos++;
				}

				else if (command[0] == 0x8B)
				{
					fprintf(txt, "Disable 'tom tom' mode?\n");
					seqPos++;
				}

				else if (command[0] == 0x8D)
				{
					fprintf(txt, "Command 8D\n");
					seqPos++;
				}

				else if (command[0] >= 0x90 && command[0] < 0xA0)
				{
					fprintf(txt, "Effect: %01X\n", command[0]);
					seqPos++;
				}

				else if (command[0] >= 0xE0)
				{
					curChanSpeed = command[0];
					fprintf(txt, "Change channel speed: %01X\n", curChanSpeed);
					seqPos++;
				}

				else
				{
					fprintf(txt, "Unknown command %01X\n", command[0]);
					seqPos++;
				}
			}




		}

		fprintf(txt, "\n");

	}

}