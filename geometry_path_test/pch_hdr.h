#pragma once

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <utility>

#if defined(NTDDI_VERSION)
#undef NTDDI_VERSION
#endif

#define NTDDI_VERSION NTDDI_WIN7

#include <Windows.h>
#include <d2d1.h>
#include <d2d1Helper.h>