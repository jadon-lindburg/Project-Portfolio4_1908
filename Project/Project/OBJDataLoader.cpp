#include "OBJDataLoader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
//#include <algorithm>

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
typedef struct _uint3_
{
	union
	{
		struct
		{
			unsigned int x;
			unsigned int y;
			unsigned int z;
		};
		unsigned int xyz[3];
	};
} uint3;
struct S_OBJ_FACE
{
	uint3 verts[3];
};

S_OBJ_DATA LoadOBJData(const char* _filepath)
{
	S_OBJ_DATA data = {};
	unsigned int numPos = 0;
	unsigned int numTex = 0;
	unsigned int numNorm = 0;
	unsigned int numFace = 0;

	std::ifstream fin(_filepath, std::ios::in);
	char charStorage[100];
	std::vector<float3> posList;
	std::vector<float2> texList;
	std::vector<float3> normList;
	std::vector<S_OBJ_FACE> faceList;

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
		char extractFrom[100];
		strcpy_s(extractFrom, charStorage);
		char separators[] = " ";
		char *token, *next_token;
		std::vector<char*> tokens;
		token = strtok_s(extractFrom, separators, &next_token);
		// check if line contains vertex data
		switch (charStorage[0])
		{
		case 'v':
		{
			while (token != NULL)
			{
				tokens.push_back(token);
				token = strtok_s(NULL, separators, &next_token);
			}
			switch (tokens[0][1])
			{
			case 't': // texcoord
				float2 tex;
				tex.x = strtof(tokens[1], nullptr);
				tex.y = strtof(tokens[2], nullptr);
				texList.push_back(tex);
				numTex++;
				break;
			case 'n': // normal
				float3 norm;
				norm.x = strtof(tokens[1], nullptr);
				norm.y = strtof(tokens[2], nullptr);
				norm.z = strtof(tokens[3], nullptr);
				normList.push_back(norm);
				numNorm++;
				break;
			default: // position
				float3 pos;
				pos.x = strtof(tokens[1], nullptr);
				pos.y = strtof(tokens[2], nullptr);
				pos.z = strtof(tokens[3], nullptr);
				posList.push_back(pos);
				numPos++;
				break;
			}
			break;
		}
		case 'f':
		{
			while (token != NULL)
			{
				tokens.push_back(token);
				token = strtok_s(NULL, separators, &next_token);
			}
			std::vector<uint3> verts;
			char faceSeparators[] = "/";
			std::vector<char*> faceTokens;
			for (unsigned int i = 1; i < tokens.size(); i++)
			{
				strcpy_s(extractFrom, tokens[i]);
				token = strtok_s(extractFrom, faceSeparators, &next_token);
				while (token != NULL)
				{
					faceTokens.push_back(token);
					token = strtok_s(NULL, faceSeparators, &next_token);
				}
				uint3 vert;
				vert.x = strtoul(tokens[0], nullptr, 10);
				vert.y = strtoul(tokens[1], nullptr, 10);
				vert.z = strtoul(tokens[2], nullptr, 10);
				verts.push_back(vert);
			}
			S_OBJ_FACE face;
			uint3 faceVerts[3] = {};
			faceVerts[0] = verts[0];
			faceVerts[1] = verts[1];
			faceVerts[2] = verts[2];
			faceList.push_back(face);
			numFace++;
			if (tokens.size() > 4)
			{
				faceVerts[0] = verts[2];
				faceVerts[1] = verts[3];
				faceVerts[2] = verts[1];
				faceList.push_back(face);
				numFace++;
			}
			break;
		}
		default:
			break;
		}
	}
	fin.close();


	return data;
}