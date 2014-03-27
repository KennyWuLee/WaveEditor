#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wave.h"

int readHeader( WaveHeader* header )
{
	if( fread( header, sizeof( WaveHeader ), 1, stdin) != 1 )
		return 0;

	return 1;
}

int writeHeader( const WaveHeader* header )
{
	if( fwrite( header, sizeof( WaveHeader ), 1, stdout) != 1 )
		return 0;

	return 1;
}

int readShort(short* s)
{
	int c;
	unsigned char a[2];
	int i;
	for(i = 0; i < 2; ++i)
	{
		if((c = getchar()) == EOF)
			return 0;
		a[i] = c;
	}
	*s = *(short*)a;
	return 1;
}

void printShort(short s)
{
	unsigned char* a;
	a = (unsigned char*)&s;
	putchar(a[0]);
	putchar(a[1]);
}

void printData(short* left, short* right, int length)
{
	int i;
	for(i = 0; i < length; ++i)
	{
		printShort(left[i]);
		printShort(right[i]);
	}
}

int readData(short* left, short* right, int length)
{
	short a, b;
	int i;
	for(i = 0; i < length; ++i)
	{
		if(readShort(&a) && readShort(&b))
		{
			left[i] = a;
			right[i] = b;
		}
		else
			return 0;
	}
	if(getchar() != EOF)
		return 0;

	return 1;
}

char* headerInfo(WaveHeader* header)
{
	char info[260];
	sprintf(info,                "ID:\t\t\t%.4s\n",					 header->ID);
	sprintf(info + strlen(info), "Size:\t\t\t%d\n",					 header->size);
	sprintf(info + strlen(info), "Format:\t\t\t%.4s\n",				 header->format);
	sprintf(info + strlen(info), "Format ID:\t\t%.4s\n",			 header->formatChunk.ID);
	sprintf(info + strlen(info), "Format Size:\t\t%-3d(PCM = 16)\n", header->formatChunk.size);
	sprintf(info + strlen(info), "Compression:\t\t%-3d(None = 1)\n", header->formatChunk.compression);
	sprintf(info + strlen(info), "Channels:\t\t%-3d(Stereo = 2)\n",	 header->formatChunk.channels);
	sprintf(info + strlen(info), "Sample Rate:\t\t%d\n",			 header->formatChunk.sampleRate);
	sprintf(info + strlen(info), "Byte Rate:\t\t%d\n",				 header->formatChunk.byteRate);
	sprintf(info + strlen(info), "Block Align:\t\t%d\n",			 header->formatChunk.blockAlign);
	sprintf(info + strlen(info), "Bits Per Sample:\t%d\n",			 header->formatChunk.bitsPerSample);
	sprintf(info + strlen(info), "Data ID:\t\t%.4s\n",				 header->dataChunk.ID);
	sprintf(info + strlen(info), "Data Size:\t\t%d\n",				 header->dataChunk.size);
	return strdup(info);
}

void flip(short** left, short** right)
{
	short* temp = *left;
	*left = *right;
	*right = temp;
}

int main(int argc, char *argv[])
{
	WaveHeader header;
	if( !readHeader(&header) || strncmp((char*)header.ID, "RIFF", 4))
	{
		fprintf(stderr, "File is not a RIFF file\n");
		return 3;
	}
	if(strncmp((char*)header.formatChunk.ID, "fmt ", 4))
	{
		fprintf(stderr, "Format chunk is corrupted\n");
		return 4;
	}
	if(strncmp((char*)header.dataChunk.ID, "data", 4))
	{
		fprintf(stderr, "Error: Data chunk is corrupted\n");
		return 5;
	}
	if(header.formatChunk.channels != 2)
	{
		fprintf(stderr, "File is not stereo\n");
		return 6;
	}
	if(header.formatChunk.sampleRate != 44100)
	{
		fprintf(stderr, "File does not use 44,100Hz sample rate\n");
		return 7;
	}
	if(header.formatChunk.bitsPerSample != 16)
	{
		fprintf(stderr, "File does not have 16-bit samples\n");
		return 8;
	}

	int channelLength = header.dataChunk.size/2/2;
	short* left = (short*) malloc(2*channelLength);
	short* right = (short*) malloc(2*channelLength);
	if(! (left && right))
	{
		fprintf(stderr, "Program out of memory\n");
		return 2;
	}
	if(! readData(left, right, channelLength))
	{
		fprintf(stderr, "File size does not match size in header\n");
		return 9;
	}

	char* info = headerInfo(&header);
	fprintf(stderr, "\nInput Wave Header Information\n\n%s\n", info);
	free(info);

	int i;
	for(i = 1; i < argc; ++i)
	{
		if(strcmp(argv[i], "-r") == 0);

		else if(strcmp(argv[i], "-s") == 0)
		{
			double factor;
			if(i < argc - 1 && sscanf(argv[++i], "%lf", &factor) != EOF && factor > 0)
				fprintf(stderr, "positive factor: %f\n", factor);
			else
			{
				fprintf(stderr, "Error: A positive number must be supplied for the speed change\n");
				return 10;
			}
		}
		else if(strcmp(argv[i], "-f") == 0)
			flip(&left, &right);
		else if(strcmp(argv[i], "-o") == 0)
		{
			double seconds;
			if(i < argc - 1 && sscanf(argv[++i], "%lf", &seconds) != EOF && seconds > 0)
				fprintf(stderr, "positive seconds: %f\n", seconds);
			else
			{
				fprintf(stderr, "Error: A positive number must be supplied for the fade in and fade out time\n");
				return 11;
			}
		}
		else if(strcmp(argv[i], "-i") == 0)
		{
			double seconds;
			if(i < argc - 1 && sscanf(argv[++i], "%lf", &seconds) != EOF && seconds > 0)
				fprintf(stderr, "positive seconds: %f\n", seconds);
			else
			{
				fprintf(stderr, "Error: A positive number must be supplied for the fade in and fade out time\n");
				return 11;
			}
		}
		else if(strcmp(argv[i], "-v") == 0)
		{
			double scale;
			if(i < argc - 1 && sscanf(argv[++i], "%lf", &scale) != EOF && scale > 0)
				fprintf(stderr, "positive scale: %f\n", scale);
			else
			{
				fprintf(stderr, "Error: A positive number must be supplied for the volume scale\n");
				return 12;
			}
		}
		else if(strcmp(argv[i], "-e") == 0)
		{
			double delay, factor;
			if(i < argc - 2 && sscanf(argv[++i], "%lf", &delay) != EOF && sscanf(argv[++i], "%lf", &factor) != EOF && delay > 0 && factor > 0)
				fprintf(stderr, "positive delay&factor: %f %f\n", delay, factor);
			else
			{
				fprintf(stderr, "Error: A positive number must be supplied for the fade in and fade out time\n");
				return 13;
			}
		}
		else
		{
			fprintf(stderr, "Usage: wave [[-r][-s factor][-f][-o delay][-i delay][-v scale][-e delay scale] < input > output\n");
			return 1;
		}
	}

	info = headerInfo(&header);
	fprintf(stderr, "\nOutput Wave Header Information\n\n%s\n", info);
	free(info);

	writeHeader(&header);
	printData(left, right, channelLength);

	free(left);
	free(right);

	return 0;
}