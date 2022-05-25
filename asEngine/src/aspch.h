#pragma once

#include <cstdint>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
using namespace DirectX;
using namespace DirectX::PackedVector;
static const XMFLOAT4X4 IDENTITYMATRIX = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
typedef uint64_t asCPUHandle;
static  asCPUHandle AS_NULL_HANDLE = 0;

#define arraysize(a) (sizeof(a) / sizeof(a[0]))
#define ALIGN_16 void* operator new(size_t i){return _mm_malloc(i, 16);} void operator delete(void* p){_mm_free(p);}
#define SAFE_RELEASE(a) if((a)!=nullptr){(a)->Release();(a)=nullptr;}
#define SAFE_DELETE(a) {delete (a);(a)=nullptr;}
#define SAFE_DELETE_ARRAY(a) {delete[](a);(a)=nullptr;}
#define GFX_STRUCT struct alignas(16)
#define NOMINMAX

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cassert>

//stl
#include <string>
#include <cstring>
#include <memory>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <functional>
#include <algorithm>

#include <atomic>
#include <stack>
#include <mutex>

#include <iostream>
#include <sstream>
#include <fstream>

#ifdef AS_PLATFORM_WINDOWS
#define  NOMINMAX
#include <Windows.h>
#endif

