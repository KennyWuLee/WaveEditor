/*******************************/
/*
/*  Project Name: Wave Goodbye
/*  Description: edit .wav files
/*  File names: wave.c wave.h
/*  Date: 03/28/14
/*  Authors: Tim Miranda, Till Krischer
/*
/*******************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
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

/*
       Description: This function reads in two integers and converts them to a short

       Parameters:  s     - pointer to short, that will be set to the result

       Returns:	    0     - in case the reading failes
       				1 	  - if everything went fine
*/
int readShort(short* s)
{
	int c;  //to store char as int to check against EOF
	unsigned char a[2];  // char array to read the chars into
	//read two chars into array a
	int i;
	for(i = 0; i < 2; ++i)
	{
		//if it hits EOF return 0
		if((c = getchar()) == EOF)
			return 0;
		a[i] = c;
	}
	//cast char array with length two into a short pointer, store the value it points to in *s
	*s = *(short*)a;
	//return 1 if everything went fine
	return 1;
}

/*
       Description: This function prints out a short as two characters

       Parameters:  s     - short to print
*/
void printShort(short s)
{
	//cast the shor to a unsigned char array
	unsigned char* a;
	a = (unsigned char*)&s;
	//printout the two chars
	putchar(a[0]);
	putchar(a[1]);
}

/*
       Description: This function prints two short arrays (the data part of the .wav file)

       Parameters:  left    - first array to print
    				right   - second array to print
    				length  - length of the two arrays
*/
void printData(short* left, short* right, int length)
{
	//loop through the two arrays
	int i;
	for(i = 0; i < length; ++i)
	{
		//printout one short of left and right each
		printShort(left[i]);
		printShort(right[i]);
	}
}

/*
       Description: This function reads shorts and stores them into the two arrays left and right
       				It always reads one short into left, then one into right, then again into left, and so on ...

       Parameters:  left   - first array to read into
       				rigth  - second array to read into
       				length - length of the two arrays

       Returns:	    0     - in case the input does not have the expected size
       				1 	  - if everything went fine
*/
int readData(short* left, short* right, int length)
{
	short a, b;  //two shorts to hold the values for left and right
	//loop through the arrays to fill them with shorts
	int i;
	for(i = 0; i < length; ++i)
	{
		//read a short into a and b, if that works store them into left and right
		//if it fails return 0
		if(readShort(&a) && readShort(&b))
		{
			left[i] = a;
			right[i] = b;
		}
		else
			return 0;
	}

	//return 1 if everything went fine
	return 1;
}

/*
	Description:	This function takes a WaveHeader reads the information and formats it into one string

	Parameters:		header pointer to the header

	Returns:	    pointer to newly allocated string contaning the formatted info
*/
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

/*
   Description: This function reverses all the elements in the array.

   Parameters:  array  - the array of shorts to be reversed
                length - the size of the array

*/
void reverse(short** array, int length)
{
	int i;
	//This loop reverses the order of the array by changing the first element to las and so on
	for(i = 0; i < length/2; ++i)
	{
		//traditional three line swap
		short temp = (*array)[i];
		(*array)[i] = (*array)[length - i - 1];
		(*array)[length - i - 1] = temp;
	}
}

/*
   Description: This function changes the speed of the sound

   Parameters:  array  - the array of shorts
                length - the size of the array
	    factor - factor by which the speed will change

   Returns:	    the new array of shorts that contains the new sound
*/
short* speed(short* array, int length, double factor)
{
	short* speed = (short*)malloc(sizeof(short)*((int)(length/factor)));
	if(! speed)
	{
		fprintf(stderr, "Program out of memory\n");
		exit(2);
	}
	int i;
	//This loop sets the ith value in speed to the i*factor value in array
	for(i = 0; i < (int)(length/factor); i++)
	{
		speed[i] = array[(int)(i*factor)];
	}
	free(array);
	return speed;
}

/*
   Description: This function flips the left audio with the right audio

   Parameters:  left   - the array of shorts corresponding to the left audio
                right  - the array of shorts corresponding to the right audio

*/
void flip(short** left, short** right)
{
	short* temp = *left;
	*left = *right;
	*right = temp;
}

/*
   Description: This function fades the audio out

   Parameters:  array  - the array of shorts
                length - the size of the array
	    		rate  - seconds that the fade out will take

*/
void fadeOut(short* array, int length, double rate)
{
	int i;
	double j = 0;
	double k = (44100*rate);
	//Checks to see if the length of audio is less than the length of the fade
	if(length < k)
	{
		j = k - length;
		//This loop scales each element so that it becomes smaller
		for(i = 0; i < length; i++)
		{
			array[i] =  (short)(array[i] * (1.0 - (j/k)) * (1 - (j/k)));
			j++;
		}
	}
	else
	{	//This loop scales each element that is in the fade range so that it becomes smaller
		for(i = length-k; i < length; i++)
		{
			array[i] =  (short)(array[i] * (1.0 - (j/k)) * (1 - (j/k)));
			j++;
		}
	}
}

