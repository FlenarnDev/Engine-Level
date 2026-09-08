-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibf4")

-- set project
set_project("Cascadia-Engine-Level")
set_version("1.0.0")
set_license("GPL-3.0")

-- set defaults
set_languages("c++23")
set_warnings("allextra")

-- set policies
set_policy("build.optimization.lto", true)
set_policy("package.requires_lock", true)

-- add rules
add_rules("mode.debug", "mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

-- targets
target("slimdetours")
    set_kind("static")
    set_languages("c17")
    add_files("lib/slimdetours/Source/KNSoft.SlimDetours/*.c")
    add_includedirs(
        "lib/slimdetours/Source/KNSoft.SlimDetours",
        "lib/knsoft-ndk/Source/Include",
        "lib/knsoft-ndk/Source/Include/KNSoft/NDK/3rdParty/terminal/dep/Console",
        -- stub for <ntlpcapi.h>
        "lib/slimdetours-shim",
        { public = true }
    )
    add_defines("_USE_KNSOFT_NDK", { public = true })
    add_syslinks("ntdll", { public = true })

target("Cascadia-Engine-Level")
    -- add dependencies to target
    add_deps("commonlibf4", "slimdetours")

    -- add commonlibsse plugin
    add_rules("commonlibf4.plugin", {
        name = "Cascadia-Engine-Level",
        author = "Flenarn"
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
