#include "OBJDataLoader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
//#include <algorithm>

struct float3
{
	float x;
	float y;
	float z;
};
struct uint3
{
	unsigned int x;
	unsigned int y;
	unsigned int z;
};
struct S_OBJ_FACE
{
	uint3 verts[3];
};

S_OBJ_DATA LoadOBJData(const char* _filepath)
{
	S_OBJ_DATA data = {};
	std::vector<float3> posList;
	std::vector<float3> texList;
	std::vector<float3> normList;
	std::vector<S_OBJ_FACE> faceList;

	std::ifstream fin(_filepath, std::ios::in);
	char readLineInto[100];
	// ----- PROCESS RAW DATA INTO LISTS -----
	// verify that file opened
	if (!fin.is_open())
		_RPTN(0, "Could not open OBJ file to load data\n", NULL);
	// read data from file
	while (true)
	{
		// read next line
		fin.getline(readLineInto, 100, '\n');
		if (fin.eof())
			break;
		// copy line into separate string for strtok
		char extractVertsFrom[100];
		strcpy_s(extractVertsFrom, readLineInto);
		char separators[] = " ";
		char *token, *next_token;
		std::vector<char*> tokens;
		token = strtok_s(extractVertsFrom, separators, &next_token);
		// check if line contains vertex data
		switch (readLineInto[0])
		{
		case 'v': // vert data
		{
			// split line into tokens (first token is line identifier and is ignored)
			while (token != NULL)
			{
				tokens.push_back(token);
				token = strtok_s(NULL, separators, &next_token);
			}
			// convert tokens into vert data
			switch (tokens[0][1])
			{
			case 't': // texcoord
				float3 tex;
				tex.x = strtof(tokens[1], nullptr);
				tex.y = strtof(tokens[2], nullptr);
				if (tokens.size() > 3)
					tex.z = strtof(tokens[3], nullptr);
				else
					tex.z = 0.0f;
				texList.push_back(tex);
				break;
			case 'n': // normal
				float3 norm;
				norm.x = strtof(tokens[1], nullptr);
				norm.y = strtof(tokens[2], nullptr);
				norm.z = strtof(tokens[3], nullptr);
				normList.push_back(norm);
				break;
			default: // position
				float3 pos;
				pos.x = strtof(tokens[1], nullptr);
				pos.y = strtof(tokens[2], nullptr);
				pos.z = strtof(tokens[3], nullptr);
				posList.push_back(pos);
				break;
			}
			break;
		}
		case 'f': // face data
		{
			// split line into tokens
			while (token != NULL)
			{
				tokens.push_back(token);
				token = strtok_s(NULL, separators, &next_token);
			}
			std::vector<uint3> tokenVerts;
			char faceSeparators[] = "/";
			// convert tokens into abstract verts
			for (unsigned int i = 1; i < tokens.size(); i++)
			{
				// copy token to split further
				char extractFacesFrom[100];
				strcpy_s(extractFacesFrom, tokens[i]);
				// split token into individual values
				std::vector<char*> faceTokens;
				token = strtok_s(extractFacesFrom, faceSeparators, &next_token);
				while (token != NULL)
				{
					faceTokens.push_back(token);
					token = strtok_s(NULL, faceSeparators, &next_token);
				}
				// store values and add to list (values are 1-based, subtract 1 from each to make them 0-based)
				uint3 vert;
				vert.x = strtoul(faceTokens[0], nullptr, 10) - 1;
				vert.y = strtoul(faceTokens[1], nullptr, 10) - 1;
				vert.z = strtoul(faceTokens[2], nullptr, 10) - 1;
				tokenVerts.push_back(vert);
			}
			// assemble face from abstract vert list, dividing quads into triangles if necessary
			S_OBJ_FACE face = {};
			face.verts[0] = tokenVerts[0];
			face.verts[1] = tokenVerts[1];
			face.verts[2] = tokenVerts[2];
			faceList.push_back(face);
			if (tokens.size() > 4)
			{
				face.verts[0] = tokenVerts[2];
				face.verts[1] = tokenVerts[3];
				face.verts[2] = tokenVerts[0];
				faceList.push_back(face);
			}
			break;
		}
		default:
			break;
		}
	}
	fin.close();
	// ----- PROCESS RAW DATA INTO LISTS -----
	// ----- CONVERT LISTS INTO VERT / IND DATA -----
	std::vector<uint3> vertDataNumList;
	std::vector<unsigned int> indexList;
	// iterate through faces
	for (unsigned int i = 0; i < faceList.size(); i++)
	{
		// iterate through face's verts
		for (unsigned int j = 0; j < 3; j++)
		{
			bool unique = true;
			unsigned int index = 0;
			// iterate through vert list
			for (unsigned int k = 0; k < vertDataNumList.size(); k++)
			{
				// check vert values against current list item values
				if (faceList[i].verts[j].x == vertDataNumList[k].x
					&& faceList[i].verts[j].y == vertDataNumList[k].y
					&& faceList[i].verts[j].z == vertDataNumList[k].z)
				{
					// vert is duplicate, store index where it was found
					unique = false;
					index = k;
					break;
				}
			}
			// if vert is unique, add to list and store index where it was added
			if (unique)
			{
				index = vertDataNumList.size();
				vertDataNumList.push_back(faceList[i].verts[j]);
			}
			// add index of vert to list
			indexList.push_back(index);
		}
	}
	// ----- CONVERT LISTS INTO VERT / IND DATA -----
	// ----- CONVERT VERT DATA INTO VERTS AND COPY INTO ARRAY -----
	S_OBJ_VERT* verts = new S_OBJ_VERT[vertDataNumList.size()];
	for (unsigned int i = 0; i < vertDataNumList.size(); i++)
	{
		S_OBJ_VERT vert = {};
		vert.pos[0] = posList[vertDataNumList[i].x].x;
		vert.pos[1] = posList[vertDataNumList[i].x].y;
		vert.pos[2] = posList[vertDataNumList[i].x].z;
		vert.tex[0] = texList[vertDataNumList[i].y].x;
		vert.tex[1] = texList[vertDataNumList[i].y].y;
		vert.tex[2] = texList[vertDataNumList[i].y].z;
		vert.norm[0] = normList[vertDataNumList[i].z].x;
		vert.norm[1] = normList[vertDataNumList[i].z].y;
		vert.norm[2] = normList[vertDataNumList[i].z].z;
		verts[i] = vert;
	}
	// ----- CONVERT VERT DATA INTO VERTS AND COPY INTO ARRAY -----
	// ----- COPY INDEX LIST INTO ARRAY -----
	unsigned int* inds = new unsigned int[indexList.size()];
	for (unsigned int i = 0; i < indexList.size(); i++)
	{
		inds[i] = indexList[i];
	}
	// ----- COPY INDEX LIST INTO ARRAY -----

	data.vertices = verts;
	data.numVerts = vertDataNumList.size();
	data.indices = inds;
	data.numInds = indexList.size();
	return data;
}