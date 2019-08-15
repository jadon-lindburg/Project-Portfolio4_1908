#include "OBJDataLoader.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

typedef struct _float2_
{
	union
	{
		struct
		{
			float x;
			float y;
		};
		float xy[2];
	};
} float2;
typedef struct _float3_
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		float xyz[3];
	};
} float3;

S_OBJ_DATA LoadOBJData(const char* _filepath)
{
	S_OBJ_DATA data = {};
	unsigned int numPos = 0;
	unsigned int numTex = 0;
	unsigned int numNorm = 0;

	std::ifstream fin(_filepath, std::ios::in);
	char charStorage[100];
	std::vector<float3> posList;
	std::vector<float2> texList;
	std::vector<float3> normList;
	// verify that file opened
	if (!fin.is_open())
	{
		_RPTN(0, "Could not open OBJ file to load data\n", NULL);
	}
	// read data from file
	while (true)
	{
		// get next line
		fin.getline(charStorage, 100, '\n');
		if (fin.eof())
			break;
		// check if line contains vertex position data
		if (charStorage[0] == 'v')
		{
			switch (charStorage[1])
			{
			case ' ':
				numPos++;
				break;
			case 't':
				numTex++;
				break;
			case 'n':
				numNorm++;
				break;
			default:
				break;
			}
		}
	}
	fin.close();


	return data;
}