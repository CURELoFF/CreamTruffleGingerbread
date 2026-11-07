# 🍦 CreamTruffleGingerbread — 项目初始化实战教程

> **定位**：C++20 · SDL3 · CMake + vcpkg + Ninja · 打造系统级别 C++ 游戏框架
>
> **适合谁**： 本科生、想系统学习编程/游戏开发底层机制的人
>
> **核心理念**： 不是"能跑就行"，而是理解每一层工具链背后的原理，建立可持续扩展的工程结构

---

# 🔍 第一部分：项目概览

## 技术栈总览

| 层级 | 工具 | 版本 | 作用 |
| --- | --- | --- | --- |
| 语言 | Modern C++ | C++20 | 核心开发语言 |
| 编译器 | MSVC (Visual Studio 2022) | 19.44 | Windows 原生编译器 |
| 构建系统 | CMake | 4.4.5 | 构建配置的"配方翻译官" |
| 增量构建 | Ninja | 1.13.2 | 快速、监督式的底层施工队 |
| 包管理 | vcpkg | VS 内置 | 自动管理三方库（SDL3 等） |
| 多媒体库 | SDL3 | 3.4.12 | 窗口、输入、音频、渲染接口 |
| 编辑器 | Zed | - | 配合 clangd 获得智能感知 |
| 版本管理 | Git | 2.54.0 | 代码版本控制 |

## 项目结构现状

```text
CreamTruffleGingerbread/
├── .git/                      # Git 仓库数据
├── .gitattributes             # Git 换行符等文件属性配置
├── .gitignore                 # 忽略 build/、vcpkg_installed/ 等可再生产物
├── .clangd                    # clangd 语言服务器的项目配置（指向编译数据库）
├── CMakeLists.txt             # CMake 主配方：目标=可执行程序、依赖=SDL3
├── CMakePresets.json          # CMake 预设：一键配置 Debug/Release（含 vcpkg 工具链）
├── vcpkg.json                 # vcpkg 清单：项目依赖声明（SDL3）与基线锁定
├── DEV_CHEAT_SHEET.md         # 你自己整理的开发速查表 👏
├── build/                     # [不进 Git] 构建产物（Ninja、.obj、.exe、.dll）
│   └── debug/
│       ├── compile_commands.json  # 编译数据库（clangd 的"指路牌"）
│       ├── CreamTruffleGingerbread.exe
│       └── SDL3.dll               # applocal.ps1 自动从 vcpkg 复制！
├── vcpkg_installed/           # [不进 Git] vcpkg 安装的库（可再生）
└── src/
    └── main.cpp               # 入口：SDL 版本诊断 + 窗口初始化
```

---

# 🧠 第二部分：核心概念——构建系统全家桶

## g++ 单文件编译 vs 工程化构建

### g++ 一条命令背后发生了什么

```text
g++ main.cpp -o main.exe  =  预处理 → 编译 → 汇编 → 链接
```

1. **预处理 (Preprocess)** — 递归展开所有 `#include`、替换宏。SDL3 头文件展开后是几万行代码
2. **编译 (Compile)** — 把单个 .cpp（翻译单元）变成目标文件 `.obj`，**此时还不知道其他文件的函数在哪**
3. **汇编 (Assemble)** — 生成目标文件
4. **链接 (Link)** — 把所有 `.obj` 和第三方库 `.lib` 拼在一起，**解决"函数在哪"的悬空引用**，生成 exe

### 为什么工程不能用 g++ 手敲？

| 失效点 | 后果 |
| --- | --- |
| 依赖爆炸 | 手写 -I/-l 路径，路径一变全线崩塌 |
| 无增量构建 | 100 个 .cpp 改 1 个，全量重编 5 分钟 |
| 协作不可能 | 每人维护一份一样的超长命令 |

### 工具链角色分工（⭐ 核心心智模型）

```text
你写的代码
    ↓
CMakeLists.txt       — "意图" 层（想要什么程序、依赖谁）
    ↓  翻译
build.ninja          — "计划" 层（谁先编译、谁依赖谁、跳过没改的）
    ↓  执行
MSVC 编译器/链接器    — "施工" 层（真正干活）
    ↓
窗口/DLL/可执行文件
```

**CMake** = 配方翻译官（人类可维护 → 机器可执行）
**Ninja** = 增量执行器（只重编受影响部分，秒级）
**vcpkg** = 库的自动采购+安装+对接系统（一行声明 SDL3，它搞定下载、编译、路径对接）

