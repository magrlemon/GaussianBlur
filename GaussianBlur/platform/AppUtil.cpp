#include "stdafx.h"
#include "AppUtil.h"
#include <fstream>

using namespace std;

//Read the binary file 'fileName' into 'content'
bool ReadBinaryFile(wstring fileName,vector<char> &content)
{
	ifstream fxFile(fileName.c_str(),ios::binary);
	if(!fxFile)
	{
		return false;
	}

	fxFile.seekg(0,ifstream::end);
	UINT size = static_cast<UINT>(fxFile.tellg());
	fxFile.seekg(0,ifstream::beg);

	content.resize(size);
	fxFile.read(&content[0],size);

	fxFile.close();

	return true;
}