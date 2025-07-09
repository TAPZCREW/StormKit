#ifndef EXAMPLES_H
#define EXAMPLES_H

{%
print("Hello world")
%}

{% for i, name in ipairs({ "FOO", "BAR", "FOOBAR" }) do %}
inline constexpr auto {% outfile:write(name .. i) %} = "{% outfile:write(name) %}";
{% end %}

#endif
