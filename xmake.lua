set_languages("cxx20")
add_rules("mode.debug", "mode.release")

local libs = {"cli11", "cpr", "nlohmann_json", "openssl"}

add_requires(table.unpack(libs))
add_includedirs("src")

target("lzunc")
    set_kind("binary")
    add_packages(table.unpack(libs))    
    add_files("src/*.cpp")

    if is_plat('windows') then
        add_links("ws2_32")
    end