---

# 🌱 第三部分：逐步复盘 — 项目初始化全过程

## Step 0 · Git 初始化与规范

- `main` 分支、规范 `.gitignore`（忽略 `build/`、`vcpkg_installed/`、VS/clangd 缓存）
- 提交命名规范：`Empty Project Setup` → `Project can be built now` → `First SDL3 diagnostics program`
- **原则**：可再生产物不进 Git，源代码和配置进 Git

## Step 1 · `CMakeLists.txt` 逐行拆解

```cmake
cmake_minimum_required(VERSION 3.20)     # 项目支持的最低 CMake 版本（约等于"生成器/Presets 支持"门槛）

project(CreamTruffleGingerbread
    VERSION 0.1.0
    LANGUAGES CXX                        # 只用 C++（不需要 C）
)

set(CMAKE_CXX_STANDARD 20)               # C++ 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)      # 编译器不达标就报错，不要静默降级
set(CMAKE_CXX_EXTENSIONS OFF)            # 禁用编译器方言后缀(-std=c++20 而非 gnu++20)

add_executable(CreamTruffleGingerbread   # 定义目标：一个可执行文件
    src/main.cpp                         # 编译所需源文件列表
)

find_package(SDL3 CONFIG REQUIRED)       # 按 Config 模式寻找 SDL3 包，找不到则硬失败
target_link_libraries(CreamTruffleGingerbread PRIVATE SDL3::SDL3)
```

### 关键概念

| 概念 | 解释 |
| --- | --- |
| **Config 模式 vs Module 模式** | Config 模式找 `XxxConfig.cmake`（库作者随库写的"说明书"）；Module 模式找 `FindXxx.cmake`（社区手写的查找脚本）。vcpkg 走 Config。 |
| **IMPORTED 目标 `SDL3::SDL3`** | "命名空间目标"约定，防止命名冲突。 |
| **Usage Requirements** | SDL3 的说明书里已声明自己的头文件路径和依赖；你 link 它，这些要求自动应用到你的目标上。 |
| **PRIVATE / PUBLIC / INTERFACE** | 可见性传播：依赖只出现在你 .cpp 里 → `PRIVATE`；出现在你暴露的头文件里 → `PUBLIC`；只在对外接口里 → `INTERFACE`。 |

## Step 2 · `vcpkg.json`（Manifest 模式 vs Classic 模式）

```json
{
  "name": "creamtrufflegingerbread",
  "version": "0.1.0",
  "builtin-baseline":"9e593bb18ea69cc5095e012465dcd675a822ed0d",
  "dependencies": [ "sdl3" ]
}
```

- **Manifest 模式**（本项目）：依赖写在 `vcpkg.json` 里，`vcpkg_installed/` 在项目内，随 CMake 自动解析。**推荐**，可移植、可复现。
- **Classic 模式**：`vcpkg install sdl3` 全局安装，不推荐，依赖不可控。
- **`builtin-baseline`**：锁死 vcpkg 官方库的一个 git commit SHA，决定每个包的**默认版本**。**不要随意改/删**，它是可复现构建的锚点。

## Step 3 · vcpkg 网络排障实录 🔥（最有教学价值的部分）

### 症状
- `vcpkg install` 卡在 "Fetching registry information..." 十分钟无响应
- `git ls-remote https://github.com/microsoft/vcpkg` 报 `fatal: expected flush after ref listing`
- 浏览器却能正常访问 GitHub

### 排查链路（一步步排除法）

1. **TCP 连层面**：`Test-NetConnection github.com -Port 443` 反复执行，4 次里 1 次失败 → **不是稳定网络**
2. **raw HTTP 层面**：`curl.exe -v` 直接模拟 git 协议首步，发现 TLS 握手前就 `Connection timed out` → **不是 Git/vcpkg 的问题**
3. **本地缓存层**：检查 `C:\Users\CURELOFF\AppData\Local\vcpkg\registries\git`，发现 0 commit + 3 个 14KB 残缺 `tmp_pack` → **是症状不是原因**
4. **代理环境**：读系统注册表 `HKCU:...\Internet Settings`，得 `ProxyServer=127.0.0.1:10081`，浏览器在用

### 真相 🎯

