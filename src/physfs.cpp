#include <streambuf>
#include <string>
#include <string.h>
#include "physfs.hpp"

using namespace PhysFS;
using std::streambuf;
using std::streamsize;
using std::streamoff;
using std::streampos;
using std::ios_base;

class fbuf : public streambuf {
private:
	fbuf(const fbuf & other);
	fbuf& operator=(const fbuf& other);

	int_type underflow() {
		if (PHYSFS_eof(file)) {
			return traits_type::eof();
		}
		std::size_t bytesRead = PHYSFS_read(file, buffer, egptr() - buffer, 1);
		if (bytesRead < 1) {
			return traits_type::eof();
		}
		setg(buffer, buffer, buffer + bytesRead);
		return *gptr();
	}

	streampos seekoff(streamoff pos, ios_base::seekdir dir, ios_base::open_mode mode) {
		switch (dir) {
		case std::ios_base::beg:
			PHYSFS_seek(file, pos);
			break;
		case std::ios_base::cur:
			PHYSFS_seek(file, PHYSFS_tell(file) + pos);
			break;
		case std::ios_base::end:
			PHYSFS_seek(file, PHYSFS_seek(file, PHYSFS_fileLength(file) + pos));
			break;
		}
		if (mode == std::ios_base::in) {
			setg(egptr(), egptr(), egptr());
		}
		else {
			setp(buffer, buffer);
		}
		return PHYSFS_tell(file);
	}

	std::streampos seekpos(streampos pos, std::ios_base::open_mode mode) {
		PHYSFS_seek(file, pos);
		if (mode == std::ios_base::in) {
			setg(egptr(), egptr(), egptr());
		}
		else {
			setp(buffer, buffer);
		}
		return PHYSFS_tell(file);
	}

	int_type overflow( int_type c = traits_type::eof() ) {
		if (pptr() == pbase() && c == traits_type::eof()) {
			return 0; // no-op
		}
		if (PHYSFS_write(file, pbase(), pptr() - pbase(), 1) < 1) {
			return traits_type::eof();
		}
		if (c != traits_type::eof()) {
			if (PHYSFS_write(file, &c, 1, 1) < 1) {
				return traits_type::eof();
			}
		}

		return 0;
	}

	int sync() {
		return overflow();
	}

	char * buffer;
protected:
	PHYSFS_File * const file;
public:
	fbuf(PHYSFS_File * file, std::size_t bufferSize = 256) : file(file) {
		buffer = new char[bufferSize];
		char * end = buffer + bufferSize;
		setg(end, end, end);
		setp(buffer, end);
	}

	~fbuf() {
		sync();
		delete buffer;
	}
};

fstream::fstream(PHYSFS_File* file) : file(file) {}

fstream::~fstream() {
	PHYSFS_close(file);
}

bool fstream::eof() {
	return PHYSFS_eof(file);
}

PhysFS::size_t fstream::length() {
	return PHYSFS_fileLength(file);
}

ifstream::ifstream(PHYSFS_File * file) : fstream(file), std::istream(new fbuf(file)) {}

ifstream::~ifstream() {
	delete rdbuf();
}

ofstream::ofstream(PHYSFS_File * file) : fstream(file), std::ostream(new fbuf(file)) {}

ofstream::~ofstream() {
	delete rdbuf();
}

Version getLinkedVersion() {
	Version version;
	PHYSFS_getLinkedVersion(&version);
	return version;
}

void init(const char* argv0) {
	PHYSFS_init(argv0);
}

void deinit() {
	PHYSFS_deinit();
}

ArchiveInfoList supportedArchiveTypes() {
	ArchiveInfoList list;
	for (const ArchiveInfo** archiveType = PHYSFS_supportedArchiveTypes(); *archiveType != NULL; archiveType++) {
		list.push_back(**archiveType);
	}
	return list;
}

string getDirSeparator() {
	return PHYSFS_getDirSeparator();
}

void permitSymbolicLinks(bool allow) {
	PHYSFS_permitSymbolicLinks(allow);
}

StringList getCdRomDirs() {
	StringList dirs;
	char ** dirBegin = PHYSFS_getCdRomDirs();
	for (char ** dir = dirBegin; *dir != NULL; dir++) {
		dirs.push_back(*dir);
	}
	PHYSFS_freeList(dirBegin);
	return dirs;
}

void getCdRomDirs(StringCallback callback, void * extra) {
	PHYSFS_getCdRomDirsCallback(callback, extra);
}

string getBaseDir() {
	return PHYSFS_getBaseDir();
}

string getUserDir() {
	return PHYSFS_getUserDir();
}

string getWriteDir() {
	return PHYSFS_getWriteDir();
}

void setWriteDir(const string& newDir) {
	PHYSFS_setWriteDir(newDir.c_str());
}

void removeFromSearchPath(const string& oldDir) {
	PHYSFS_removeFromSearchPath(oldDir.c_str());
}

StringList getSearchPath() {
	StringList pathList;
	char ** pathBegin = PHYSFS_getSearchPath();
	for (char ** path = pathBegin; *path != NULL; path++) {
		pathList.push_back(*path);
	}
	PHYSFS_freeList(pathBegin);
	return pathList;
}

void getSearchPath(StringCallback callback, void * extra) {
	PHYSFS_getSearchPathCallback(callback, extra);
}

