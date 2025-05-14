rule("stormkit.flags", function()
    on_load("linux", "mingw", "macos", "ios", "android", function(target)
        if get_config("sanitizers") then
            target:set("policy", "build.sanitizer.address", true)
            target:set("policy", "build.sanitizer.undefined", true)
        end
        if get_config("lto") then
            target:set("policy", "build.optimization.lto", true)
        end
    end)
    on_load("windows", function(target)
        if get_config("sanitizers") then
            import("core.tool.compiler")
            local compinst = compiler.load("cxx")
            local cxx = path.filename(compinst:program())
            if
                not cxx:startswith("clang")
                or cxx:startswith("clang") and target:has_runtime("c++_shared", "c++_static")
            then
                target:set("policy", "build.sanitizer.address", true)
                target:set("policy", "build.sanitizer.undefined", true)
            end
        end
    end)
    on_config(function(target)
        target:set("warnings", "all", "pedantic", "extra", "error")

        local flags = {
            cl = {
                cxx = { "/Zc:__cplusplus" },
                cx = {
                    "/utf-8",
                    "/bigobj",
                    "/permissive-",
                    "/Zc:wchar_t",
                    "/Zc:inline",
                    "/Zc:lambda",
                    "/Zc:preprocessor",
                    "/Zc:referenceBinding",
                    "/Zc:strictStrings",
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
                    -- "-Wno-missing-field-initializers",
                    "-Wno-unknown-attributes",
                    "-Wno-deprecated-declarations",
                    "-Wno-ignored-attributes",
                },
                mx = {
                    -- "-Wno-missing-field-initializers",
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
        target:add("cxxflags", flags.clang.cxx or {}, { tools = { "clang" } })
        target:add("cxxflags", flags.gcc.cxx or {}, { tools = { "gcc" } })
        target:add("cxxflags", flags.cl.cxx or {}, { tools = { "cl", "clang_cl" } })

        target:add("cxflags", flags.clang.cx or {}, { tools = { "clang" } })
        target:add("cxflags", flags.gcc.cx or {}, { tools = { "gcc" } })
        target:add("cxflags", flags.cl.cx or {}, { tools = { "cl", "clang_cl" } })

        target:add("mxflags", flags.clang.mx or {}, { tools = { "clang" } })

        target:add("mxxflags", flags.clang.mxx or {}, { tools = { "clang" } })

        target:add("ldflags", flags.clang.ld or {}, { tools = { "clang", "clangxx", "lld" } })
        target:add("ldflags", flags.gcc.ld or {}, { tools = { "gcc", "g++", "ld" } })
        target:add("ldflags", flags.cl.ld or {}, { tools = { "cl", "link" } })

        target:add("shflags", flags.clang.sh or {}, { tools = { "clang", "clangxx", "lld" } })
        target:add("shflags", flags.gcc.sh or {}, { tools = { "gcc", "g++", "ld" } })
        target:add("shflags", flags.cl.sh or {}, { tools = { "cl", "link" } })

        target:add("arflags", flags.clang.ar or {}, { tools = { "clang", "clangxx", "llvm-ar" } })
        target:add("arflags", flags.gcc.ar or {}, { tools = { "gcc", "g++", "ar" } })
        target:add("arflags", flags.cl.ar or {}, { tools = { "cl", "clang_cl" } })
        if is_plat("windows") then
            target:add("defines", {
                "_CRT_SECURE_NO_WARNINGS",
                "WIN32_LEAN_AND_MEAN",
                "NOMINMAX",
            })

            if not target:has_runtime("c++_shared") then
                target:add("defines", "_MSVC_STL_HARDENING")
                if is_mode("debug") then
                    target:set("runtimes", "MDd")
                else
                    target:set("runtimes", "MD")
                end
            end
        end
        target:set("symbols", "hidden")
        if is_mode("release") then
            target:set("optimize", "fastest")
        elseif is_mode("debug") then
            target:set("symbols", "debug", "hidden")
            target:add("cxflags", "-ggdb3", { tools = { "clang", "gcc" } })
            target:add("mxflags", "-ggdb3", { tools = { "clang", "gcc" } })
        elseif is_mode("releasedbg") then
            target:set("optimize", "fast")
            target:set("symbols", "debug", "hidden")
            target:add("cxflags", "-fno-omit-frame-pointer", { tools = { "clang", "gcc" } })
            target:add("mxflags", "-ggdb3", { tools = { "clang", "gcc" } })
        end
    end)
end)
