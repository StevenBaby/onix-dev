#pragma once

#ifdef __i386__
asm(".code32\n\t");
#define arch i386
#endif

#ifdef __x86_64__
#define arch x64
#endif

#ifndef arch
#error "Unknown architecture!!!"
#endif

namespace arch
{
};

using namespace arch;
