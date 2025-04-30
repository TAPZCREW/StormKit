set_policy("compatibility.version", "3.0")

modules = {
    core = {
        public_packages = { "glm", "frozen", "unordered_dense", "magic_enum master", "tl_function_ref" },
        modulename = "core",
        has_headers = true,
        custom = function()
            if is_plat("windows") then add_packages("wil") end

            set_configdir("$(builddir)/.gens/include/")
            add_configfiles("include/(stormkit/core/**.hpp.in)")
            add_headerfiles("$(builddir)/.gens/include/(stormkit/core/**.hpp)")

            add_files("modules/stormkit/core.mpp")
            add_includedirs("$(builddir)/.gens/include", { public = true })
            add_cxflags("clang::-Wno-language-extension-token")

            on_load(function(target)
                local has_stacktrace = target:check_cxxsnippets({
                    test = [[
             void test() {
                 std::stacktrace::current();
             }
         ]],
                }, { configs = { languages = "c++23" }, includes = { "stacktrace" } })

                if not has_stacktrace then
                    print("No std C++23 stacktrace, falling back to cpptrace")
                    target:add("packages", "cpptrace")
                    if not target:is_plat("windows") then
                        target:add("packages", "libdwarf")
                    end
                end
            end)
            on_config(function(target)
                local output, errors = os.iorunv("git", { "rev-parse", "--abbrev-ref", "HEAD" })

                if not errors == "" then
                    print("Failed to get git hash and branch, reason: ", errors, output)
                    target:set("configvar", "STORMKIT_GIT_BRANCH", " ")
                    target:set("configvar", "STORMKIT_GIT_COMMIT_HASH", " ")
                    return
                end

                target:set("configvar", "STORMKIT_GIT_BRANCH", output:trim())
                output, errors = os.iorunv("git", { "rev-parse", "--verify", "HEAD" })

                target:set("configvar", "STORMKIT_GIT_COMMIT_HASH", output:trim())
            end)
        end,
    },
    log = {
        modulename = "log",
        public_deps = { "stormkit-core" },
        has_headers = true,
    },
    entities = {
        modulename = "entities",
        public_deps = { "stormkit-core" },
    },
    image = {
        packages = { "libktx", "libpng", "libjpeg-turbo 3.1.0" },
        modulename = "Image",
        public_deps = { "stormkit-core" },
    },
    main = {
        modulename = "main",
        has_headers = true,
        deps = { "stormkit-core" },
        custom = function()
            add_cxflags("-Wno-main")
            set_strip("debug")
        end,
        frameworks = is_plat("macosx") and { "CoreFoundation" } or nil,
    },
    wsi = {
        modulename = "Wsi",
        public_deps = { "stormkit-core" },
        deps = { "stormkit-log" },
        packages = is_plat("linux") and {
            "libxkbcommon",
            "libxcb",
            "xcb-util-keysyms",
            "xcb-util",
            "xcb-util-wm",
            "xcb-util-errors",
            "wayland",
            "wayland-protocols",
        } or nil,
        frameworks = is_plat("macosx") and { "CoreFoundation", "Foundation", "AppKit", "Metal", "IOKit", "QuartzCore" }
            or nil,
        custom = function()
            if is_plat("macosx") then
            elseif is_plat("linux") then
                add_rules("wayland.protocols")

                on_load(function(target)
                    local wayland_protocols_dir =
                        path.join(target:pkg("wayland-protocols"):installdir() or "/usr", "share", "wayland-protocols")
                    assert(wayland_protocols_dir, "wayland protocols directory not found")

                    local protocols = {
                        path.join("stable", "xdg-shell", "xdg-shell.xml"),
                        path.join("unstable", "xdg-decoration", "xdg-decoration-unstable-v1.xml"),
                        path.join("unstable", "pointer-constraints", "pointer-constraints-unstable-v1.xml"),
                        path.join("unstable", "relative-pointer", "relative-pointer-unstable-v1.xml"),
                    }

                    for _, protocol in ipairs(protocols) do
                        target:add("files", path.join(wayland_protocols_dir, protocol))
                    end
                end)
            elseif is_plat("mingw") then
                add_syslinks("user32", "shell32")
                add_ldflags("-Wl,-mwindows", { public = true })
            elseif is_plat("windows") then
                add_syslinks("user32", "shell32")
            end
        end,
    },
    engine = {
        modulename = "Engine",
        has_headers = true,
        public_deps = {
            "stormkit-core",
            "stormkit-log",
            "stormkit-wsi",
            "stormkit-image",
            "stormkit-entities",
            "stormkit-gpu",
        },
        custom = function()
            add_rules("compile.shaders")
            add_files("shaders/Engine/**.nzsl")
        end,
    },
    gpu = {
        modulename = "Gpu",
        has_headers = true,
        public_packages = {
            "vulkan-headers v1.4.309",
            "vulkan-memory-allocator 3.2.0",
            "vulkan-memory-allocator-hpp 3.2.1",
        },
        public_deps = { "stormkit-core", "stormkit-log", "stormkit-wsi", "stormkit-image" },
        packages = is_plat("linux") and {
            "libxcb",
            "wayland",
        } or nil,
        public_defines = {
            "VK_NO_PROTOTYPES",
            "VMA_DYNAMIC_VULKAN_FUNCTIONS=1",
            "VMA_STATIC_VULKAN_FUNCTIONS=0",
            "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1",
            "VULKAN_HPP_NO_STRUCT_CONSTRUCTORS",
            "VULKAN_HPP_NO_UNION_CONSTRUCTORS",
            "VULKAN_HPP_NO_EXCEPTIONS",
            "VULKAN_HPP_NO_CONSTRUCTORS",
            -- "VULKAN_HPP_NO_SMART_HANDLE",
            "VULKAN_HPP_STD_MODULE=std.compat",
            "VULKAN_HPP_ENABLE_STD_MODULE",
            "VMA_HPP_ENABLE_VULKAN_HPP_MODULE",
            "VMA_HPP_ENABLE_STD_MODULE",
        },
        custom = function()
            on_load(function(target)
                if target:kind() == "shared" then
                    target:add("defines", "VK_HPP_STORAGE_SHARED", { public = true })
                else
                    target:add("defines", "VK_HPP_STORAGE_API", { public = true })
                end
            end)
            if is_plat("linux") then
                add_defines("VK_USE_PLATFORM_XCB_KHR", { public = true })
                add_defines("VK_USE_PLATFORM_WAYLAND_KHR", { public = true })
            elseif is_plat("macosx") then
                add_defines("VK_USE_PLATFORM_MACOS_MVK", { public = true })
            elseif is_plat("windows") then
                add_defines("VK_USE_PLATFORM_WIN32_KHR", { public = true })
            end
        end,
    },
}

