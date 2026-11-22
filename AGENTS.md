# AGENTS.md — CreamTruffleGingerbread 项目指令

## 项目定位

系统级 C++ 游戏框架学习项目：纯 C++20 + SDL3 + CMake/vcpkg/Ninja，Windows/MSVC。
目标是不用游戏引擎从零构建框架，以系统级理解（408 四科映射）为第一优先级。

## 代码风格约束

- 现代 C++（C++20）：RAII、智能指针、`std::span`、`concepts`、`ranges`；禁止裸 new/delete 与 C 风格陈旧写法。
- SDL 资源必须 RAII 封装；模块单一职责；代码含中文注释、可编译、适合学习者阅读。
- 构建产物（`build/`）与依赖（`vcpkg_installed/`）不进 Git；每个里程碑规范提交。

## 知识库与文档

- 知识文章统一写入 `docs/knowledge/NN-主题.md`（扁平结构，编号递增），并同步更新 `README-知识库索引.md`。
- 长教程放 `docs/`。写作后自查乱码与断句；表格勿含多余行。

## 相关 Skill

教学与工程细节问题优先加载项目 skills：cmake、conan-vcpkg、msvc-cl、dynamic-linking、sanitizers、memory-hierarchy-and-caches、virtual-memory-paging-and-tlb、cpu-cache-opt、memory-model、custom-allocators、cpp-sdl3-systems-tutor（导师规范全文在此 skill 中）。

## 当前状态快照