/*
   Description: This function fades he audio in

   Parameters:  array  - the array of shorts
                length - the size of the array
	    rate - seconds that the fade in will take

*/
void fadeIn(short* array, int length, double rate)
{
	int i;
	double j = 0;
	double k = (44100*rate);
	//Checks to see if the length of audio is less than the length of the fade
	if(length < k)
	{	//This loop scales each element so that it becomes larger
		for(i = 0; i < length; i++)
		{
			array[i] =  (short)(array[i] * (j/k) * (j/k));
			j++;
		}
	}
	else
	{	//This loop scales each element that is in the fade range so that it becomes larger
		for(i = 0; i < k; i++)
		{
			array[i] =  (short)(array[i] * (j/k) * (j/k));
			j++;
		}
	}
}

/*
   Description: This function changes the volume of the sound

   Parameters:  array  - the array of shorts
                length - the size of the array
	    factor - the scale used to amplify the sound

*/
void volume(short* array, int length, double factor)
{
	int i;
	//This loop multiplies each element in the array by the scaling factor
	for(i = 0; i < length; ++i)
	{
		int temp = array[i] * factor;
		if(temp > SHRT_MAX) // Checks to see if temp surpasses bounds
			temp = SHRT_MAX;
		if(temp < SHRT_MIN)// Checks to see if temp surpasses bounds
			temp = SHRT_MIN;
		array[i] = (short)temp;
	}
}

/*
       Description: This function adds an echo to an array of shorts representing a sound wave

       Parameters:  array     - the array of shorts
                    length    - length of the array
                    delay     - delay of the echo as a double
                    factor	  - factor by which the echo is quieter than the original sound

       Returns:	    a pointer to a new array that contains the changes to array
*/
short* echo(short* array, int length, double delay, double factor)
{
	//calculate new length by adding delay to current length
	int newLength = length + (int)(44100 * delay);
	//allocate new array with enough room for echo
	short* temp = (short*) malloc(sizeof(short)*newLength);
	if(! temp)
	{
		fprintf(stderr, "Program out of memory\n");
		exit(2);
	}
	//copy the original data from array int temp
	int i;
	for (i = 0; i < length; ++i)
		temp[i] = array[i];

	//loop through the new array temp starting from the point where the echo starts
	//and add the echo scaled by factor to the values  in temp
	int j;
	for(i = (int)(44100 * delay), j = 0; i < newLength; ++i, ++j)
	{
		//calculate new value for element at i, by adding echo
		int sum = (temp[i] + (array[j] * factor));
		//check if the new value is out of bounds
		if(sum > SHRT_MAX)
			sum = SHRT_MAX;
		if(sum < SHRT_MIN)
			sum = SHRT_MIN;
		//set element i in temp to new value
		temp[i] = sum;
	}

	//free the old array
	free(array);
	//return new array
	return temp;
}

