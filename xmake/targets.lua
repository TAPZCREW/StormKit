modules = {
    core = {
        public_packages = { "frozen", "unordered_dense", "tl_function_ref" },
        modulename = "core",
        has_headers = true,
        public_defines = {
            "FROZEN_USE_STD_MODULE",
        },
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
                    if not target:is_plat("windows") then target:add("packages", "libdwarf") end
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
    test = {
        modulename = "test",
        public_deps = { "stormkit-core" },
        has_headers = true,
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
        packages = { "libktx", "libpng", "libjpeg-turbo" },
        modulename = "image",
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
        modulename = "wsi",
        public_deps = { "stormkit-core" },
        deps = { "stormkit-log" },
        packages = is_plat("linux") and {
            "libxcb",
            "xcb-util-keysyms",
            "xcb-util",
            "xcb-util-wm",
            "xcb-util-errors",
            "wayland",
            "wayland-protocols",
            "libxkbcommon",
        } or nil,
        frameworks = is_plat("macosx") and { "CoreFoundation", "Foundation", "AppKit", "Metal", "IOKit", "QuartzCore" }
            or nil,
        custom = function()
            if is_plat("macosx") then
            elseif is_plat("linux") then
                add_rules("wayland.protocols")

                on_load(function(target)
                    assert(target:pkg("wayland-protocols"))
                    local wayland_protocols_dir =
                        path.join(target:pkg("wayland-protocols"):installdir() or "/usr", "share", "wayland-protocols")
                    assert(wayland_protocols_dir, "wayland protocols directory not found")

                    local protocols = {
                        path.join("stable", "xdg-shell", "xdg-shell.xml"),
                        path.join("stable", "tablet", "tablet-v2.xml"),
                        path.join("stable", "viewporter", "viewporter.xml"),
                        path.join("staging", "content-type", "content-type-v1.xml"),
                        path.join("staging", "pointer-warp", "pointer-warp-v1.xml"),
                        path.join("staging", "cursor-shape", "cursor-shape-v1.xml"),
                        path.join("staging", "single-pixel-buffer", "single-pixel-buffer-v1.xml"),
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
            elseif is_plat("windows") then
                add_syslinks("user32", "shell32")
            end
        end,
    },
    engine = {
        modulename = "engine",
        has_headers = true,
        public_deps = {
            "stormkit-core",
            "stormkit-log",
            "stormkit-wsi",
            "stormkit-image",
            "stormkit-entities",
            "stormkit-gpu",
        },
        packages = { "nzsl" },
        custom = function()
            add_rules("compile.shaders")
            add_files("shaders/engine/**.nzsl")
        end,
    },
    gpu = {
        modulename = "gpu",
        has_headers = true,
        public_packages = {
            "frozen",
            "volk",
            "vulkan-headers",
            "vulkan-memory-allocator",
        },
        public_deps = { "stormkit-core", "stormkit-log", "stormkit-wsi", "stormkit-image" },
        packages = is_plat("linux") and {
            "libxcb",
            "wayland",
        } or nil,
        public_defines = {
            "VMA_DYNAMIC_VULKAN_FUNCTIONS=0",
            "VMA_STATIC_VULKAN_FUNCTIONS=0",
            "STORMKIT_GPU_VULKAN",
        },
        custom = function()
            if is_plat("linux") then
                add_defines("VK_USE_PLATFORM_XCB_KHR", { public = true })
                add_defines("VK_USE_PLATFORM_WAYLAND_KHR", { public = true })
            elseif is_plat("macosx") then
                add_defines("VK_USE_PLATFORM_MACOS_MVK", { public = true })
            elseif is_plat("windows") then
                add_defines("VK_USE_PLATFORM_WIN32_KHR", { public = true })
            end
            add_cxflags("clang::-Wno-missing-declarations")
        end,
    },
}
