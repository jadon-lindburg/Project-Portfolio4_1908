#pragma once

struct S_OBJ_VERT
{
	float pos[3];
	float tex[3];
	float norm[3];
};

struct S_OBJ_DATA
{
	S_OBJ_VERT* vertices;
	unsigned int numVerts;
	unsigned int* indices;
	unsigned int numInds;
};

S_OBJ_DATA LoadOBJData(const char*);