/*
 * IntrinsicLibcalls.h
 *
 *  Created on: Apr 2, 2010
 *      Author: charlie
 */

#ifndef INTRINSICLIBCALLS_H_
#define INTRINSICLIBCALLS_H_

#include <map>
#include <set>

#include "llvm/ADT/StringRef.h"

static inline std::map<llvm::StringRef, llvm::StringRef>& getLibcallMap() {
    static std::map<llvm::StringRef, llvm::StringRef> libcall_map;
    return libcall_map;
}

static inline std::set<llvm::StringRef>& getAlwaysInlinedSet() {
    static std::set<llvm::StringRef> inlined;
    return inlined;
}

inline void InitLibcalls() {
    static bool initialized = false;
    if(initialized) {
        return;
    }
    initialized = true;

    auto& inlined = getAlwaysInlinedSet();
    auto& libcall_map = getLibcallMap();

    inlined.insert("llvm.va_start");
    inlined.insert("llvm.va_copy");
    inlined.insert("llvm.va_end");

    inlined.insert("llvm.dbg.declare");
    inlined.insert("llvm.dbg.value");

    inlined.insert("llvm.expect.i8");
    inlined.insert("llvm.expect.i16");
    inlined.insert("llvm.expect.i32");
    inlined.insert("llvm.expect.i64");

    inlined.insert("llvm.uadd.with.overflow.i32");

    inlined.insert("llvm.objectsize.i8");
    inlined.insert("llvm.objectsize.i16");
    inlined.insert("llvm.objectsize.i32");
    inlined.insert("llvm.objectsize.i64");

    inlined.insert("llvm.bswap.i8");
    inlined.insert("llvm.bswap.i16");
    inlined.insert("llvm.bswap.i32");

    inlined.insert("llvm.stacksave");
    inlined.insert("llvm.stackrestore");
    inlined.insert("llvm.trap");

    inlined.insert("llvm.uadd.with.overflow.i64");
    inlined.insert("llvm.umul.with.overflow.i64");

    inlined.insert("llvm.eh.exception");
    inlined.insert("llvm.eh.selector");

    inlined.insert("llvm.lifetime.start");
    inlined.insert("llvm.lifetime.end");

    // Memory intrinsics: support both typed-pointer and opaque-pointer spellings.
    // Prefer wrappers keyed by length type to avoid ABI pitfalls for small integer
    // lengths (e.g., i8/i16) when calling libc directly.
    libcall_map["llvm.memcpy.p0i8.p0i8.i8"] =  "memcpy_i8";
    libcall_map["llvm.memcpy.p0i8.p0i8.i16"] = "memcpy_i16";
    libcall_map["llvm.memcpy.p0i8.p0i8.i32"] = "memcpy_i32";
    libcall_map["llvm.memcpy.p0i8.p0i8.i64"] = "memcpy_i64";
    libcall_map["llvm.memcpy.p0.p0.i8"] =  "memcpy_i8";
    libcall_map["llvm.memcpy.p0.p0.i16"] = "memcpy_i16";
    libcall_map["llvm.memcpy.p0.p0.i32"] = "memcpy_i32";
    libcall_map["llvm.memcpy.p0.p0.i64"] = "memcpy_i64";
    libcall_map["llvm.memcpy.i8"] =  "memcpy_i8";
    libcall_map["llvm.memcpy.i16"] = "memcpy_i16";
    libcall_map["llvm.memcpy.i32"] = "memcpy_i32";
    libcall_map["llvm.memcpy.i64"] = "memcpy_i64";

    libcall_map["llvm.memmove.p0i8.p0i8.i8"] =  "memmove_i8";
    libcall_map["llvm.memmove.p0i8.p0i8.i16"] = "memmove_i16";
    libcall_map["llvm.memmove.p0i8.p0i8.i32"] = "memmove_i32";
    libcall_map["llvm.memmove.p0i8.p0i8.i64"] = "memmove_i64";
    libcall_map["llvm.memmove.p0.p0.i8"] =  "memmove_i8";
    libcall_map["llvm.memmove.p0.p0.i16"] = "memmove_i16";
    libcall_map["llvm.memmove.p0.p0.i32"] = "memmove_i32";
    libcall_map["llvm.memmove.p0.p0.i64"] = "memmove_i64";
    libcall_map["llvm.memmove.i8"] =  "memmove_i8";
    libcall_map["llvm.memmove.i16"] = "memmove_i16";
    libcall_map["llvm.memmove.i32"] = "memmove_i32";
    libcall_map["llvm.memmove.i64"] = "memmove_i64";

    libcall_map["llvm.memset.p0i8.i8"] =  "memset_i8";
    libcall_map["llvm.memset.p0i8.i16"] = "memset_i16";
    libcall_map["llvm.memset.p0i8.i32"] = "memset_i32";
    libcall_map["llvm.memset.p0i8.i64"] = "memset_i64";
    libcall_map["llvm.memset.p0.i8"] =  "memset_i8";
    libcall_map["llvm.memset.p0.i16"] = "memset_i16";
    libcall_map["llvm.memset.p0.i32"] = "memset_i32";
    libcall_map["llvm.memset.p0.i64"] = "memset_i64";
    libcall_map["llvm.memset.i8"] =  "memset_i8";
    libcall_map["llvm.memset.i16"] = "memset_i16";
    libcall_map["llvm.memset.i32"] = "memset_i32";
    libcall_map["llvm.memset.i64"] = "memset_i64";

    libcall_map["llvm.sqrt.f32"] = "sqrtf";
    libcall_map["llvm.sqrt.f64"] = "sqrt";
    libcall_map["llvm.sqrt.f80"] = "sqrtl";

    libcall_map["llvm.log.f32"] = "logf";
    libcall_map["llvm.log.f64"] = "log";
    libcall_map["llvm.log.f80"] = "logl";

    libcall_map["llvm.exp.f32"] = "expf";
    libcall_map["llvm.exp.f64"] = "exp";
    libcall_map["llvm.exp.f80"] = "expl";

    libcall_map["llvm.pow.f32"] = "powf";
    libcall_map["llvm.pow.f64"] = "pow";
    libcall_map["llvm.pow.f80"] = "powl";

    libcall_map["llvm.powi.f32"] = "powif";
    libcall_map["llvm.powi.f64"] = "powif";
    libcall_map["llvm.powi.f80"] = "powil";

    libcall_map["llvm.log10.f32"] = "log10f";
    libcall_map["llvm.log10.f64"] = "log10";
    libcall_map["llvm.log10.f80"] = "log10l";

    libcall_map["llvm.fabs.f32"] = "fabsf";
    libcall_map["llvm.fabs.f64"] = "fabs";
    libcall_map["llvm.fabs.f80"] = "fabsl";
}

inline bool isAlwaysInlined(llvm::StringRef intrinsic) {
    InitLibcalls();
    const auto& inlined = getAlwaysInlinedSet();
    return inlined.find(intrinsic) != inlined.end();
}

inline llvm::StringRef GetLibcall(llvm::StringRef intrinsic) {
    InitLibcalls();
    const auto& libcall_map = getLibcallMap();
    auto i = libcall_map.find(intrinsic);
    if(i == libcall_map.end()) {
        return llvm::StringRef();
    }
    return i->second;
}

#endif /* INTRINSICLIBCALLS_H_ */
