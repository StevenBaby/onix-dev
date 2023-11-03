#pragma once

#ifdef __i386__
asm(".code32\n\t");
#define arch i386
typedef uint32_t word_t;
#endif

#ifdef __x86_64__
#define arch x64
typedef uint64_t word_t;
#endif

#ifndef arch
#error "Unknown architecture!!!"
#endif

namespace arch
{
};

using namespace arch;
