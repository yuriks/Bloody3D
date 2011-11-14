#include "mmap.hpp"

#include <Windows.h>
#include <algorithm>
#include <cassert>

namespace util {

namespace {

struct MMapInfo {
	bool active;
	HANDLE file_handle;
	HANDLE mmap_handle;
	void* data_ptr;
};

static const int MMAP_MAX_MAPPINGS = 256;

static MMapInfo mmap_info_table[MMAP_MAX_MAPPINGS];

static bool mmapValidateHandle(MMapHandle handle) {
	if (handle < 0 || handle >= MMAP_MAX_MAPPINGS)
		return false;

	if (!mmap_info_table[handle].active)
		return false;

	return true;
}

} // namespace

MMapHandle mmapFile(const char* fname) {
	MMapInfo* info = std::find_if(std::begin(mmap_info_table), std::end(mmap_info_table), [](const MMapInfo& i) { return !i.active; });
	if (info == std::end(mmap_info_table))
		return -1;

	HANDLE file_handle = CreateFileA(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file_handle == INVALID_HANDLE_VALUE)
		goto error_file;

	HANDLE mmap_handle = CreateFileMapping(file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
	if (mmap_handle == INVALID_HANDLE_VALUE)
		goto error_mmap;

	void* data_ptr = MapViewOfFile(mmap_handle, FILE_MAP_READ, 0, 0, 0);
	if (data_ptr == nullptr)
		goto error_data;

	info->file_handle = file_handle;
	info->mmap_handle = mmap_handle;
	info->data_ptr = data_ptr;
	info->active = true;

	MMapHandle handle = info - mmap_info_table;
	assert(mmapValidateHandle(handle));
	return handle;

error_data:
	CloseHandle(mmap_handle);
error_mmap:
	CloseHandle(file_handle);
error_file:
	return -1;
}

const void* mmapGetData(MMapHandle handle) {
	assert(mmapValidateHandle(handle));
	return mmap_info_table[handle].data_ptr;
}

void mmapClose(MMapHandle handle) {
	assert(mmapValidateHandle(handle));

	MMapInfo& info = mmap_info_table[handle];

	UnmapViewOfFile(info.data_ptr);
	CloseHandle(info.mmap_handle);
	CloseHandle(info.file_handle);

	info.active = false;
}

} // namespace util
