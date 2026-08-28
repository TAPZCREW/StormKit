local runtimes
local toolchain
if is_plat("windows") then
    runtimes = "MD"
    toolchain = "msvc"
elseif is_plat("linux") then
    runtimes = "stdc++_shared"
    toolchain = "gcc"
end
add_requires("nzsl", { configs = { fs_watcher = false, kind = "binary", toolchains = toolchain, runtimes = runtimes } })

target("textured_cube", function()
    add_rules("stormkit::example", "compile.shaders")

    add_files("src/*.cpp", "src/*.cppm", "../common/app.cppm", "shaders/*.nzsl")

    if get_config("devmode") then
        add_defines(format('RESOURCE_DIR="%s"', path.unix(path.join(os.projectdir(), "examples/gpu/textured_cube"))))
        add_defines(format('SHADER_DIR="%s"', path.unix("$(builddir)/shaders")))
    end
    add_embeddirs("$(builddir)/shaders")

    if get_config("devmode") then set_rundir("$(projectdir)") end

    add_packages("nzsl")

    set_group("examples/stormkit-gpu")
end)