local allowedmodes = {
    "debug",
    "release",
    "releasedbg",
    "check",
    "coverage",
    "profile",
    "check",
    "minsizerel",
}

add_repositories("nazara-engine-repo https://github.com/NazaraEngine/xmake-repo")
add_repositories("tapzcrew-repo https://github.com/Tapzcrew/xmake-repo main")

set_xmakever("2.9.5")
set_project("StormKit")

set_version("0.1.0", { build = "%Y%m%d%H%M" })

includes("xmake/rules/*.lua")
includes("xmake/*.lua")

---------------------------- global rules ----------------------------
if get_config("vsxmake") then add_rules("plugin.vsxmake.autoupdate") end

if get_config("compile_commands") then
    add_rules("plugin.compile_commands.autoupdate", { outputdir = "build", lsp = "clangd" })
end

add_rules(
    "mode.debug",
    "mode.release",
    "mode.releasedbg",
    "mode.check",
    "mode.coverage",
    "mode.profile",
    "mode.check",
    "mode.minsizerel"
)

if not is_plat("windows") or not is_plat("mingw") then add_rules("mode.valgrind") end

set_fpmodels("fast")
add_vectorexts("fma")
add_vectorexts("neon")
add_vectorexts("avx", "avx2")
add_vectorexts("sse", "sse2", "sse3", "ssse3", "sse4.2")

---------------------------- options ----------------------------
option("examples_engine", {
    default = false,
    category = "root menu/others",
    deps = { "examples" },
    after_check = function(option)
        if option:dep("examples"):enabled() then option:enable(true) end
    end,
})
option("examples_wsi", {
    default = false,
    category = "root menu/others",
    deps = { "examples" },
    after_check = function(option)
        if option:dep("examples"):enabled() then option:enable(true) end
    end,
})
option("examples_log", {
    default = false,
    category = "root menu/others",
    deps = { "examples" },
    after_check = function(option)
        if option:dep("examples"):enabled() then option:enable(true) end
    end,
})
option("examples_entities", {
    default = false,
    category = "root menu/others",
    deps = { "examples" },
    after_check = function(option)
        if option:dep("examples"):enabled() then
        end
    end,
}) --option:enable(true) end end })
option("examples", { default = false, category = "root menu/others" })
option("applications", { default = false, category = "root menu/others" })
option("tests", { default = false, category = "root menu/others" })
option("tests_core", {
    default = false,
    category = "root menu/others",
    deps = { "tests" },
    after_check = function(option)
        if option:dep("tests"):enabled() then option:enable(true) end
    end,
})

