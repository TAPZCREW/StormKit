namespace("stormkit", function()
    rule("example", function()
        add_deps("stormkit::flags")
        add_deps("platform.windows.subsystem.console")

        on_load(function(target)
            target:set("kind", "binary")
            target:set("languages", "cxxlatest", "clatest")
            target:add("deps", "stormkit::stormkit", "stormkit::main")
            if target:is_plat("windows") then target:add("files", "examples/common/Windows/manifest.manifest") end
            if get_config("apple_bundle") then
                target:add("rules", "xcode.application")
                if target:is_plat("macosx") then
                    target:add("files", path.join("$(projectdir)", "examples/common/macOS/info.plist"))
                elseif target:is_plat("iphoneos") then
                    target:add("files", path.join("$(projectdir)", "examples/common/iOS/info.plist"))
                end
            end
        end)
    end)
end)
