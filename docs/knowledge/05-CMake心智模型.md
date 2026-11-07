# 05 · CMake 心智模型——构造自己的"构建流水线"地图

> 背景与上下文：这几天你亲手经历了 vcvars 环境缺失、C1083、自动重配置、vcpkg install 重跑。
> 每个现象背后都有一个机制。本文用**项目里真实的三个文件**，
> 把你已经摸过的碎片拼成一张完整地图。文中所有代码都来自本项目，无一虚构。

---

## 一、一句话核心

> **CMake 不是编译器，是"构建系统的生成器"**：它读你的声明（CMakeLists.txt），
> 产出本地构建脚本（Ninja 规则），真正干活的是 Ninja 调 cl.exe/link.exe。
> 忘掉"写 CMake 就是在写编译命令"——你写的是**声明**，翻译是它的事。

## 二、地图总览：一次 `cmake --preset debug` 背后的三段行程

```text
你手写的 4 份"声明"
   CMakeLists.txt    构建什么、依赖谁（本文主角）
   CMakePresets.json 命令行参数的存档点（见第五节）
   vcpkg.json        我有哪些第三方依赖
   dev.bat/vcvars    喂给编译器的环境变量（先用着，408·OS 课再深挖）
        │
        ▼ ① configure 配置阶段（CMake 作为"解释器"）
        │   - 逐条执行 CMakeLists.txt
        │   - toolchainFile 指路：先跑 vcpkg，装好依赖（manifest 模式）
        │   - 探测编译器、检查 SDL3 能否找到
        │   - 把一切结果记进【缓存】（build/debug/CMakeCache.txt）
        │   产出：build.ninja（机器友好的增量构建规则）
        ▼ ② generate 收尾
        ▼ ③ build 构建阶段（ninja 接手）
            ninja 读 build.ninja → 对每个源文件调 cl.exe 产出 .obj
            → link.exe 把 .obj + SDL3 库 → CreamTruffleGingerbread.exe
```

**配置 ≠ 构建**，这两步你能分开调用：

```powershell
cmake --preset debug          # 只做 ①②：生成/更新 build.ninja（改 CMakeLists 后）
cmake --build --preset debug # 只做 ③：增量编译（日常循环用这个）
```

最近你亲历的 `[0/1] Re-running CMake...` 正是 ③ 检测到"CMakeLists.txt 变了"，
**自动补跑了一遍 ①②**——增量构建系统会盯着配置文件的时间戳，等于自带"忘跑 configure"保险。

## 三、主角登场：CMakeLists.txt 逐行还原（下面就是它的全文）

```cmake
cmake_minimum_required(VERSION 3.20)      # ① 最低版本门槛：低于此版本的 CMake 直接拒绝开工
project(CreamTruffleGingerbread            # ② 声明项目身份：名字、版本 0.1.0、语言 C++
    VERSION 0.1.0
    LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 20)                 # ③ 这些是【全局变量】风格写法（传统层）
set(CMAKE_CXX_STANDARD_REQUIRED ON)        #    - C++20 是硬要求，不是"建议"
set(CMAKE_CXX_EXTENSIONS OFF)              #    - 不要 GNU 扩展，只认标准 C++
add_executable(CreamTruffleGingerbread     # ④ 【核心：创建第一个 target】
    src/main.cpp                           # 文件从哪里来
    src/core/Window.cpp                    # ← 这一行就是你亲手加进去的
)
find_package(SDL3 CONFIG REQUIRED)         # ⑤ 找到 SDL3：CONFIG 模式，必填项（找不到直接报错）
target_link_libraries(CreamTruffleGingerbread PRIVATE SDL3::SDL3)  # ⑥ 声明依赖
```

**两个必须先建立的心智对象**：

### target（目标）——CMake 世界的原子

`add_executable` 创建的东西叫 **target**：可以理解成"一张生产配方"。
这张配方捆住了一组信息：**用哪些源文件、链接哪些库、用什么编译选项**。
我们最近的 `/utf-8` 加的就是这张配方上的一个选项（`target_compile_options`）。

