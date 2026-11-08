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

- 已完成：构建体系（CMake+vcpkg+Presets+/utf-8）、SDL3 3.4.12、知识文章 01~05/07、`core/Window` + `core/Application`（Subsystem+Window 组合，逆序析构）——**框架第一层骨架落成**（用户已验证安全开关窗）。
- 进行中：文章 07《渲染》已发出待自测 → 实战 `core/Renderer`（RAII №3，Get() 借句柄）+ Application 每帧画第一个矩形；CMakeLists 需追加 `src/core/Renderer.cpp`。
- dev.bat：带 MSVC 环境一键启动 Zed（全 ASCII 版）；用户已可官方三种环境启动任选。模板：`cmd /c "vcvars && cmake --build --preset debug"`。
- 学生确认：代理已从 TUN 切回系统代理模式（速度原因）；**需要网络操作（下载依赖/抓文档）时先提醒用户开 TUN**。
- 路线图（接近）：渲染 → 输入让矩形动起来（408·数据结构）→ 碰撞 → 帧率控制。
- 机器注意：两套 VS 并存（F 盘 14.51 / E 盘 14.44，vcpkg hash 与 CMake 所用不同，无害已记账）；机器相关路径 dev.bat 内硬编码。
- 教学 style：Doxygen 注释（@file/@brief/@param）学生已自发采用；稍后 clang-format 收编风格。