void setSaneConfig(const string& orgName, const string& appName,
		const string& archiveExt, bool includeCdRoms, bool archivesFirst) {
	PHYSFS_setSaneConfig(orgName.c_str(), appName.c_str(), archiveExt.c_str(), includeCdRoms, archivesFirst);
}

void mkdir(const string& dirName) {
	PHYSFS_mkdir(dirName.c_str());
}

void deleteFile(const string& filename) {
	PHYSFS_delete(filename.c_str());
}

string getRealDir(const string& filename) {
	return PHYSFS_getRealDir(filename.c_str());
}

StringList enumerateFiles(const string& directory) {
	StringList files;
	char ** listBegin = PHYSFS_enumerateFiles(directory.c_str());
	for (char ** file = listBegin; *file != NULL; file++) {
		files.push_back(*file);
	}
	PHYSFS_freeList(listBegin);
	return files;
}

void enumerateFiles(const string& directory, EnumFilesCallback callback, void * extra) {
	PHYSFS_enumerateFilesCallback(directory.c_str(), callback, extra);
}

bool exists(const string& filename) {
	return PHYSFS_exists(filename.c_str());
}

bool isDirectory(const string& filename) {
	return PHYSFS_isDirectory(filename.c_str());
}

bool isSymbolicLink(const string& filename) {
	return PHYSFS_isSymbolicLink(filename.c_str());
}

sint64 getLastModTime(const string& filename) {
	return PHYSFS_getLastModTime(filename.c_str());
}

ofstream openWrite(const string& filename) {
	PHYSFS_File * file = PHYSFS_openWrite(filename.c_str());
	return ofstream(file);
}

ofstream openAppend(const string& filename) {
	PHYSFS_File * file = PHYSFS_openAppend(filename.c_str());
	return ofstream(file);
}

ifstream openRead(const string& filename) {
	PHYSFS_File * file = PHYSFS_openRead(filename.c_str());
	return ifstream(file);
}

bool isInit() {
	return PHYSFS_isInit();
}

bool symbolicLinksPermitted() {
	return PHYSFS_symbolicLinksPermitted();
}

void setAllocator(const Allocator* allocator) {
	PHYSFS_setAllocator(allocator);
}

void mount(const string& newDir, const string& mountPoint, bool appendToPath) {
	PHYSFS_mount(newDir.c_str(), mountPoint.c_str(), appendToPath);
}

string getMountPoint(const string& dir) {
	return PHYSFS_getMountPoint(dir.c_str());
}

sint16 Util::swapSLE16(sint16 value) {
	return PHYSFS_swapSLE16(value);
}

uint16 Util::swapULE16(uint16 value) {
	return PHYSFS_swapULE16(value);
}

sint32 Util::swapSLE32(sint32 value) {
	return PHYSFS_swapSLE32(value);
}

uint32 Util::swapULE32(uint32 value) {
	return PHYSFS_swapULE32(value);
}

sint64 Util::swapSLE64(sint64 value) {
	return PHYSFS_swapSLE64(value);
}

uint64 Util::swapULE64(uint64 value) {
	return PHYSFS_swapULE64(value);
}

sint16 Util::swapSBE16(sint16 value) {
	return PHYSFS_swapSBE16(value);
}

uint16 Util::swapUBE16(uint16 value) {
	return PHYSFS_swapUBE16(value);
}

sint32 Util::swapSBE32(sint32 value) {
	return PHYSFS_swapSBE32(value);
}

uint32 Util::swapUBE32(uint32 value) {
	return PHYSFS_swapUBE32(value);
}

sint64 Util::swapSBE64(sint64 value) {
	return PHYSFS_swapSBE64(value);
}

uint64 Util::swapUBE64(uint64 value) {
	return PHYSFS_swapUBE64(value);
}

string Util::utf8FromUcs4(const uint32* src) {
	string value;
	std::size_t length = strlen((char*) src);
	char * buffer = new char[length]; // will be smaller than len
	PHYSFS_utf8FromUcs4(src, buffer, length);
	value.append(buffer);
	return value;
}

string Util::utf8ToUcs4(const char* src) {
	string value;
	std::size_t length = strlen(src) * 4;
	char * buffer = new char[length]; // will be smaller than len
	PHYSFS_utf8ToUcs4(src, (uint32*) buffer, length);
	value.append(buffer);
	return value;
}

string Util::utf8FromUcs2(const uint16* src) {
	string value;
	std::size_t length = strlen((char*) src);
	char * buffer = new char[length]; // will be smaller than len
	PHYSFS_utf8FromUcs2(src, buffer, length);
	value.append(buffer);
	return value;
}

string Util::utf8ToUcs2(const char* src) {
	string value;
	std::size_t length = strlen(src) * 2;
	char * buffer = new char[length]; // will be smaller than len
	PHYSFS_utf8ToUcs2(src, (uint16*) buffer, length);
	value.append(buffer);
	return value;
}

string Util::utf8FromLatin1(const char* src) {
	string value;
	std::size_t length = strlen((char*) src) * 2;
	char * buffer = new char[length]; // will be smaller than len
	PHYSFS_utf8FromLatin1(src, buffer, length);
	value.append(buffer);
	return value;
}