> 现代 CMake 的价值观：**少操作全局变量，多操作 target**。③ 的三行 set 其实是"传统全局风"，
> 更现代的写法是在 target 上用 `target_compile_features(...)` 表达 C++ 标准——
> 纯可执行项目先用全局写法也完全够用，不强扭。

### 依赖的写法到底在说什么

`target_link_libraries(myapp PRIVATE SDL3::SDL3)` 逐块拆：

| 片段 | 含义 |
| --- | --- |
| `SDL3::SDL3` | SDL3 官方提供的**另一个 target**：它自带 include 路径、库路径等全部信息，你不用手动传任何 -I/-L |
| `find_package(... CONFIG REQUIRED)` | 问 vcpkg："帮我找到 SDL3 的 config 文件，找不到就立刻报错" |
| `PRIVATE` | "我需要 SDL3，但**只在我自己内部用**，不把我对 SDL3 的依赖传给将来链接我的人" |

> **PRIVATE/PUBLIC/INTERFACE 是依赖的"传播范围"**，类似信息可见性：
> - PRIVATE：内部使用，不外传
> - PUBLIC：内部用，也随我自己传出
> - INTERFACE：自己不用，纯转发给别人用
> 之后把可执行文件拆成库 target 的时候，这三个字的威力才完全释放，这里先埋种子。

## 四、vcpkg 在流水线里守着哪个岗

还记得那次编译失败日志里的 `-IE:\...\vcpkg_installed\x64-windows\include` 吗？（`-I` 是命令行里手动指定头文件搜索路径的参数）

```text
CMakePresets.json 指定 toolchainFile = $env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
        │
        ▼ configure 阶段第 1 步
vcpkg 读 vcpkg.json（manifest）：你要 sdl3
        │  —— builtin-baseline = 9e593bb... 是"所有依赖版本的集中快照"
        ▼
装到项目专属目录 build/debug/vcpkg_installed/x64-windows/（不动系统、不动全局）
        │
        ▼ 注入给 CMake
"SDL3 的头文件和库文件在这里，编译选项也给你生成好了"
        │
        ▼ 于是 find_package(SDL3 ...) 才能找到 SDL3::SDL3
```

重点记住两件：
1. **manifest 模式 = 依赖清单跟着项目走**，任何人 clone 你的仓库，配置一次就自动装齐——像 UE 里的 `Build.cs` 里声明的依赖被引擎自动处理，只不过现在是明牌；
2. baseline 是版本"冰箱快照"：**将来加新库时写进 vcpkg.json 即可，这也是 06 号文章（暂缓）的实际触发点**。

## 五、CMakePresets.json：命令行参数的"存档点"

没有 presets 的世界是这样的：

```powershell
cmake -B build/debug -G Ninja -DCMAKE_TOOLCHAIN_FILE=... -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug ...
```

每个参数都是一条独立配置，敲错一个就全盘重来。presets 的定义："**把这堆 -D 参数打包成有名字的存档**"。

| CMakePresets.json 字段 | 含义 |
| --- | --- |
| `hidden: true` 的 `"base"` | 通用骨架，不单独给用户用（只能被继承） |
| `inherits: base` | debug/release 只是骨架的特化：仅追加 `CMAKE_BUILD_TYPE` |
| `binaryDir: ${sourceDir}/build/${presetName}` | 每个 preset 有**独立 Build 目录**（debug 和 release 互不污染） |
| `toolchainFile: $env{VCPKG_ROOT}/...` | `$env{}` 表示"读取环境变量"——toolchain 靠它找到 vcpkg 的安装根目录 |
| `buildPresets: configurePreset` | `--build` 用哪个配置存档 |

> `${sourceDir}` 和 `$env{...}` 是两种占位符：前者等于"项目根目录"，后者表示"读取环境变量"——和 cmd 里写 `%VCPKG_ROOT%` 属于同一件事的两种方言。

## 六、用这张地图回看最近的四个现象

