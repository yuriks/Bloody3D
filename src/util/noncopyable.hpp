#pragma once

#define NONCOPYABLE(cls) cls(const cls&); const cls& operator=(const cls&)
