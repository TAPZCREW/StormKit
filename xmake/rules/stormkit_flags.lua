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

        local flags = {
            cl = {
                cxx = {
                    "/Zc:__cplusplus",
                    "/Zc:lambda",
                    "/Zc:referenceBinding",
                },
                cx = {
                    "/utf-8",
                    "/bigobj",
                    "/permissive-",
                    "/Zc:wchar_t",
                    "/Zc:inline",
                    "/Zc:preprocessor",
                    "/Zc:strictStrings",
                    "/analyze",
                    "/wd4251", -- Disable warning: class needs to have dll-interface to be used by clients of class blah blah blah
                    "/wd4297",
                    "/wd5063",
                    "/wd5260",
                    "/wd5050",
                    "/wd4005",
                    "/wd4611", -- Disable setjmp warning
                },
            },
            gcc = {
                cx = {
                    "-fstrict-aliasing",
                    "-Wstrict-aliasing",
                    "-fanalyzer",
                    "-Wconversion",
                    "-Wshadow"
                },
            },
            clang = {
                cxx = {
                    "-Wno-include-angled-in-module-purview",
                },
                cx = {
                    "-fstrict-aliasing",
                    "-fexperimental-library",
                    "-Wstrict-aliasing",
                    "-Wconversion",
                    "-Wshadow",
                },
                mx = {
                },
                ld = {
                    "-fexperimental-library",
                },
                sh = {
                    "-fexperimental-library",
                },
                mxx = {
                    "-fexperimental-library",
                },
            },
        }
        target_add(target, "cxxflags", flags.clang.cxx or {}, { tools = { "clang" } })
        target_add(target, "cxxflags", flags.gcc.cxx or {}, { tools = { "gcc" } })
        target_add(target, "cxxflags", flags.cl.cxx or {}, { tools = { "cl", "clang_cl" } })

        target_add(target, "cxflags", flags.clang.cx or {}, { tools = { "clang" } })
        target_add(target, "cxflags", flags.gcc.cx or {}, { tools = { "gcc" } })
        target_add(target, "cxflags", flags.cl.cx or {}, { tools = { "cl", "clang_cl" } })

        target_add(target, "mxflags", flags.clang.mx or {}, { tools = { "clang" } })

        target_add(target, "mxxflags", flags.clang.mxx or {}, { tools = { "clang" } })

        target_add(target, "ldflags", flags.clang.ld or {}, { tools = { "clang", "clangxx", "lld" } })
        target_add(target, "ldflags", flags.gcc.ld or {}, { tools = { "gcc", "g++", "ld" } })
        target_add(target, "ldflags", flags.cl.ld or {}, { tools = { "cl", "link" } })

        target_add(target, "shflags", flags.clang.sh or {}, { tools = { "clang", "clangxx", "lld" } })
        target_add(target, "shflags", flags.gcc.sh or {}, { tools = { "gcc", "g++", "ld" } })
        target_add(target, "shflags", flags.cl.sh or {}, { tools = { "cl", "link" } })

        target_add(target, "arflags", flags.clang.ar or {}, { tools = { "clang", "clangxx", "llvm-ar" } })
        target_add(target, "arflags", flags.gcc.ar or {}, { tools = { "gcc", "g++", "ar" } })
        target_add(target, "arflags", flags.cl.ar or {}, { tools = { "cl", "clang_cl" } })
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
