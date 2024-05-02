for name, _ in pairs(modules) do
	if has_config("tests_" .. name) then
		target(name .. "-tests")
		do
			on_config(function(target)
				function parseTestFile(filename)
					local code = io.readfile(filename)

					local suite_name_regex = [[TestSuite%s-{.-"(.-)",]]
					local test_name_regex = [[{%s-"(.-)"%s-,]]

					local suite_name = code:match(suite_name_regex)

					local test_names
					for test_name in code:gmatch(test_name_regex) do
						test_names = test_names or {}
						if test_name ~= suite_name then
							table.insert(test_names, test_name)
						end
					end

					return { suite_name = suite_name, test_names = test_names }
				end

				for _, file in ipairs(os.files(path.join("tests", "src", name, "**.cpp"))) do
					target:add("files", file)

					local tests = parseTestFile(file)

					for _, test_name in ipairs(tests.test_names) do
						target:add(
							"tests",
							tests.suite_name .. "/" .. test_name,
							{ group = tests.suite_name, runargs = "--test_name=" .. test_name }
						)
					end
				end
			end)
			set_group("tests")
			set_kind("binary")
			set_languages("cxxlatest", "clatest")

			add_files("src/main.cpp", path.join("src", name, "**.cpp"), "src/Test.mpp")

			if has_config("mold") then
				add_ldflags("-Wl,-fuse-ld=mold")
				add_shflags("-Wl,-fuse-ld=mold")
			end

			add_deps("stormkit-main")
			add_deps("stormkit-" .. name)
		end
		target_end()
	end
end