- 已完成：构建体系（CMake+vcpkg+Presets+/utf-8 + dev.bat）、知识文章 01~10（07 渲染 / 08 输入 / 09 碰撞 / 10 帧率 dt+vsync，自测多数已回收）、`core/Window` + `core/Application`（Subsystem+Window 组合）、`core/Renderer`（RAII+vsync）、`core/Collision.hpp`（header-only AABB）。碰撞实战已验收（敌人瞬移+贴边不算碰撞）；"框架骨架 + 可玩矩形"阶段完成。
- 进行中：13 号四道自测已回收判卷（Q1 半对：override 非开关；Q2 满分；Q3 纯虚=虚函数特化已补录；Q4 vtable 每类一份、阈值修正）。**11/12 号六道自测已于 11/21 回收判卷**：11-1 ✅、11-2 ⚠️（accumulator 补拍/插值补不了空帧；>0.1s 丢弃是防死亡螺旋）、11-3 ⚠️（prev=旧位置；修法 Snap 在逻辑层）、12-1 ✅大半（vector≠栈、翻倍扩容均摊证明）、12-2 ❌（遍历中 push_back 触发扩容→迭代器野指针；规矩：先收集后追加/reserve）、12-3 ⚠️（const auto& 只护指针不护对象；正解 Draw const 化，挂账作 B 阶段顺手小改）。除艺术外**知识账全清**（仅剩《依赖注入与上下文》文章 + 碰撞别名后门 + Draw const 化小改）。学生已决定不装角色卡/人设 skill。**待写文章挂账：字符串与编码（string/string_view 生命周期、UTF-8 vs UTF-16/GBK、filesystem::path 编码转换、SSO）——排在资源管理器或存档系统前后**；11/21 学生问 `string(view).c_str()` 是否危险，已答：同表达式内安全、存指针才悬空、string_view.data() 不保证 null 结尾。**14 号《Surface 与 Texture》文章已于 11/21 发出待读**（含实测 API：SDL_image 3.x 无 IMG_Init、SDL_RenderTexture 改名、IMG_LoadTexture 一步式、pitch/析构顺序/path 坑、408 帧缓冲挂钩；自测 3 题）。**15 号《C++ 初始化语法》语法补给支线已写**（因学生见 `Type name(expr)` 而问四种初始化；下一课精灵动画顺延为 16）。`tools/gen_sprite.ps1` 已写并验证（PowerShell+System.Drawing 程序化生成 16x16→128x128 带 alpha 像素史莱姆；PowerShell 哈希键不区分大小写踩坑已记）。下一步：学生读 14/15 后自写 Surface/Texture RAII 封装（№5/№6）+ 玩家贴图。**游戏类型已定 = 地牢 Roguelike（学生称 Shattered Pixel Dungeon 除美术外即理想型）**；视角 = 2D 俯视（top-down, RPG Maker 风）。
- dev.bat：带 MSVC 环境一键启动 Zed（全 ASCII 版）；用户已可官方三种环境启动任选。模板：`cmd /c "vcvars && cmake --build --preset debug"`。
- 学生确认：代理已从 TUN 切回系统代理模式（速度原因）；**需要网络操作（下载依赖/抓文档）时先提醒用户开 TUN**。
- 路线图（学生已拍板，408 反客为主·项目作实验载体；**11/21 学生选定游戏类型 = C 地牢 Roguelike（回合制·网格）**）：
  A 地基收尾：13 号《多态与虚函数》✅ → 模板课《InterpProperty 泛型插值》✅ → 15 号语法补给 ✅
  B 资源层：Surface/Texture（RAII №5/№6，进行中）→ 精灵图与瓦片图集（srcrect/atlas）
  C 表现层：图层与渲染排序（**排序算法正席**）→ SDL_ttf 文本（消息日志/HUD）→ UI 层（背包/装备界面）
  D 地牢正体（Roguelike）：瓦片地图渲染 → **网格移动与回合制**（edge-triggered 输入 + 能量调度=优先队列）→ 程序化地牢生成（RNG/房间/走廊）→ 视野与光照（FOV/射线）→ 背包/装备（**哈希表**）→ 敌人 AI/寻路（**BFS/A\***）→ 存档（**OS 文件 IO**）
  **表现层视角已定（11/21）**：2D 俯视角（top-down / overhead）+ 3/4 视角角色精灵（three-quarter view），即 RPG Maker 风格；top-down tile-based 2D。参数暂定：瓦片 32×32、角色 32×32 四方向、瓦片图集 srcrect 取帧；等距（isometric）不做。学生已澄清：只借 Roguelike 玩法（程序生成/回合/永久死亡/资源管理），不借 ASCII 表现。
  E 手感升级：空间哈希（多实体邻接查询，09 课前瞻兑现）→ 音频；物理改为可选（回合制不需要运动学，但动画补间仍需插值）
  F 精绘选修：SDL_GPU 着色器（**组成原理 GPU 流水线**）
  设计变化备忘：Roguelike 为离散步进 → 输入需从"按住连续移动"改为"按键触发一步（含 key-repeat）"，可挂 OS 边沿/电平触发中断对照；固定步长+插值仍保留用于动画补间。
  估算：Roguelike 基本功能 ≈25~30 课。深度优先，不赶速度。
- 待提交：docs/knowledge 文章 11+12；12 号实战完成后建议提交：`Add fixed timestep loop, vector entities, and Kinematic interpolation`（连同两篇文章一起）。
- 学生亮点：连续高质量追问到机制层（传递包含/IWYU、常见坑=通缉令读法、inline=ODR 特赦而非速度开关、Present=限帧闸门、墙钟vs单调钟、GameMaker Step/Draw 次数与插值）——多已写入对应文章补录。**新增**：学生自主实验（STEP=1/10 无插值 vs 有插值对照）独立推理出"Update/Render 成本不对称→分离节拍"结论，并提出 RM 手感之问（答：网格量化为主+插值为辅，已答；D 阶段网格移动课时可回引此实验作铺垫）。
- 机器注意：两套 VS 并存（F 盘 14.51 / E 盘 14.44，vcpkg hash 与 CMake 所用不同，无害已记账）；机器相关路径 dev.bat 内硬编码。
- 教学 style：Doxygen 注释（@file/@brief/@param）学生已自发采用；稍后 clang-format 收编风格。
