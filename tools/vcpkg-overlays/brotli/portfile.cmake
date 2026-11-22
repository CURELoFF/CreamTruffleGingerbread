vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO google/brotli
    REF v${VERSION} # v1.1.0 
    SHA512 f94542afd2ecd96cc41fd21a805a3da314281ae558c10650f3e6d9ca732b8425bba8fde312823f0a564c7de3993bdaab5b43378edab65ebb798cefb6fd702256
    HEAD_REF master
    PATCHES
        install.patch
        pkgconfig.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBROTLI_DISABLE_TESTS=ON
)
vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_fixup_pkgconfig()
vcpkg_cmake_config_fixup(CONFIG_PATH share/unofficial-brotli PACKAGE_NAME unofficial-brotli)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/tools")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/man")

# --- 本地覆盖：跳过 brotli 命令行工具的安装 ---
# 原版端口会调用 vcpkg_copy_tools()，进而调用 vcpkg_copy_tool_dependencies()，
# 后者需要 PowerShell Core (pwsh)。本机 pwsh 是损坏的 dotnet 全局工具 shim，
# 会在 vcpkg 的干净环境中启动失败。
# 我们只需要 brotli 作为压缩库（freetype 的 WOFF2 支持），不需要 brotli.exe，
# 因此移除工具安装步骤。
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/tools")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
