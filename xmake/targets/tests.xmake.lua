namespace("tests", function()
    for _, name in ipairs({ "core", "math", "log", "entities", "gpu", "image", "wsi", "lua" }) do
        if name == "core" or get_config(name) then
            for _, file in ipairs(os.files(path.join(os.projectdir(), "tests", name, "**.cpp"))) do
                local testname = path.basename(file)

                target(name .. "-" .. testname, function()
                    add_rules("stormkit::example")

                    on_config(function(target)
                        function parseTestFile()
                            local code = io.readfile(file)

                            local suite_name_regex = [[test_suite%s-{.-"(.-)",]]
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

                    add_files(file)

                    add_deps("stormkit::test")

                    add_options("sanitizers")

                    set_group("tests")
                end)
            end
        end
    end
end)
