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
- 进行中：13 号《多态与虚函数》文章已发出。**代笔已完成待学生测试**（学生明确请求，今晚休息明日测）：`InterpProperty.hpp` 全量重写（Interpolate 重载前置修 C3861 + 恢复 SDL_FRect 重载 + float 用 std::lerp + 四通道契约注释）+ `EntityPlayer.hpp/.cpp` 改用 `InterpProperty<SDL_FRect>`（删 posX/posY 双权威，命名保留学生起的 collisionRect_，构造收 (worldW,worldH) 注入）+ `Application.cpp` 出生处传 init_win_w/h。已构建通过。学生测试清单：移动平滑/碰撞瞬移无滑行/改小窗口不再穿透。Enemy 保持旧实现（SetCollisionRectPos=事件语义，正确；后续可迁 Snap）。其余挂账：13 号四道自测 + 11/12 号共六道自测（共十道）、《依赖注入与上下文》文章、碰撞别名后门（暂接受）。
- dev.bat：带 MSVC 环境一键启动 Zed（全 ASCII 版）；用户已可官方三种环境启动任选。模板：`cmd /c "vcvars && cmake --build --preset debug"`。
- 学生确认：代理已从 TUN 切回系统代理模式（速度原因）；**需要网络操作（下载依赖/抓文档）时先提醒用户开 TUN**。
- 路线图（学生已拍板，408 反客为主·项目作实验载体）：
  A 地基收尾：13 号《多态与虚函数》（学生自研代码的概念债清单=大纲）→ 模板课《InterpolationProperty 泛型插值》（进行中，学生自研）
  B 资源层：Surface/Texture（RAII №4）+ 文章 06 vcpkg 兑现（**已兑现：sdl3-image 装好，manifest+find_package+链接全链路通过**）→ 精灵/动画帧
  C 表现层：图层与渲染排序（**排序算法正席**）→ SDL_ttf 文本 → UI 层（屏幕/世界空间）
  D 内容层·JRPG 正体：瓦片地图→摄像机→对话系统（队列）→回合制战斗（状态机）→背包/商店→存档（**OS 文件 IO**）
  E 手感升级：简单物理（运动学）→空间哈希（09 课前瞻兑现，**散列**）→音频
  F 精绘选修：SDL_GPU 着色器（**组成原理 GPU 流水线**）
  估算：JRPG 基本功能 ≈25~30 课；+物理 ≈30~36；着色器选修另计。深度优先，不赶速度。
- 待提交：docs/knowledge 文章 11+12；12 号实战完成后建议提交：`Add fixed timestep loop, vector entities, and Kinematic interpolation`（连同两篇文章一起）。
- 学生亮点：连续高质量追问到机制层（传递包含/IWYU、常见坑=通缉令读法、inline=ODR 特赦而非速度开关、Present=限帧闸门、墙钟vs单调钟、GameMaker Step/Draw 次数与插值）——多已写入对应文章补录。**新增**：学生自主实验（STEP=1/10 无插值 vs 有插值对照）独立推理出"Update/Render 成本不对称→分离节拍"结论，并提出 RM 手感之问（答：网格量化为主+插值为辅，已答；D 阶段网格移动课时可回引此实验作铺垫）。
- 机器注意：两套 VS 并存（F 盘 14.51 / E 盘 14.44，vcpkg hash 与 CMake 所用不同，无害已记账）；机器相关路径 dev.bat 内硬编码。
- 教学 style：Doxygen 注释（@file/@brief/@param）学生已自发采用；稍后 clang-format 收编风格。
