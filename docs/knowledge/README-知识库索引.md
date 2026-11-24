# 📚 CreamTruffleGingerbread 知识库索引

> 用途：项目开发中产生的**知识点沉淀**，每篇一个主题，复制到 Notion 长期反复阅读。
> 阅读顺序建议：按编号顺序读，编号即学习时的先后因果。

| # | 主题 | 状态 | 一句话简介 |
| --- | --- | --- | --- |
| 01 | 消息泵与窗口未响应 | ✅ 已完成 | 用删除 PollEvent 的实验理解 GUI 程序的存活动力学 |
| 02 | RAII 与资源所有权 | ✅ 已完成 | 构造时获取、析构时释放，让作用域管理资源 |
| 03 | 头文件与实现分离（从 UE 经验看 C++ 通用规则） | ✅ 已完成 | 翻译单元、声明/定义、#pragma once、include 最小化 |
| 04 | C++ 类初见：构造 / 析构与 explicit（Window 预科） | ✅ 已完成 | 构造/析构语法、explicit、= delete、访问控制 |
| 05 | CMake 心智模型（构建流水线地图） | ✅ 已完成 | 三阶段行程、target 原子、vcpkg manifest 链、presets 存档 |
| 06 | vcpkg 凭什么自动装库 | ⏸️ 推迟（下次引入新依赖时写） | manifest → 基线 → 工具链注入的全链条 |
| 07 | 渲染：从窗口到像素（SDL_Renderer） | ✅ 已完成 | 命令队列、Present 交付、Get 借句柄、生命周期嵌套 |
| 08 | 输入：键盘事件让矩形动起来（408·数据结构） | ✅ 已完成 | 三拍子循环、输入轮询、快照数组 O(1) 寻址、clamp 钳制 |
| 09 | 碰撞：AABB 与"找谁"问题（408·数据结构续篇） | ✅ 已完成 | 四个不等式、碰撞响应一页纸、空间哈希前瞻、header-only 工具 |
| 10 | 帧率控制与 delta time | ✅ 已完成 | dt 汇率模型、单调时钟、vsync 限帧、隧穿预告 |
| 11 | 游戏循环成人式：累加器、固定步长与插值 | ✅ 已完成 | 累加器分账、逻辑 60Hz 固定步长、prev/curr 插值、确定性、隧穿还债 |
| 12 | 多对象容器：vector 登场（408·数据结构正席） | ✅ 已完成 | 顺序表三件套、均摊 O(1)、迭代器失效排雷、const& 借阅、Kinematic 插值封装 |
| 13 | 多态与虚函数：一个指针多种行为 | ✅ 已完成 | virtual/纯虚/override、虚析构、vtable 间接跳、unique_ptr 容器（RAII №4）、move 过户、切片立案 |
| 14 | Surface 与 Texture：像素的上传之路 | ✅ 已完成 | CPU 像素 vs 显存贴图、pitch 对齐、IMG_Load 三段/一步、Texture 认 Renderer、RAII №5/№6、析构顺序 |
| 15 | C++ 初始化语法：括号、花括号与等号 | ✅ 已完成（语法补给支线） | 直接/拷贝/列表初始化、禁窄化、最烦恼解析、聚合初始化、explicit、initializer_list、成员初始化列表 |
| 16 | 精灵图与瓦片图集：一张大图如何变成许多小图 | ✅ 已完成 | srcRect/dstRect 取帧、行列索引=二维数组寻址、图集减少绑定与提升缓存局部性、纹理渗色与最近邻采样、水平翻转省素材 |
| 17 | 精灵表动画与方向帧：让静止的格子动起来 | ✅ 已完成 | 帧索引+时间+状态三要素、row=方向 col=时间、accumulator 分频换帧（while 补帧、保留余数）、AnimationClip 数据驱动、FSM 切状态复位、帧索引绝不可插值、回合制离散步进三方案 |
| 18 | 资源清单 manifest：把布局知识搬出代码 | ✅ 已完成 | 存储载体（constexpr/JSON/Lua）与 schema 正交、单一真相源、startX/startY/stepX/stepY 把"帧往哪排"也交给数据、frameMs 用整数、运行期两道闸门 Validate() + ValidateFits()、nlohmann/json 只许出现在 loader.cpp、实战复盘：真实素材是转置的 |

> 未完待续——每完成一个里程碑，这里会多一行。