> **系统代理靠"程序主动读取注册表"才能生效**。浏览器会读，Git/libcurl/vcpkg 不读认 `HTTP(S)_PROXY` 环境变量——它们根本不知道代理存在，全部走直连。
> 直连线路对 GitHub 的 `git 协议` 流量做了**间歇性干扰**（网了半个 pack 就断），产生残缺 `tmp_pack`。

### 验证与修复

```powershell
# 仅会话内生效，验证用
$env:HTTPS_PROXY = "http://127.0.0.1:10081"
$env:HTTP_PROXY  = "http://127.0.0.1:10081"
vcpkg install --triplet x64-windows     # ✅ 成功，1.9 分钟装好 SDL3
```

### 终极方案：TUN 模式（关闭系统代理，让软件无感）

> **系统代理** = 应用层拦截，程序"配合才生效"
> **TUN 模式** = 网络层创建虚拟网卡，系统级别"流量一律接管"，**任何程序都无感好用**
>
> 切换到 TUN 后，Git/vcpkg/cargo/pip 一切"不理会系统代理"的工具都自动走好路由。**推荐所有开发环境一步到位用 TUN**。

### 学到的方法论 📚

1. **分层排查**：TCP → TLS → HTTP → 应用，一步步缩小
2. **症状 ≠ 原因**：残缺 `tmp_pack` 是结果，不是原因
3. **善用环境变量做诊断**：`ProxyServer` 就在注册表 `HKCU:\...\Internet Settings` 里，一查便知系统代理真相

## Step 4 · `CMakePresets.json`（为什么值得加）

### 核心思想

> **Presets = 把"构建配方"写成版本化的项目内文件** —— 团队/CI/未来的你，用同一条 `cmake --preset debug`，获得完全一致的结果（可复现构建）。

```json
{
  "version": 3,                                // Presets 文件本身格式版本(v3 从 CMake 3.20 起支持)
  "configurePresets": [
    {
      "name": "base", "hidden": true,          // 隐藏预设只作为公共模板基底(DRY 原则)
      "generator": "Ninja",                    // 底层构建工具
      "binaryDir": "${sourceDir}/build/${presetName}",  // build/debug、build/release 分置
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",  // ⭐ vcpkg 与 CMake 的接口点
      "cacheVariables": {
        "VCPKG_TARGET_TRIPLET": "x64-windows"   // 64位、动态链接运行库
      }
    },
    {
      "name": "debug",
      "inherits": "base",                       // 继承公共配置
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug" }
    }
  ],
  "buildPresets": [{ "name": "debug", "configurePreset": "debug" }]
}
```

| 字段 | 意义 |
| --- | --- |
| `toolchainFile` | **vcpkg 进入 CMake 的唯一接口**，它读取 vcpkg.json → 自动 install → 注入库路径给 find_package |
| `$env{...}` | 读环境变量，不含本机绝对路径 → **可持续移植** |
| `binaryDir` | Debug/Release 分目录，隔离缓存，一行 `build/` 全部 ignore |
| `Ninja` 单配置 | CMAKE_BUILD_TYPE 必须在 preset 中定死（不同于 VS 的多配置 `--config`） |

## Step 5 · SDL3 接入与诊断程序

### SDL3 API 的改变（相对 SDL2）

| 领域 | SDL2 | SDL3 |
| --- | --- | --- |
| 版本号 API | `SDL_version` 结构体 | **已移除**，改为 `int SDL_GetVersion()` 返回打包整数 |
| 错误返回风格 | `0` = 成功，负数 = 失败 | 大量改为返回 `bool`（`true`= 成功） |
| main 处理 | `SDL_main` 单独链接 | `SDL_main.h` 全家桶（**控制台程序不要 include**） |

### 诊断程序要点

```cpp
const int version = SDL_GetVersion();
// SDL3 打包规则: major * 1_000_000 + minor * 1_000 + patch
// 3.4.12 → 3004012   (Bit/Field Packing 常用于游戏代码: RGB、状态位)

if (!SDL_Init(SDL_INIT_VIDEO)) {           // false = 失败(SDL3 新风格)
    std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';   // 线程局部错误通道
    return EXIT_FAILURE;
}
SDL_Quit();                                 // 与 SDL_Init 对称，RAII 思想雏形
```


### 运行时依赖问题（DLL 机制学到的一课）

