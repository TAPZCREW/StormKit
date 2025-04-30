target("test-base", function()
    set_group("tests")
    set_kind("object")
    set_languages("cxxlatest", "clatest")
    add_rules("stormkit.flags")
    add_rules("windows.subsystem.console")
    add_files("src/main.cpp")
    add_files("src/Test.mpp", { public = true })
    add_options("sanitizers")
    add_packages("frozen")
    add_deps("stormkit-main")
end)

for name, _ in pairs(modules) do
    if has_config("tests_" .. name) then
        for _, file in ipairs(os.files(path.join("src", name, "**.cpp"))) do
            local testname = path.basename(file)
            target(name .. "-" .. testname .. "-tests", function()
                set_group("tests")
                set_kind("binary")
                set_languages("cxxlatest", "clatest")

                on_config(function(target)
                    function parseTestFile()
                        local code = io.readfile(path.join("tests", file))

                        local suite_name_regex = [[TestSuite%s-{.-"(.-)",]]
                        local test_name_regex = [[{%s-"(.-)"%s-,]]

                        local suite_name = code:match(suite_name_regex)

                        local test_names
                        for test_name in code:gmatch(test_name_regex) do
                            test_names = test_names or {}
                            if test_name ~= suite_name then table.insert(test_names, test_name) end
                        end

                        return { suite_name = suite_name, test_names = test_names }
                    end

                    local tests = parseTestFile()
                    for _, test_name in ipairs(tests.test_names) do
                        target:add(
                            "tests",
                            tests.suite_name .. "/" .. test_name,
                            { group = tests.suite_name, runargs = "--test_name=" .. test_name }
                        )
                    end
                end)

                add_rules("stormkit.flags")
                add_rules("windows.subsystem.console")

                add_files(file)

                add_packages("frozen")
                add_deps("stormkit-main")
                add_deps("stormkit-" .. name)

                add_options("sanitizers")

                add_deps("test-base")
            end)
        end
    end
end
