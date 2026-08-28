namespace("stormkit", function()
    rule("flags", function()
        on_config("linux", "mingw", "macosx", "ios", "android", function(target)
            if get_config("lto") then
                target:set("policy", "build.optimization.lto", true)
                if get_config("toolchain") == "llvm" or get_config("toolchain") == "clang" then
                    target:add("ldflags", "-flto=thin", { force = true })
                    target:add("shflags", "-flto=thin", { force = true })
                end
            end
            if get_config("mold") and not is_subhost("windows") then
                local arg = "-fuse-ld=mold"
                if type(get_config("mold")) == "string" then arg = "-fuse-ld=" .. get_config("mold") end
                target:add("ldflags", arg, { force = true })
                target:add("shflags", arg, { force = true })
            end
            target:set("utf-8", true)

            if get_config("sanitizers") and is_mode("debug", "release", "releasedbg") and target:is_binary() then
                target:set("policy", "build.sanitizer.address", true)
                target:set("policy", "build.sanitizer.undefined", true)
            end
            if
                get_config("toolchain") == "llvm"
                or get_config("toolchain") == "clang"
                or get_config("toolchain") == "gcc"
            then
                target:add("syslinks", "dl")
            end
        end)
        on_config("windows", function(target)
            import("core.tool.compiler")
            local rad_enabled = false
            if get_config("rad") and is_subhost("windows") then
                rad_enabled = true
                target:add("ldflags", "-fuse-ld=radlink", { force = true })
                target:add("shflags", "-fuse-ld=radlink", { force = true })
            else
                target:add("ldflags", "-fuse-ld=lld-link", { force = true })
                target:add("shflags", "-fuse-ld=lld-link", { force = true })
            end

            target:add("cxflags", "clang::-fms-compatibility")
            target:add("defines", "_CRT_STDIO_ISO_WIDE_SPECIFIERS=1")

            if get_config("sanitizers") and is_mode("release", "releasedbg") and target:is_binary() then
                if get_config("toolchain") == "llvm" or get_config("toolchain") == "clang" then
                    if get_config("runtimes") == "c++_shared" or get_config("runtimes") == "c++_static" then
                        target:set("policy", "build.sanitizer.address", true)
                        target:set("policy", "build.sanitizer.undefined", true)
                    end
                end
            end
        end)
        on_config(function(target)
            if get_config("devmode") then
                target:set("warnings", "allextra", "pedantic")
            else
                target:set("warnings", "allextra", "pedantic", "error")
            end
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
                    cx = table.join(
                        {
                            "-fstrict-aliasing",
                            "-Wno-error=unknown-attributes",
                            "-Wno-error=sign-conversion",
                            "-Wno-error=shadow",
                            "-Wstrict-aliasing",
                            "-fanalyzer",
                            "-Wconversion",
                            "-Wshadow",
                            "-fdiagnostics-color=always",
                            "-fstack-protector-strong",
                            "-fstack-clash-protection",
                            "-fcf-protection=full",
                            "-ftrivial-auto-var-init=zero",
                            "-Wsuggest-attribute=pure",
                            "-Wsuggest-attribute=const",
                        },
                        is_mode("debug", "releasedbg")
                                and { "-ggdb3", "-fno-omit-frame-pointer", "-fno-sanitize-merge" }
                            or {}
                    ),
                    ld = target:has_runtime("c++_shared", "c++_static") and {} or {},
                    sh = target:has_runtime("c++_shared", "c++_static") and {} or {},
                },
                clang = {
                    cxx = {
                        "-Wno-include-angled-in-module-purview",
                    },
                    cx = table.join(
                        {
                            "-fstrict-aliasing",
                            "-Wno-gnu-statement-expression-from-macro-expansion",
                            "-Wno-error=gnu-statement-expression-from-macro-expansion",
                            "-Wno-error=unknown-attributes",
                            "-Wstrict-aliasing",
                            "-Wno-error=sign-conversion",
                            "-Wno-error=shadow",
                            "-Wconversion",
                            "-Wshadow",
                            "-Wno-c23-extensions",
                            "-Wno-error=c23-extensions",
                            "-fretain-comments-from-system-headers",
                            "-fdiagnostics-color=always",
                            "-fcolor-diagnostics",
                            "-fansi-escape-codes",
                            "-fstack-protector-strong",
                            "-fstack-clash-protection",
                            "-ftrivial-auto-var-init=zero",
                        },
                        is_plat("linux")
                                and {
                                    "-fcf-protection=full",
                                }
                            or {},
                        is_mode("debug", "releasedbg")
                                and { "-ggdb3", "-fno-omit-frame-pointer", "-fno-sanitize-merge" }
                            or {},
                        target:has_runtime("c++_shared", "c++_static") and { "-fexperimental-library" } or {}
                    ),
                    mx = is_mode("debug", "releasedbg")
                            and { "-ggdb3", "-fno-omit-frame-pointer", "-fno-sanitize-merge" }
                        or {},
                    mxx = {
                        "-fexperimental-library",
                    },
                    ld = table.join(
                        target:has_runtime("c++_shared", "c++_static") and { "-fexperimental-library" } or {},
                        (
                            target:is_plat("windows")
                            and is_mode("release")
                            and target:has_runtime("c++_shared", "c++_static")
                        )
                                and { "-Xlinker -NODEFAULTLIB:libcmt" }
                            or {}
                    ),
                    sh = target:has_runtime("c++_shared", "c++_static") and { "-fexperimental-library" } or {},
                },
            }
            if target:has_tool("cxx", "clang", "clangxx") then
                target:add("cxxflags", flags.clang.cxx or {}, { tools = { "clang", "clangxx" }, force = true })
                target:add("cxxflags", flags.clang.cx or {}, { tools = { "clang", "clangxx" }, force = true })
                target:add("cflags", flags.clang.cx or {}, { tools = { "clang" }, force = true })
                target:add("mxflags", flags.clang.mx or {}, { tools = { "clang" }, force = true })
                target:add("mxxflags", flags.clang.mxx or {}, { tools = { "clang", "clang++" }, force = true })
                target:add("ldflags", flags.clang.ld or {}, { tools = { "clang", "clangxx", "lld" }, force = true })
                target:add("shflags", flags.clang.sh or {}, { tools = { "clang", "clangxx", "lld" }, force = true })
                target:add("arflags", flags.clang.ar or {}, { tools = { "clang", "clangxx", "llvm-ar" }, force = true })
                if (is_plat("linux", "mingw")) and not target:has_runtime("c++_shared", "c++_static") then
                    target:add("syslinks", "stdc++exp", "stdc++fs")
                end
            end

            if target:has_tool("cxx", "gcc", "gxx") then
                target:add("cxxflags", flags.gcc.cxx or {}, { tools = { "gcc", "g++" }, force = true })
                target:add("cxxflags", flags.gcc.cx or {}, { tools = { "gcc", "g++" }, force = true })
                target:add("cflags", flags.gcc.cx or {}, { tools = { "gcc" }, force = true })
                target:add("ldflags", flags.gcc.ld or {}, { tools = { "gcc", "g++", "ld" }, force = true })
                target:add("shflags", flags.gcc.sh or {}, { tools = { "gcc", "g++", "ld" }, force = true })
                target:add("arflags", flags.gcc.ar or {}, { tools = { "gcc", "g++", "ar" }, force = true })
                target:add("syslinks", "stdc++exp", "stdc++fs")
            end

            if target:has_tool("cxx", "cl", "clang_cl") then
                target:add("cxxflags", flags.cl.cxx or {}, { tools = { "cl", "clang_cl" }, force = true })
                target:add("cxxflags", flags.cl.cx or {}, { tools = { "cl", "clang_cl" }, force = true })
                target:add("cflags", flags.cl.cx or {}, { tools = { "cl", "clang_cl" }, force = true })
                target:add("ldflags", flags.cl.ld or {}, { tools = { "cl", "link" }, force = true })
                target:add("shflags", flags.cl.sh or {}, { tools = { "cl", "link" }, force = true })
                target:add("arflags", flags.cl.ar or {}, { tools = { "cl", "clang_cl" }, force = true })
            end

            if is_plat("windows") then
                local runtimes = { is_mode("debug") and "MDd" or "MD" }

                local libcpp = target:has_runtime("c++_shared", "c++_static")
                local libstdcpp = target:has_runtime("stdc++_shared", "stdc++_static")

                if is_mode("debug") then
                    if libcpp then
                        target:add("defines", "_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG")
                    elseif libstdcpp then
                        target:add("defines", "_FORTIFY_SOURCE=3")
                        if get_config("devmode") then
                            target:add("defines", "_GLIBCXX_DEBUG")
                        else
                            target:add("defines", "_GLIBCXX_ASSERTIONS")
                        end
                    else
                        target:add("defines", "_MSVC_STL_HARDENING=1")
                    end
                elseif is_mode("releasedbg") then
                    if libcpp then
                        target:add("defines", "_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE")
                    elseif libstdcpp then
                        target:add("defines", "_FORTIFY_SOURCE=2", "_GLIBCXX_ASSERTIONS")
                    else
                        target:add("defines", "_MSVC_STL_HARDENING=1")
                    end
                elseif is_mode("release") then
                    if libcpp then
                        target:add("defines", "_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST")
                    elseif libstdcpp then
                        target:add("defines", "_FORTIFY_SOURCE=1")
                    else
                        target:add("defines", "_MSVC_STL_HARDENING=1")
                    end
                end

                if target:has_runtime("c++_shared") then
                    table.insert(runtimes, "c++_shared")
                elseif target:has_runtime("c++_static") then
                    table.insert(runtimes, "c++_static")
                end
                target:set("runtimes", table.unpack(runtimes))
            end
            if is_mode("release") then
                target:set("symbols", "hidden")
                target:set("optimize", "fast")
            elseif is_mode("debug") then
                target:set("symbols", "debug")
                target:add("cxflags", "-ggdb3", { tools = { "clang", "gcc" } })
                target:add("mxflags", "-ggdb3", { tools = { "clang", "gcc" } })
            elseif is_mode("releasedbg") then
                target:set("optimize", "fast")
                target:set("symbols", "debug", "hidden")
                target:add("mxflags", "-ggdb3", { tools = { "clang", "gcc" } })
            end
            target:set("fpmodels", "fast")
            target:add("vectorexts", "fma")
            target:add("vectorexts", "neon")
            target:add("vectorexts", "avx", "avx2")
            target:add("vectorexts", "sse", "sse2", "sse3", "ssse3", "sse4.2")
        end)
    end)
end)
