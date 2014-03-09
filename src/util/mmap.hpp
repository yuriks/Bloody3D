#pragma once

typedef int MMapHandle;

MMapHandle mmapFile(const char* fname);
const void* mmapGetData(MMapHandle handle);
void mmapClose(MMapHandle handle);
