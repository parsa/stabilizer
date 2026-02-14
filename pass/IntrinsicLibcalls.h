/*
 * IntrinsicLibcalls.h
 *
 *  Created on: Apr 2, 2010
 *      Author: charlie
 */

#ifndef INTRINSICLIBCALLS_H_
#define INTRINSICLIBCALLS_H_

#include <mutex>

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

namespace stabilizer {
namespace intrinsic_libcalls {
    struct Tables {
        llvm::StringMap<llvm::StringRef> libcall_map;
    };

    inline Tables& getTables() {
        static Tables tables;
        static std::once_flag once;
        std::call_once(once, [&] {
            auto& libcall_map = tables.libcall_map;

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
            // Int-exponent pow: let LLVM lower `llvm.powi.*` itself. Mapping this
            // to a libcall is non-portable and easy to get wrong.

            libcall_map["llvm.log10.f32"] = "log10f";
            libcall_map["llvm.log10.f64"] = "log10";
            libcall_map["llvm.log10.f80"] = "log10l";

            libcall_map["llvm.fabs.f32"] = "fabsf";
            libcall_map["llvm.fabs.f64"] = "fabs";
            libcall_map["llvm.fabs.f80"] = "fabsl";
        });
        return tables;
    }
} // namespace intrinsic_libcalls
} // namespace stabilizer

inline llvm::StringRef GetLibcall(llvm::StringRef intrinsic) {
    const auto& tables = stabilizer::intrinsic_libcalls::getTables();
    auto i = tables.libcall_map.find(intrinsic);
    if(i == tables.libcall_map.end()) {
        return llvm::StringRef();
    }
    return i->second;
}

#endif /* INTRINSICLIBCALLS_H_ */