int main(int argc, char *argv[])
{
	WaveHeader header; //holds the header
	//if there is an error reading the header or if the header ID is not "RIFF" exit
	if( !readHeader(&header) || strncmp((char*)header.ID, "RIFF", 4))
	{
		fprintf(stderr, "File is not a RIFF file\n");
		return 3;
	}
	//check the formatChunk.ID, if not "fmt " exit
	if(strncmp((char*)header.formatChunk.ID, "fmt ", 4))
	{
		fprintf(stderr, "Format chunk is corrupted\n");
		return 4;
	}
	//check the header.dataChunk.ID, if not "data" exit
	if(strncmp((char*)header.dataChunk.ID, "data", 4))
	{
		fprintf(stderr, "Error: Data chunk is corrupted\n");
		return 5;
	}
	//check if the number of channels is 2, if not exit
	if(header.formatChunk.channels != 2)
	{
		fprintf(stderr, "File is not stereo\n");
		return 6;
	}
	//check the sampling rate, if its not 44100 exit
	if(header.formatChunk.sampleRate != 44100)
	{
		fprintf(stderr, "File does not use 44,100Hz sample rate\n");
		return 7;
	}
	//check the bits per sample, if not 16 bit (short) exit
	if(header.formatChunk.bitsPerSample != 16)
	{
		fprintf(stderr, "File does not have 16-bit samples\n");
		return 8;
	}

	//channelLenth is the size of the dataChunk divided by two for two channels
	//and again by two because one sample has two bytes
	int channelLength = header.dataChunk.size/4;
	//allocate two arrays for the two channels
	short* left = (short*) malloc(sizeof(short)*channelLength);
	short* right = (short*) malloc(sizeof(short)*channelLength);
	//if malloc fails (returns NULL pointer) exit
	if(! (left && right))
	{
		fprintf(stderr, "Program out of memory\n");
		return 2;
	}
	//read the data Chunk
	//if its size does not match the expected length exit
	if(! readData(left, right, channelLength))
	{
		fprintf(stderr, "File size does not match size in header\n");
		return 9;
	}

	//get formatted header info
	char* info = headerInfo(&header);
	//printout input header information to stderr
	fprintf(stderr, "\nInput Wave Header Information\n\n%s\n", info);
	free(info);

	//loops throuh the commandline arguments and executes the right action on the data stored in left and right
	int i;
	for(i = 1; i < argc; ++i)
	{
		//case "-r" reverse
		if(strcmp(argv[i], "-r") == 0)
		{
			//reverse both the left and right channel
			reverse(&left, channelLength);
			reverse(&right, channelLength);
		}
		//case "-s" speed
		else if(strcmp(argv[i], "-s") == 0)
		{
			//check if there is a positive number in the argument after "-s" and read it into factor,
			//if not exit
			double factor;
			if(i < argc - 1 && sscanf(argv[++i], "%lf", &factor) != EOF && factor > 0)
			{
				//speed up left and right channel
				left = speed(left, channelLength, factor);
				right = speed(right, channelLength, factor);
				//update the channelLength
				channelLength /= factor;
				//update the header data
				header.dataChunk.size = 4*channelLength;
				header.size = 4 * channelLength + 36;
			}
			else
			{
				fprintf(stderr, "Error: A positive number must be supplied for the speed change\n");
				return 10;
			}
		}
		//case "-f" flip channels
		else if(strcmp(argv[i], "-f") == 0)
			//flip channels left and right
			flip(&left, &right);
		//case "-o" fade out
		else if(strcmp(argv[i], "-o") == 0)
		{
			//check if there is a positive number in the argument after "-s" and read it into seconds,
			//if not exit
			double seconds;
			if(i < argc - 1 && sscanf(argv[++i], "%lf", &seconds) != EOF && seconds > 0)
			{
				//fade out left and right channel
				fadeOut(left, channelLength, seconds);
				fadeOut(right, channelLength, seconds);
			}
			else
			{
				fprintf(stderr, "Error: A positive number must be supplied for the fade in and fade out time\n");
				return 11;
			}
		}
		//case "-i" fade in
		else if(strcmp(argv[i], "-i") == 0)
		{
			//check if there is a positive number in the argument after "-s" and read it into seconds,
			//if not exit
			double seconds;
			if(i < argc - 1 && sscanf(argv[++i], "%lf", &seconds) != EOF && seconds > 0)
			{
				//fade in left and right channel
				fadeIn(left, channelLength, seconds);
				fadeIn(right, channelLength, seconds);
			}
			else
			{
				fprintf(stderr, "Error: A positive number must be supplied for the fade in and fade out time\n");
				return 11;
			}
		}
		//case "-v" volume
		else if(strcmp(argv[i], "-v") == 0)
		{
			//check if there is a positive number in the argument after "-s" and read it into scale,
			//if not exit
			double scale;
			if(i < argc - 1 && sscanf(argv[++i], "%lf", &scale) != EOF && scale > 0)
			{
				//change the volume of left and right
				volume(left, channelLength, scale);
				volume(right, channelLength, scale);
			}
			else
			{
				fprintf(stderr, "Error: A positive number must be supplied for the volume scale\n");
				return 12;
			}
		}
		//case "-e" echo
		else if(strcmp(argv[i], "-e") == 0)
		{
			//check if there are two positve numbers in the arguments after "-e" and read them into delay and factor,
			//if not exit
			double delay, factor;
			if(i < argc - 2 && sscanf(argv[++i], "%lf", &delay) != EOF && sscanf(argv[++i], "%lf", &factor) != EOF && delay > 0 && factor > 0)
			{
				//ad the echo to the left and right channel
				left = echo(left, channelLength, delay, factor);
				right = echo(right, channelLength, delay, factor);
				//update channelLength
				channelLength += (int)(44100 * delay);
				//update header data
				header.dataChunk.size = 4*channelLength;
				header.size = 4 * channelLength + 36;
			}
			else
			{
				fprintf(stderr, "Error: A positive number must be supplied for the fade in and fade out time\n");
				return 13;
			}
		}
		//if the argument is anything else then the known commands print a help
		else
		{
			fprintf(stderr, "Usage: wave [[-r][-s factor][-f][-o delay][-i delay][-v scale][-e delay scale] < input > output\n");
			return 1;
		}
	}

	//get formatted header info
	info = headerInfo(&header);
	//print output header info the stderr
	fprintf(stderr, "\nOutput Wave Header Information\n\n%s\n", info);
	free(info);

	//write the header to stdout
	writeHeader(&header);
	//wrinte the data to stdout
	printData(left, right, channelLength);

	//free the two arrays holding the channels
	free(left);
	free(right);

	return 0;
}

short* echo(short* array, int length, double delay, double factor)
{
	int newLength = length + (int)(44100 * delay);
	short* temp = (short*) malloc(sizeof(short)*newLength);
	int i;
	for (i = 0; i < length; ++i)
		temp[i] = array[i];
	int j;
	for(i = (int)(44100 * delay), j = 0; i < newLength; ++i, ++j)
	{
		int sum = (temp[i] + (array[j] * factor));
		if(sum > SHRT_MAX)
			sum = SHRT_MAX;
		if(sum < SHRT_MIN)
			sum = SHRT_MIN;
		temp[i] = sum;
	}
	free(array);
	return temp;
}
