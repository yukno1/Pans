add_library(pans_options INTERFACE) # 不产生编译产物，只传递编译选项

target_compile_options(pans_options INTERFACE
    # gcc/clang 通用选项
    $<$<OR:$<CXX_COMPILER_ID:GNU>, $<CXX_COMPILER_ID:CLANG>>:
        -Wall
        -Wextra # 额外警告，比Wall更严格
        -Wpedantic # 严格遵循C++标准
        -fno-strict-aliasing # 关闭严格别名检查，避免编译器优化导致的错误
    >
)

target_compile_options(pans_options INTERFACE
    $<$<AND:$<PLATFORM_ID:Linux>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:CLANG>>>:
        -fPIC # 生成位置无关代码
    >
)

target_link_options(pans_options INTERFACE
    $<$<AND:$<PLATFORM_ID:Linux>,$<OR:$<CXX_COMPILER_ID:GNU>, $<CXX_COMPILER_ID:CLANG>>>:
        -rdynamic # 生成动态符号表，允许编译器运行时动态解析，栈回溯需要
    >
)

target_compile_options(pans_options INTERFACE
    $<$<CONFIG:Debug>:PANS_DEBUG> # windows 下，DEBUG被占用
)

target_compile_options(pans_options INTERFACE
    $<$<CONFIG:Debug>:-O0>   # 关闭优化
    $<$<CONFIG:Debug>:-g3>   # 生成更完整的调试信息
    $<$<CONFIG:Debug>:-ggdb> # 生成适合gdb调试信息
)

# ==== Release / RelWithDebugInfo 共用 ====
target_compile_options(pans_options INTERFACE
    $<$<CONFIG:Release>:-DNDEBUG>                # 加 NDEBUG 宏，关闭调试信息
    $<$<CONFIG:Release>:-O2> 
    $<$<CONFIG:Release>:-fno-omit-frame-pointer> # 尽可能保留栈指针

    $<$<CONFIG:RelWithDebInfo>:-DNDEBUG> 
    $<$<CONFIG:RelWithDebInfo>:-O2> 
    $<$<CONFIG:RelWithDebInfo>:-g> 
    $<$<CONFIG:RelWithDebInfo>:-fno-omit-frame-pointer> 
)

option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(pans_options INTERFACE
        $<$<CONFIG:Debug>:--coverage>
    )
    target_link_options(pans_options INTERFACE
        $<$<CONFIG:Debug>:--coverage>
    )
endif()
