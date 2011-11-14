#pragma once

namespace util {

typedef int MMapHandle;

MMapHandle mmapFile(const char* fname);
const void* mmapGetData(MMapHandle handle);
void mmapClose(MMapHandle handle);

} // namespace util