- `SDL_Init` 能跑是因为 **vcpkg 工具链在链接后自动运行 `applocal.ps1`**，把 `SDL3.dll` 从 `vcpkg_installed/.../debug/bin/` **复制到 exe 旁**
- 验证方式：看运行输出版本 3.4.12 与 vcpkg 安装版本一致 → 链接的是"我们装的那份"

## Step 6 · 编辑器集成（Zed + clangd）

- **核心认知**: 编辑器红线 ≠ 编译器。真正说了算的是构建结果。
- **clangd** 需要一份编译数据库 `compile_commands.json`，记录每个源文件的完整编译参数（-I 路径、-std 等）
- CMake + Ninja 可自动生成（`CMAKE_EXPORT_COMPILE_COMMANDS: ON` 加到 presets 里）
- `.clangd` 配置文件告诉 clangd 去哪儿找数据库：

```yaml
CompileFlags:
  CompilationDatabase: build/debug
```

- Zed 重启 clangd:`Ctrl+Shift+P` → `editor: restart language server`

---

# 🛠 第四部分：你的日常开发循环（背下这四行）

```powershell
# 在 x64 Native Tools Command Prompt for VS 2022 中
cd E:\!Codes\Cpp_toolchained_game\CreamTruffleGingerbread

cmake --preset debug            # 配置：CMakeLists/vcpkg.json/CMakePresets.json 变了才跑
cmake --build --preset debug    # 构建：改了 src/** 代码就跑（增量，秒级）
.\build\debug\CreamTruffleGingerbread.exe    # 运行
```


## 常用扩展

```powershell
cmake --build --preset release          # Release 优化版
cmake --build --preset debug --target clean    # 清空
vcpkg list                              # 查看已装的包（在 VCPKG_ROOT 内调用）
```

---

# 📖 第五部分：概念词典（持续更新）

| 术语 | 一句话解释 |
| --- | --- |
| **翻译单元** | 一个 .cpp 展开所有 #include 后的完整内容，编译的基本单位 |
| **链接器** | 合并 .obj/.lib、解决跨文件符号引用，生成 exe 的最后一环 |
| **三元组 (Triplet)** | vcpkg 的"目标平台+ABI 命名"，x64-windows = 64位动态链接 |
| **工具链文件** | CMake 在感知编译器之前加载的钩子，vcpkg 靠它注入路径 |
| **打包整数 (Packing)** | 把多个字段压进一个 int 的技巧（版本号、颜色、状态位） |
| **位掩码标志** | `SDL_INIT_VIDEO | SDL_INIT_AUDIO` 并列标志，用 OR 合成 |
| **RAII** | 资源在构造时获取、析构时释放，靠作用域保证不泄漏 |
| **使用要求传播** | 目标把自己的 include/lib 要求自动传导给依赖它的目标 |

---

# 🗺 第六部分：未来路线图

- [ ] **阶段 1**：创建窗口 + 事件循环读取 `SDL_QUIT`（游戏心脏"事件循环"概念）
- [ ] **阶段 2**：第一个真正的模块划分（`core/`、`platform/`，头文件与实现分离）
- [ ] **阶段 3**：`.clang-format` 统一代码风格
- [ ] **阶段 4**：渲染（`SDL_Renderer` 基础屏幕绘制）
- [ ] **阶段 5**：输入系统（键盘/鼠标事件抽象层）
- [ ] **阶段 6**：资源管理（纹理、音频的加载与生命周期）
- [ ] **阶段 7**：帧率控制与游戏主循环（固定时间步进、Delta Time）
- [ ] **阶段 8**：ECS 或组件化架构（游戏对象管理）
- [ ] **阶段 9**：音频（SDL_Audio 子系统与声音事件）
- [ ] **阶段 10**：打包与发布（CI/CD、GitHub Actions 自动构建）

---

# 💎 项目纪律

1. **每一步都提交**，提交信息写明"做了什么"而不是"改了文件"
2. **编辑器红线 ≠ 编译器**，信构建，不信 IDE
3. **改 API 前先读本机头文件**（vcpkg_installed/.../include），SDL3 相对 SDL2 变化大
4. **不随意删除/重写配置**（尤其 vcpkg baseline）
5. **理解优先于抄命令**，每条命令都要知道自己在干什么

---

*本教程由项目实战记录整理，配合 DEV_CHEAT_SHEET.md 使用。持续更新中 🍦*