option("sanitizers", { default = false, category = "root menu/build" })
option("mold", { default = false, category = "root menu/build" })
option("lto", { default = false, category = "root menu/build" })
option("on_ci", { default = false, category = "root menu/build" })

---------------------------- module options ----------------------------
option("log", { default = true, category = "root menu/modules" })
option("entities", { default = true, category = "root menu/modules" })
option("image", { default = true, category = "root menu/modules" })
option("main", { default = true, category = "root menu/modules" })
option("wsi", { default = true, category = "root menu/modules" })
option("gpu", { default = true, category = "root menu/modules", deps = { "log", "image", "wsi" } })
option("engine", {
    default = true,
    category = "root menu/modules",
    deps = { "log", "entities", "image", "wsi", "gpu" },
})
option("compile_commands", { default = false, category = "root menu/support" })
option("vsxmake", { default = false, category = "root menu/support" })

---------------------------- global configs ----------------------------
set_allowedmodes(allowedmodes)
set_allowedplats("windows", "mingw", "linux", "macosx", "wasm")
set_allowedarchs("windows|x64", "mingw|x86_64", "linux|x86_64", "linux|aarch64", "macosx|x86_64", "macosx|arm64")

add_defines("MAGIC_ENUM_USE_STD_MODULE")
add_defines("MAGIC_ENUM_DEFAULT_ENABLE_ENUM_FORMAT=0")
add_defines("FROZEN_USE_STD_MODULE")

set_policy("build.c++.modules.gcc.cxx11abi", true)

if not is_plat("wasm") then
    add_requireconfs("vulkan-headers", { system = false })
    -- add_requireconfs("vulkan-memory-allocator")
    add_requireconfs("vulkan-memory-allocator-hpp", { system = false, configs = { use_vulkanheaders = true } })
end

if get_config("toolchain") == "llvm" then
    add_requireconfs("libktx", { configs = { cxflags = "-Wno-overriding-option" } })
end

add_requireconfs("*", { configs = { modules = true, std_import = true, cpp = "latest" } })

if get_config("lto") then
    set_policy("build.optimization.lto", true)
    if get_config("kind") == "static" then add_defines("STORMKIT_LTO") end
end

add_requireconfs("libxkbcommon", { configs = { ["x11"] = true, wayland = true } })
add_requireconfs("frozen", { system = false })

add_requires("cpptrace")
if not is_plat("windows") then
    add_requires("libdwarf")
end