| 最近现象 | 地图上的哪个位置 |
| --- | --- |
| `[0/1] Re-running CMake` | ③ 构建前自动补跑 ①②（CMakeLists 变了） |
| vcvars 后才能编译 | ③ 里被调用的 cl.exe 要靠环境变量 `INCLUDE`/`LIB` 找系统头和静态库 |
| C1083 找不到 stdarg.h | `INCLUDE` 缺失：SDL 头靠命令行 -I 找得到，MSVC 自家头靠环境变量找不到 |
| vcpkg install 重跑了一遍 | ① 里 toolchain 的固定动作：检查 manifest，已安装就秒过（你日志里的 364us） |

## 七、常见坑

1. **改了 .hpp 却感觉没生效？** 先确认文件真的保存了（没保存当然不会触发重编）；若怀疑缓存中毒，用 `cmake --preset debug --fresh` 清缓存重来（副作用：vcpkg 会重装一遍依赖）。
2. **缓存是毒也是药**：CMakeCache.txt 会记住一切"上次的结果"（编译器路径、库位置）。换机器/挪目录后 build 目录过期是老坑；**每次全新配置都从干净 build/ 开始**是本项目纪律。
3. **"一台机器两套 VS"现象**：vcpkg 嗅到的编译器可能和 CMake 实际用的不是同一套（我们机器上 F 盘 14.51 vs E 盘 14.44）。通常无害（MSVC 版本间 ABI 兼容），记入"待排查账本"，保持观察。

## 八、概念词典

| 术语 | 一句话解释 |
| --- | --- |
| configure | CMake 读声明、装依赖、探测环境，产出 build.ninja |
| generate | configure 收尾：把规则落成具体产物的环节（与 configure 紧密相邻） |
| build | ninja 执行 build.ninja，增量编译 + 链接 |
| target | 一张生产配方：源文件 + 依赖 + 选项的打包单元 |
| generator | 产出哪种构建系统，Ninja（我们的选择，速度最快） |
| toolchain file | "交叉/环境 injector"：vcpkg 用它在 configure 早期进场 |
| cache | build 目录里的"上次结果"档案（CMakeCache.txt），记忆上次成功/失败 |
| manifest 模式 | vcpkg.json 声明依赖，随项目自动安装，成果在 build/<preset>/vcpkg_installed |
| baseline | vcpkg 依赖版本的"冰箱快照"（一次 commit 引用） |
| PRIVATE / PUBLIC / INTERFACE | 依赖的三种传播范围（当前仅需理解 PRIVATE = 不外传） |
| preset | 一组命令行参数的有名字的存档 |

## 九、记忆锚点

> CMake 声明、Ninja 干活；三个阶段 configure → generate → build。
> target 是原子，PRIVATE 定传播；vcpkg manifest 在 configure 跑腿，
> presets 是命令行的存档。**改完 CMakeLists 忘了 configure？Ninja 会补跑，但别依赖它**。

## 十、自测问题（回收后进入下一实战）

1. "CMake 不编译代码"——那最终谁把 `Window.cpp` 变成 `.obj`？谁把 `.obj` 们变成 `.exe`？
   各自属于地图上的哪个阶段？
2. `find_package(SDL3 CONFIG REQUIRED)` 若去掉 `REQUIRED` 会发生什么，
   后面 `target_link_libraries(... SDL3::SDL3)` 那一行又会各自报什么错？
3. 为什么 `CMakeLists.txt` 变了**不需要**手动重新 configure（构建时 Ninja 自动补跑），
   而 vcvars 环境变量变了却**必须**关掉窗口重开（Zed 继承环境副本发生在进程诞生瞬间）？
   这两个"自动/不自动"的分界线本质是什么？

## 相关文章

- 01《消息泵与窗口未响应》——事件循环地基
- 02《RAII 与资源所有权》—— Window 类的"为什么"
- 03《头文件与实现分离》—— 声明/定义如何跨文件对上号
- 04《C++ 类初见》—— 语法通行证
- 下一篇：模块化实战《第一拆》——main.cpp 改用 core/Window，框架骨架成形（Application 在列）
