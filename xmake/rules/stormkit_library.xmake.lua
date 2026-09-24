namespace("stormkit", function()
    rule("library", function()
        add_deps("@stormkit::flags")

        on_config(function(target)
            import("core.base.hashset")

            local stormkit_components = target:values("stormkit.components") or {}
            local stormkit_components_set = hashset.from(stormkit_components)

            -- core --
            target:add("packages", "frozen", "unordered_dense", "nontype_functional")

            if stormkit_components_set:has("image") then target:add("packages", "libktx", "libpng", "libjpeg-turbo") end
            if stormkit_components_set:has("wsi") then
                if target:is_plat("linux") then
                    target:add(
                        "packages",
                        "libxcb",
                        "xcb-util-keysyms",
                        "xcb-util",
                        "xcb-util-image",
                        "xcb-util-wm",
                        "xcb-util-errors",
                        "wayland",
                        "wayland-protocols",
                        "libxkbcommon"
                    )
                end
            end
            if stormkit_components_set:has("gpu") then
                target:add("packages", "volk", "vulkan-headers", "vulkan-memory-allocator")
            end

            target:add("packages", "stormkit", { components = table.join("core", stormkit_components) })
        end)
    end)
end)
