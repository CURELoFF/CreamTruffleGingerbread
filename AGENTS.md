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

- 已完成：CMake + vcpkg + Presets 初始化、SDL3 诊断程序、主窗口 + 事件循环、clangd 集成、知识文章 01（消息泵）/02（RAII）/03（头文件分离）、OpenCode skills ×11 装入 `.opencode/skills/`、AGENTS.md + opencode.jsonc。
- 进行中：下一课 = 文章 04《CMake 心智模型》+ `core/Window`（RAII 实战，.hpp/.cpp 分离）+ 模块拆分；文章 05（vcpkg）已从关键路径摘除，推迟到装下一个依赖时写。
- 路线图：模块拆分 → Application/主循环 → SDL_Renderer 渲染第一个矩形 → 键盘输入控制对象移动（408 数据结构登场）→ 碰撞 → 帧率控制。
- 待提交：`.opencode/`、`AGENTS.md`、`opencode.jsonc`、`docs/`（01~03 文章）、`DEV_CHEAT_SHEET.md`；建议提交信息 `Add OpenCode skills, AGENTS.md, and knowledge base articles`。
- 学生进度：02 自测已通过（理解所有权优先于释放、RAII 构造/析构自动释放、禁拷贝、unique_ptr 定位）；03 文章已发出，自测未回收；期待"窗口内游戏对象交互"目标。
