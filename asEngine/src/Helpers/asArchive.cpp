#include "aspch.h"
#include "asArchive.h"
#include "asHelper.h"

#include <fstream>
#include <sstream>

using namespace std;

// this should always be only INCREMENTED and only if a new serialization is implemeted somewhere!
uint64_t __archiveVersion = 33;
// this is the version number of which below the archive is not compatible with the current version
uint64_t __archiveVersionBarrier = 22;

// version history is logged in ArchiveVersionHistory.txt file!

asArchive::asArchive()
{
	CreateEmpty();
}
asArchive::asArchive(const std::string& fileName, bool readMode) : fileName(fileName), readMode(readMode)
{
	if (!fileName.empty())
	{
		if (readMode)
		{
			ifstream file(fileName, ios::binary | ios::ate);
			if (file.is_open())
			{
				dataSize = (size_t)file.tellg();
				file.seekg(0, file.beg);
				DATA = new uint8_t[(size_t)dataSize];
				file.read((char*)DATA, dataSize);
				file.close();
				(*this) >> version;
				if (version < __archiveVersionBarrier)
				{
					stringstream ss("");
					ss << "The archive version (" << version << ") is no longer supported!";
					asHelper::messageBox(ss.str(), "Error!");
					Close();
				}
				if (version > __archiveVersion)
				{
					stringstream ss("");

					ss << "The archive version (" << version << ") is higher than the program's (" << __archiveVersion << ")!";
					asHelper::messageBox(ss.str(), "Error!");
					Close();
				}
			}
		}
		else
		{
			CreateEmpty();
		}
	}
}


asArchive::~asArchive()
{
	Close();
}

void asArchive::CreateEmpty()
{
	readMode = false;
	pos = 0;

	version = __archiveVersion;
	dataSize = 128; // this will grow if necessary anyway...
	DATA = new uint8_t[dataSize];
	(*this) << version;
}

void asArchive::SetReadModeAndResetPos(bool isReadMode)
{
	readMode = isReadMode;
	pos = 0;

	if (readMode)
	{
		(*this) >> version;
	}
	else
	{
		(*this) << version;
	}
}

bool asArchive::IsOpen()
{
	// when it is open, DATA is not null because it contains the version number at least!
	return DATA != nullptr;
}

void asArchive::Close()
{
	if (!readMode && !fileName.empty())
	{
		SaveFile(fileName);
	}
	SAFE_DELETE_ARRAY(DATA);
}

bool asArchive::SaveFile(const std::string& fileName)
{
	if (pos <= 0)
	{
		return false;
	}

	ofstream file(fileName, ios::binary | ios::trunc);
	if (file.is_open())
	{
		file.write((const char*)DATA, (streamsize)pos);
		file.close();
		return true;
	}

	return false;
}

string asArchive::GetSourceDirectory() const
{
	return asHelper::GetDirectoryFromPath(fileName);
}

string asArchive::GetSourceFileName() const
{
	return fileName;
}
