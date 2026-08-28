---------------------------- global options ----------------------------
option("examples_gpu", {
    default = false,
    category = "root menu/build",
    deps = { "examples", "gpu" },
    after_check = function(option)
        if option:dep("examples"):enabled() then option:enable(option:dep("gpu"):enabled()) end
    end,
})
option("examples_wsi", {
    default = false,
    category = "root menu/build",
    deps = { "examples", "wsi" },
    after_check = function(option)
        if option:dep("examples"):enabled() then option:enable(option:dep("wsi"):enabled()) end
    end,
})
option("examples_log", {
    default = false,
    category = "root menu/build",
    deps = { "examples", "log" },
    after_check = function(option)
        if option:dep("examples"):enabled() then option:enable(option:dep("log"):enabled()) end
    end,
})
option("examples_entities", {
    default = false,
    category = "root menu/build",
    deps = { "examples", "entities" },
    after_check = function(option)
        if option:dep("examples"):enabled() then option:enable(option:dep("entities"):enabled()) end
    end,
}) --option:enable(true) end end })
option("examples_lua", {
    default = false,
    category = "root menu/build",
    deps = { "examples", "lua" },
    after_check = function(option)
        if option:dep("examples"):enabled() then option:enable(option:dep("lua"):enabled()) end
    end,
})
option("examples", { default = true, category = "root menu/build" })
option("tools", {
    default = false,
    category = "root menu/build",
})
option("tests", { default = false, category = "root menu/build" })
option("sanitizers", { default = false, category = "root menu/build" })
option("mold", { default = false, category = "root menu/build" })
option("lto", { default = false, category = "root menu/build" })
option("shared_deps", { default = false, category = "root menu/build" })
option("on_ci", { default = false, category = "root menu/build" })
option("rad", { default = false, category = "root menu/build" })

---------------------------- module options ----------------------------
option("log", { default = true, category = "root menu/modules" })
option("math", { default = false, category = "root menu/modules" })
option("entities", { default = false, category = "root menu/modules" })
option("image", { default = false, category = "root menu/modules", deps = { "log" } })
option("wsi", { default = false, category = "root menu/modules", deps = { "log" } })
option("gpu", { default = false, category = "root menu/modules", deps = { "log", "image", "wsi" } })
option("lua", { default = false, category = "root menu/modules", deps = { "log" } })

option("compile_commands", { default = false, category = "root menu/support" })
option("vsxmake", { default = false, category = "root menu/support" })

option("devmode", {
    category = "root menu/support",
    deps = { "tests", "examples", "mold", "sanitizers" },
    after_check = function(option)
        if option:enabled() then
            for _, name in ipairs({ "tests", "examples", "mold", "sanitizers" }) do
                option:dep(name):enable(true)
            end
        end
    end,
})
