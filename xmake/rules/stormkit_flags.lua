rule("stormkit.flags", function()
    on_load("linux", "mingw", "macos", "ios", "android", function(target)
        local target_add = target.add
        local target_set = target.set
        if get_config("sanitizers") then
            target_set(target, "policy", "build.sanitizer.address", true)
            target_set(target, "policy", "build.sanitizer.undefined", true)
        end
        if get_config("lto") then target_set(target, "policy", "build.optimization.lto", true) end
        if get_config("mold") then
            target_add(target, "ldflags", "-fuse-ld=mold")
            target_add(target, "shflags", "-fuse-ld=mold")
        end
        target_set(target, "utf-8", true)
    end)
    on_load("windows", function(target)
        import("core.tool.compiler")
        local compinst = compiler.load("cxx")
        local cxx = path.filename(compinst:program())
        local target_add = target.add
        local target_set = target.set
        local startswith = string.startswith
        if get_config("sanitizers") then
            if
                not startswith(cxx, "clang")
                or startswith(cxx, "clang") and target:has_runtime("c++_shared", "c++_static")
            then
                target_set(target, "policy", "build.sanitizer.address", true)
                target_set(target, "policy", "build.sanitizer.undefined", true)
            end
        end
        if startswith(cxx, "clang") or startswith(cxx, "g++") or startswith(cxx, "gcc") or get_config("mold") then
            target_add(target, "ldflags", "-fuse-ld=mold")
            target_add(target, "shflags", "-fuse-ld=mold")
        end
    end)
    on_config(function(target)
        local target_add = target.add
        local target_set = target.set
        target_set(target, "warnings", "allextra", "pedantic", "error")
        target_set(target, "symbols", "hidden")
        if is_mode("release") then
            target_set(target, "optimize", "fastest")
        elseif is_mode("debug") then
            target_set(target, "symbols", "debug", "hidden")
        elseif is_mode("releasedbg") then
            target_set(target, "optimize", "fast")
            target_set(target, "symbols", "debug", "hidden")
        end
        target_set(target, "fpmodels", "fast")
        target_add(target, "vectorexts", "fma")
        target_add(target, "vectorexts", "neon")
        target_add(target, "vectorexts", "avx", "avx2")
        target_add(target, "vectorexts", "sse", "sse2", "sse3", "ssse3", "sse4.2")

        local flags = {
            cl = {
                cxx = {
                    "/Zc:__cplusplus",
                    "/Zc:lambda",
                    "/Zc:referenceBinding",
                },
                cx = {
                    "/bigobj",
                    "/permissive-",
                    "/Zc:wchar_t",
                    "/Zc:inline",
                    "/Zc:preprocessor",
                    "/Zc:strictStrings",
                    "/analyze",
                    "/wd4251", -- Disable warning: class needs to have dll-interface to be used by clients of class blah blah blah
                    "/wd4297",
                    "/wd4996",
                    "/wd5063",
                    "/wd5260",
                    "/wd5050",
                    "/wd4005",
                    "/wd4611", -- Disable setjmp warning
                },
            },
            gcc = {
                cx = table.join({
                    "-fstrict-aliasing",
                    "-Wno-error=unknown-attributes",
                    "-Wno-error=sign-conversion",
                    "-Wno-error=shadow",
                    "-Wstrict-aliasing",
                    "-fanalyzer",
                    "-Wconversion",
                    "-Wshadow"
                }, is_mode("debug", "releasedbg") and { "-ggdb3", "-fno-omit-frame-pointer", "-fno-sanitize-merge"} or {}),
                ld = target:has_runtime("c++_shared", "c++_static") and {} or {},
                sh = target:has_runtime("c++_shared", "c++_static") and {} or {},
            },
            clang = {
                cxx = {
                    "-Wno-include-angled-in-module-purview",
                },
                cx = table.join({
                    "-fstrict-aliasing",
                    "-Wno-error=unknown-attributes",
                    "-Wstrict-aliasing",
                    "-Wno-error=sign-conversion",
                    "-Wno-error=shadow",
                    "-Wconversion",
                    "-Wshadow",
                    "-Wno-c23-extensions",
                    "-Wno-error=c23-extensions",
                    "-fretain-comments-from-system-headers"
                }, is_mode("debug", "releasedbg") and { "-ggdb3", "-fno-omit-frame-pointer", "-fno-sanitize-merge"} or {}, target:has_runtime("c++_shared", "c++_static") and {"-fexperimental-library"} or {}),
                mx = is_mode("debug", "releasedbg") and { "-ggdb3", "-fno-omit-frame-pointer", "-fno-sanitize-merge"} or {},
                mxx = {
                    "-fexperimental-library",
                },
                ld = target:has_runtime("c++_shared", "c++_static") and {"-fexperimental-library"} or {},
                sh = target:has_runtime("c++_shared", "c++_static") and {"-fexperimental-library"} or {},
            },
        }
        local toolchain = get_config("toolchain")
        local is_clang = toolchain and (toolchain == "clang" or toolchain == "llvm") or false
        if is_plat("macosx") or is_clang then
            target_add(target, "cxxflags", flags.clang.cxx or {}, { tools = { "clang", "clangxx" } })
            target_add(target, "cxxflags", flags.clang.cx or {}, { tools = { "clang", "clangxx" } })
            target_add(target, "cflags", flags.clang.cx or {}, { tools = { "clang" } })
            target_add(target, "mxflags", flags.clang.mx or {}, { tools = { "clang" } })
            target_add(target, "mxxflags", flags.clang.mxx or {}, { tools = { "clang", "clang++" } })
            target_add(target, "ldflags", flags.clang.ld or {}, { tools = { "clang", "clangxx", "lld" } })
            target_add(target, "shflags", flags.clang.sh or {}, { tools = { "clang", "clangxx", "lld" } })
            target_add(target, "arflags", flags.clang.ar or {}, { tools = { "clang", "clangxx", "llvm-ar" } })
            if (is_plat("linux") or is_plat("mingw")) and not target:has_runtime("c++_shared", "c++_static") then
                target_add(target, "syslinks", "stdc++exp", "stdc++fs")
            end
        end

        if is_plat("linux") and (not toolchain or toolchain == "gcc") then
            target_add(target, "cxxflags", flags.gcc.cxx or {}, { tools = { "gcc", "g++" } })
            target_add(target, "cxxflags", flags.gcc.cx or {}, { tools = { "gcc", "g++" } })
            target_add(target, "cflags", flags.gcc.cx or {}, { tools = { "gcc" } })
            target_add(target, "ldflags", flags.gcc.ld or {}, { tools = { "gcc", "g++", "ld" } })
            target_add(target, "shflags", flags.gcc.sh or {}, { tools = { "gcc", "g++", "ld" } })
            target_add(target, "arflags", flags.gcc.ar or {}, { tools = { "gcc", "g++", "ar" } })
            target_add(target, "syslinks", "stdc++exp", "stdc++fs")
        end

        if is_plat("windows") and (not toolchain or toolchain == "clang-cl") then
            target_add(target, "cxxflags", flags.cl.cxx or {}, { tools = { "cl", "clang_cl" } })
            target_add(target, "cxxflags", flags.cl.cx or {}, { tools = { "cl", "clang_cl" } })
            target_add(target, "cflags", flags.cl.cx or {}, { tools = { "cl", "clang_cl" } })
            target_add(target, "ldflags", flags.cl.ld or {}, { tools = { "cl", "link" } })
            target_add(target, "shflags", flags.cl.sh or {}, { tools = { "cl", "link" } })
            target_add(target, "arflags", flags.cl.ar or {}, { tools = { "cl", "clang_cl" } })
        end

        if is_plat("windows") then
            target_add(target, "defines", {
                "_CRT_SECURE_NO_WARNINGS",
                "WIN32_LEAN_AND_MEAN",
                "NOMINMAX",
            })

            if not target:has_runtime("c++_shared") then
                target_add(target, "defines", "_MSVC_STL_HARDENING")
                if is_mode("debug") then
                    target_set(target, "runtimes", "MDd")
                else
                    target_set(target, "runtimes", "MD")
                end
            end
        end
        target_set(target, "symbols", "hidden")
        if is_mode("release") then
            target_set(target, "optimize", "fastest")
        elseif is_mode("debug") then
            target_set(target, "symbols", "debug", "hidden")
            target_add(target, "cxflags", "-ggdb3", { tools = { "clang", "gcc" } })
            target_add(target, "mxflags", "-ggdb3", { tools = { "clang", "gcc" } })
        elseif is_mode("releasedbg") then
            target_set(target, "optimize", "fast")
            target_set(target, "symbols", "debug", "hidden")
            target_add(target, "cxflags", "-fno-omit-frame-pointer", { tools = { "clang", "gcc" } })
            target_add(target, "mxflags", "-ggdb3", { tools = { "clang", "gcc" } })
        end
    end)
end)