---------------------------- targets ----------------------------
for name, module in pairs(modules) do
    local modulename = module.modulename

    if name == "core" or name == "main" or get_config(name) then
        local packages = table.join(module.packages or {}, module.public_packages or {})
        add_requires(packages)
        local _packages = {}
        for _, package in ipairs(_packages) do
            table.insert(packages, package:split(" ")[1])
        end
        add_requireconfs(_packages, { configs = { modules = true, std_import = true, cpp = "latest" } })
        if get_config("on_ci") then add_requireconfs(_packages, { system = false }) end
        target("stormkit-" .. name, function()
            set_group("libraries")

            if module.custom then module.custom() end

            if name == "main" then
                set_kind("static")
            else
                set_kind("$(kind)")
            end

            set_languages("cxxlatest", "clatest")

            add_rules("stormkit.flags")
            add_defines("STORMKIT_BUILD")
            if is_mode("debug") then
                add_defines("STORMKIT_BUILD_DEBUG")
                set_suffixname("-d")
            end

            if is_kind("static") then add_defines("STORMKIT_STATIC", { public = true }) end

            local src_path = path.join("src", modulename)
            local module_path = path.join("modules", "stormkit", modulename)
            local include_path = path.join("include", "(stormkit", modulename)

            for _, file in ipairs(os.files(path.join(src_path, "**.mpp"))) do
                add_files(file)
            end
            for _, file in ipairs(os.files(path.join(src_path, "**.cpp"))) do
                add_files(file)
            end
            for _, file in ipairs(os.files(path.join(src_path, "**.mm"))) do
                add_files(file, { mxxflags = "-std=c++23" })
            end
            for _, file in ipairs(os.files(path.join(src_path, "**.m"))) do
                add_files(file)
            end
            for _, file in ipairs(os.files(path.join(src_path, "**.inl"))) do
                add_files(file)
            end

            if os.exists(module_path .. ".mpp") then add_files(module_path .. ".mpp", { public = true }) end

            if os.files(module_path) then
                for _, file in ipairs(os.files(path.join(module_path, "**.mpp"))) do
                    add_files(file, { public = true })
                end
                for _, file in ipairs(os.files(path.join(module_path, "**.inl"))) do
                    add_headerfiles(file)
                end
            end

            local _include_path = include_path:gsub("%(", "")
            if os.exists(_include_path) then
                add_headerfiles(path.join(include_path, "**.inl)"))
                add_headerfiles(path.join(include_path, "**.hpp)"))
            end

            if is_plat("windows") or is_plat("mingw") then
                for _, plat in ipairs({ "posix", "linux", "darwin", "macos", "ios", "bsd", "android" }) do
                    remove_files(path.join(src_path, plat, "**"))
                    remove_headerfiles(path.join(src_path, plat, "**"))
                end
            elseif is_plat("macosx") then
                for _, plat in ipairs({ "linux", "win32", "ios", "bsd", "android" }) do
                    remove_files(path.join(src_path, plat, "**"))
                    remove_headerfiles(path.join(src_path, plat, "**"))
                end
            elseif is_plat("ios") then
                for _, plat in ipairs({ "linux", "macOS", "win32", "bsd", "android" }) do
                    remove_files(path.join(src_path, plat, "**"))
                    remove_headerfiles(path.join(src_path, plat, "**"))
                end
            elseif is_plat("android") then
                for _, plat in ipairs({ "linux", "darwin", "macos", "ios", "bsd", "win32" }) do
                    remove_files(path.join(src_path, plat, "**"))
                    remove_headerfiles(path.join(src_path, plat, "**"))
                end
            elseif is_plat("linux") then
                for _, plat in ipairs({ "win32", "darwin", "macos", "ios", "bsd", "android" }) do
                    remove_files(path.join(src_path, plat, "**"))
                    remove_headerfiles(path.join(src_path, plat, "**"))
                end
            end

            add_includedirs("$(projectdir)/include", { public = true })

            if module.defines then add_defines(module.defines) end

            if module.public_defines then add_defines(module.public_defines, { public = true }) end

            if module.cxxflags then
                add_cxxflags(module.cxxflags)
                add_mxxflags(module.cxxflags)
            end

            if module.deps then add_deps(module.deps) end

            if module.public_deps then add_deps(module.public_deps, { public = true }) end

            if module.packages then
                local packages = {}
                for _, package in ipairs(module.packages) do
                    table.insert(packages, package:split(" ")[1])
                end

                add_packages(packages)
            end

            if module.public_packages then
                local packages = {}
                for _, package in ipairs(module.public_packages) do
                    table.insert(packages, package:split(" ")[1])
                end

                add_packages(packages, { public = true })
            end

            if module.frameworks then add_frameworks(module.frameworks, { public = is_kind("static") }) end
            
            set_fpmodels("fast")
            add_vectorexts("fma")
            add_vectorexts("neon")
            add_vectorexts("avx", "avx2")
            add_vectorexts("sse", "sse2", "sse3", "ssse3", "sse4.2")

            set_symbols("hidden")
            if is_mode("release") then
                set_optimize("fastest")
            elseif is_mode("debug") then
                set_symbols("debug", "hidden")
                add_cxflags("-ggdb3", { tools = { "clang", "gcc" } })
                add_mxflags("-ggdb3", { tools = { "clang", "gcc" } })
            elseif is_mode("releasedbg") then
                set_optimize("fast")
                set_symbols("debug", "hidden")
                add_cxflags("-fno-omit-frame-pointer", { tools = { "clang", "gcc" } })
                add_mxflags("-ggdb3", { tools = { "clang", "gcc" } })
            end
            
            add_options("sanitizers")
        end)
    end
end

for name, _ in pairs(modules) do
    if get_config("examples_" .. name) then
        local example_dir = path.join("examples", name)
        if os.exists(example_dir) and has_config("" .. name) then
            includes(path.join(example_dir, "**", "xmake.lua"))
        end
    end
end

if get_config("tests") then includes("tests/xmake.lua") end
