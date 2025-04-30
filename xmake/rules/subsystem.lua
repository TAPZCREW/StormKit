rule("windows.subsystem.console", function()
    add_deps("windows.subsystem")
    add_orders("windows.subsystem.console", "windows.subsystem")
    on_load(function(target) target:data_set("windows.subsystem", "console") end)
end)

rule("windows.subsystem.windows", function()
    add_deps("windows.subsystem")
    add_orders("windows.subsystem.windows", "windows.subsystem")
    on_load(function(target) target:data_set("windows.subsystem", "windows") end)
end)

rule("windows.subsystem", function()
    on_config("mingw", "windows", function(target)
        local subsystem = target:data("windows.subsystem")
        local linker = target:tool("ld")
        linker = path.filename(linker)
        if linker:startswith("clang") then
            target:add("ldflags", "-Wl,-subsystem:" .. subsystem, { force = true })
        elseif linker:startswith("link") or linker:startswith("lld") then
            target:add("ldflags", "/SUBSYSTEM:" .. string.upper(subsystem), { force = true })
        elseif linker:startswith("gcc") or linker:startswith("g++") then
            target:add("ldflags", "-Wl,-m" .. subsystem)
        elseif linker:startswith("ld") then
            target:add("ldflags", "-m" .. subsystem)
        end
    end)
end)
