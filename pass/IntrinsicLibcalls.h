/*
 * IntrinsicLibcalls.h
 *
 *  Created on: Apr 2, 2010
 *      Author: charlie
 */

#ifndef INTRINSICLIBCALLS_H_
#define INTRINSICLIBCALLS_H_

#include "llvm/ADT/StringRef.h"

#include <map>
#include <set>

using llvm::StringRef;

static std::map<StringRef, StringRef> libcall_map;

static std::set<StringRef> inlined;

void InitLibcalls() {
    if (!inlined.empty()) return; // Already initialized

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
    inlined.insert("llvm.lifetime.start.p0");
    inlined.insert("llvm.lifetime.end.p0");

    libcall_map["llvm.powi.f32.i32"] = "powif";
    libcall_map["llvm.powi.f64.i32"] = "powid";
    libcall_map["llvm.powi.f80.i32"] = "powil";
}

static StringRef matchMemoryIntrinsic(StringRef intrinsic) {
    if(!intrinsic.starts_with("llvm.")) {
        return "";
    }
    StringRef rest = intrinsic.drop_front(5);

    auto startsWithAny = [&](StringRef prefix) -> bool {
        return rest.starts_with(prefix);
    };

    if(startsWithAny("memcpy.") || startsWithAny("memcpy.inline.") || startsWithAny("memcpy.element.unordered.atomic.")) {
        return "memcpy";
    }
    if(startsWithAny("memmove.") || startsWithAny("memmove.inline.") || startsWithAny("memmove.element.unordered.atomic.")) {
        return "memmove";
    }
    if(startsWithAny("memset.") || startsWithAny("memset.inline.") || startsWithAny("memset.element.unordered.atomic.")) {
        size_t lastDot = intrinsic.rfind('.');
        if(lastDot == StringRef::npos || lastDot + 1 >= intrinsic.size()) {
            return "";
        }
        StringRef lenTy = intrinsic.drop_front(lastDot + 1);
        if(!lenTy.consume_front("i")) {
            return "";
        }
        if(lenTy == "8") {
            return "memset_i8";
        } else if(lenTy == "16") {
            return "memset_i16";
        } else if(lenTy == "32") {
            return "memset_i32";
        } else {
            return "memset_i64";
        }
    }

    return "";
}

static StringRef mapMathLibcall(StringRef base, StringRef type) {
    auto pick = [&](StringRef f32, StringRef f64, StringRef f80) -> StringRef {
        if(type == "f32") return f32;
        if(type == "f64") return f64;
        if(type == "f80") return f80;
        return "";
    };

    if(base == "sqrt") {
        return pick("sqrtf", "sqrt", "sqrtl");
    } else if(base == "log") {
        return pick("logf", "log", "logl");
    } else if(base == "exp") {
        return pick("expf", "exp", "expl");
    } else if(base == "pow") {
        return pick("powf", "pow", "powl");
    } else if(base == "powi") {
        return pick("powif", "powid", "powil");
    } else if(base == "fabs") {
        return pick("fabsf", "fabs", "fabsl");
    } else if(base == "log10") {
        return pick("log10f", "log10", "log10l");
    }
    return "";
}

static StringRef matchScalarMathIntrinsic(StringRef intrinsic) {
    if(!intrinsic.starts_with("llvm.")) {
        return "";
    }
    StringRef rest = intrinsic.drop_front(5);
    auto pair = rest.split('.');
    StringRef base = pair.first;
    StringRef suffix = pair.second;
    if(base.empty() || suffix.empty()) {
        return "";
    }

    if(base == "powi") {
        auto typeAndRest = suffix.split('.');
        StringRef type = typeAndRest.first;
        if(type.empty()) {
            return "";
        }
        return mapMathLibcall(base, type);
    }

    // Ignore vector forms: they contain 'v' immediately after base.
    if(suffix.starts_with("v")) {
        return "";
    }

    auto type = suffix.split('.').first;
    return mapMathLibcall(base, type);
}

static bool isPatternInlined(StringRef intrinsic) {
    if(!intrinsic.starts_with("llvm.")) {
        return false;
    }
    StringRef rest = intrinsic.drop_front(5);
    auto skipVector = [&](StringRef base) {
        return rest.starts_with(base);
    };
    return skipVector("sqrt.v") || skipVector("log.v") || skipVector("exp.v")
        || skipVector("pow.v") || skipVector("powi.v")
        || skipVector("fabs.v") || skipVector("log10.v");
}

bool isAlwaysInlined(StringRef intrinsic) {
    if(inlined.find(intrinsic) != inlined.end()) {
        return true;
    }
    return isPatternInlined(intrinsic);
}

StringRef GetLibcall(StringRef intrinsic) {
    auto found = libcall_map.find(intrinsic);
    if(found != libcall_map.end()) {
        return found->second;
    }

    if(StringRef mem = matchMemoryIntrinsic(intrinsic); !mem.empty()) {
        return mem;
    }

    if(StringRef math = matchScalarMathIntrinsic(intrinsic); !math.empty()) {
        return math;
    }

    return "";
}

#endif /* INTRINSICLIBCALLS_H_ */
