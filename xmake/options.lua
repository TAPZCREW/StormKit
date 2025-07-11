---------------------------- global options ----------------------------
option("examples_engine", {
    default = false,
    category = "root menu/others",
    deps = { "examples" },
    after_check = function(option)
        if option:dep("examples"):enabled() then option:enable(true) end
    end,
})
option("examples_gpu", {
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
option("tools", {
    default = false,
    category = "root menu/others",
})
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
option("shared_deps", { default = false, category = "root menu/build" })
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

option("devmode", {
    category = "root menu/others",
    deps = { "tests", "examples", "compile_commands", "mold", "sanitizers", "engine", "mode" },
    after_check = function(option)
        if option:enabled() then
            for _, name in ipairs({ "tests", "examples", "compile_commands", "mold", "sanitizers" }) do
                option:dep(name):enable(true)
            end
            for _, name in ipairs({ "engine" }) do
                option:dep(name):enable(false)
            end
        end
    end,
})
