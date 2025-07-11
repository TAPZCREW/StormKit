includes("ktx.lua")

local global_package_configs = {
    configs = {
        shared = get_config("shared_deps"),
        ["x11"] = true,
        wayland = true,
        modules = true,
        std_import = true,
        cpp = "latest",
    },
}

if get_config("on_ci") then global_package_configs.system = false end

if is_plat("windows") then
    add_requires("cpptrace", { system = false, configs = {
            override = true,
            configs = {
                cflags = {},
                cxflags = {},
                cxxflags = {}
            }}})
else
    add_requires("cpptrace")
end
if not is_plat("windows") then add_requires("libdwarf") end

local package_configs = {
    libktx = {
        llvm = {
            configs = {
                cxflags = "-Wno-overriding-option",
            },
        },
    },
    volk = {
        global = {
            configs = {
                -- header_only = true,
            },
        },
        windows = {
            override = true,
            configs = {
                cflags = {},
                cxflags = {},
                cxxflags = {}
            }
        },
    },
    frozen = {
        global = {
            system = false,
        },
    },
    ["vulkan-header"] = {
        global = {
            version = "v1.4.309",
            system = false,
        },
    },
    ["vulkan-memory-allocator"] = {
        global = {
            version = "v3.2.1",
            system = false,
        },
    },
    nzsl = {
        linux = {
            configs = {
                toolchains = "gcc",
                runtimes = "stdc++_shared",
            },
        },
        windows = {
            configs = {
                toolchains = "msvc",
                runtimes = "MD",
            },
        },
        global = {
            override = true,
            configs = {
                fs_watcher = false,
                link = {},
            },
        },
    },
}

function add_requires_with_conf(package)
    local configs = package_configs[package] or {}
    add_requires(
        package,
        table.join(
            global_package_configs,
            configs.global or {},
            is_plat("windows") and configs.windows or {},
            is_plat("linux") and configs.linux or {},
            is_plat("macosx") and configs.macos or {},
            get_config("toolchain") == "llvm" and configs.llvm or {},
            get_config("toolchain") == "msvc" and configs.msvc or {},
            get_config("toolchain") == "gcc" and configs.gcc or {}
        )
    )
end

function add_requires_with_conf_transitive(package)
    local configs = package_configs[package] or {}
    add_requires(
        package,
        table.join(
            global_package_configs,
            configs.global or {},
            is_plat("windows") and configs.windows or {},
            is_plat("linux") and configs.linux or {},
            is_plat("macosx") and configs.macos or {},
            get_config("toolchain") == "llvm" and configs.llvm or {},
            get_config("toolchain") == "msvc" and configs.msvc or {},
            get_config("toolchain") == "gcc" and configs.gcc or {}
        )
    )
    add_requireconfs(
        package .. ".**",
        table.join(
            global_package_configs,
            configs.global or {},
            is_plat("windows") and configs.windows or {},
            is_plat("linux") and configs.linux or {},
            is_plat("macosx") and configs.macos or {},
            get_config("toolchain") == "llvm" and configs.llvm or {},
            get_config("toolchain") == "msvc" and configs.msvc or {},
            get_config("toolchain") == "gcc" and configs.gcc or {}
        )
    )
end
