# 18 · 资源清单 manifest：把布局知识搬出代码

> 前置：16 号（`srcRect` 取哪一格）+ 17 号（什么时候换格）。
> 本文回答一个问题：**"第一行第三列那块 32×32" 这件事，应该写在代码里，还是写在数据里？**

---

## 一、这是什么

### 1.1 起因：`32` 这个东西已经在代码里出现了很多次

接 17 号落地时，你会写下这样的代码：

```cpp
constexpr int FRAME_W = 32;
constexpr int FRAME_H = 32;
constexpr int COLS    = 4;

SDL_FRect SrcRect(int row, int col) {
    return { col * 32.f, row * 32.f, 32.f, 32.f };   // ← 又是 32
}
```

能跑。但下面这些问题里，每一个都是"知识被复制到多处"造成的：

| 场景 | 现在要改几处 |
| --- | --- |
| 美术把表从 4 列改成 6 列（为了放 6 帧攻击） | `COLS`、`gen_player_sheet.ps1` 的 `$COLS`、文件名里的 `4x4`、可能还有硬编码的 `128` |
| 某个动画只有 3 帧，不是 4 帧 | 只能靠调用方传对 `frameCount`，写错不报错 |
| 加第二个角色（史莱姆，5 行） | 整个 `SrcRect` 函数要复制一份 |
| 想知道"这张表到底有几种动画" | 得读代码 `switch` |

**一条知识（这张图怎么切），被拆散在常量、函数、枚举、生成脚本、文件名五个地方。** 这就是要解决的问题。

### 1.2 manifest 是什么：一张图的"身份证 + 分割说明"

manifest（清单）的原意是货运单：**里面写的不是货，而是货怎么装的**。

在游戏里，它的标准名叫 **asset manifest / atlas descriptor / sprite descriptor**，内容是：

```text
这张图在哪 → 每帧多大 → 一共有几种动画 → 每种动画从哪开始、几帧、每帧多久、要不要循环
```

注意它**不含**任何画面像素，也不含任何算法。它只描述"事实"。

业界同类物：

| 工具 | 文件 | 说明 |
| --- | --- | --- |
| Aseprite | `hero.png` + `hero.json` | 同目录同名的 sidecar（边车文件） |
| TexturePacker | `atlas.png` + `atlas.plist` / `.json` | 支持不规则打包，所以每条都写全 rect |
| Unity | `hero.png` + `hero.png.meta` | 引擎自动维护 |
| Godot | `hero.png` + `hero.png.import` | 导入参数 |
| Unreal | `.uasset` | 打包进二进制，你在 UE 里见过 |

**你提的"同文件夹下同名"就是这个 sidecar 约定**，方向完全正确。

### 1.3 三个术语先钉死

| 术语 | 英文 | 含义 |
| --- | --- | --- |
| 资源清单 | manifest / descriptor | 描述资源布局的数据文件 |
| 模式 | schema | 清单里有哪些字段、什么类型、什么单位 |
| 片段 | clip | 一段完整动画：起点、帧数、帧时长、是否循环 |
| 边车文件 | sidecar | 与主文件同目录同名的附属文件（`a.png` + `a.json`） |

### 1.4 关键认知：你在问的其实是两个问题

这是本文最重要的一句话：

```text
问题 A：数据存在哪里？        → 编译进 exe（constexpr）  vs  磁盘文件（json/yaml）
问题 B：数据长什么样？        → 字段、单位、嵌套、命名
```

**A 和 B 完全正交。** `constexpr` 和 `json` 都属于 A，它们是同一个角色的两个选项；B 才决定你写多少行、会不会写错。

所以本项目的决策是：

```text
B：按工业界的样子设计（能原样搬进 JSON）
A：评估过 constexpr，最终直接选了 JSON
```

> 为什么跳过 `constexpr`？因为需求已经足够确定（`vcpkg.json` 加一行依赖），而 `constexpr` 版写完还得整体改写成运行期结构——等于把同一件事做两遍。**分层设计的意义正在于此：换了 A，`SrcRect()`、`SpriteAnimator`、`Draw` 一行都不用改**，这一条在实战里被验证了。

还有第三个载体：**Lua**。它既是文本文件，又能写函数，看起来是"两边好处都要"。它值得单独一节讨论——见 §3.7，结论是：**现在不用，等有"行为"要描述时再用**。

---

## 二、为什么在游戏里重要

### 2.1 单一真相源（Single Source of Truth）

"32" 出现在 6 个地方时，你并没有 6 份真相，你只有**一个真相和 5 个随时会过期的副本**。

manifest 的作用是：把"32"收进**唯一**一个字段，其它所有地方（常量、脚本、文件名）都从它派生。判断标准很简单：

> **改一个数字，需要动几处？** 答案应该是 1。

### 2.2 408 · 操作系统：数据与元数据分离

文件系统里，**文件内容**在数据块中，**文件属性**（大小、时间、权限、数据块位置）在 inode 里。分开存的好处：查属性不用读内容，改内容不影响属性。

manifest 就是图集的 inode：

```text
player.png   ← 数据块（像素）
player.json  ← inode（布局，几百字节）
```

这也解释了为什么美术替换一张图时**清单不用动**——只要尺寸和格数没变，元数据依然正确。

### 2.3 408 · 数据结构：稠密存储 vs 压缩存储

你在两版草案里纠结的"每帧写一遍坐标"vs"起点 + 步长"，就是图论里 **邻接矩阵 vs 邻接表** 的同一个权衡：

| | 显式列表（稠密） | 起点 + 步长（压缩） |
| --- | --- | --- |
| 空间 | 每条 clip 占 N 条记录 | 1 条记录 |
| 取第 i 帧 | 直接读 | 一次乘法 |
| 能表达不规则尺寸 | ✅ | ❌ |
| 手写出错概率 | 高（N 个数字都能写错） | 低（只有起点和帧数） |

表是**程序化生成的严格网格**时，压缩永远赢。等你接 TexturePacker（它会为了省空间把帧打乱塞进图集），就必须用稠密列表。

### 2.4 408 · 组成原理：`.rdata` 与磁盘文件的区别

| | `constexpr` 表 | 外部 JSON |
| --- | --- | --- |
| 存放位置 | exe 的只读数据段 `.rdata` | 磁盘文件，运行时读进堆 |
| 加载时机 | 加载器映射，零成本 | 程序启动后显式读取（文件 IO） |
| 改一个数 | 重编译 + 重链接 | 改文件重启（甚至热重载） |
| 被篡改风险 | 低（改 exe 要重新签名/校验） | 高（文本谁都能改，mod 的入口） |

> 热重载在 OS 上讲就是"文件变化通知"：Windows 的 `ReadDirectoryChangesW`、Linux 的 `inotify`——本质是**中断/事件驱动**，对照 08 号的边沿触发。

### 2.5 数据驱动：谁改数据，谁不需要是程序员

代码里写死 → 美术调一个帧数要找程序员重编译。
数据里写 → 美术自己改文件，程序只负责读。

这就是"数据驱动（data-driven）"的全部含义，也是引擎里 `manifest.json`、`.ini`、`.uasset` 存在的唯一理由。

### 2.6 `constexpr` 的杀手锏：编译期校验

这是 JSON 永远给不了的：

```cpp
static_assert(kPlayerSheet.Valid(), "manifest inconsistent");
```

`startY = 65`（手滑）→ **编译失败**。
`frames = 5` 但只有 4 列 → **编译失败**。
JSON 方案里，这两种错要等游戏跑起来、看到"按上键往左走"或"最后一帧被切开"才发现。

> **把错误的发现时机提前**，是所有工程学科的共同目标：数据库的约束、类型系统、`static_assert`、CI——同一个思想。

> **本文的最终形态没用 `static_assert`。** 载体选了 JSON，编译期校验就无从谈起；它换成了 `SheetDesc::Validate()`（返回错误描述字符串）和 `ValidateFits(w, h)`（拿真实贴图尺寸查越界）。**目标没变，战场变了**：从"编译失败"挪到"启动的第一秒失败"。§3.8 会看到这道闸门实际抓到了什么。

---

## 三、怎么做

### 3.1 先定 schema（B），再选载体（A）

字段清单，只列"改了这个数画面必须跟着变"的：

| 字段 | 类型 | 单位 | 为什么必需 |
| --- | --- | --- | --- |
| `texture` | 字符串 | 路径 | 一张清单对一张图（sidecar 约定） |
| `frameWidth` / `frameHeight` | 整数 | 像素 | 单帧矩形的宽高 |
| `clips[].stepX` / `stepY` | 整数 | 像素 | 每前进一帧走多远——决定帧沿哪个方向排 |
| `clips[].name` | 字符串 | — | 调试打印、查找、将来 JSON 的键 |
| `clips[].startX` / `startY` | 整数 | 像素 | 该片段第一帧在源图上的左上角 |
| `clips[].frames` | 整数 | 格 | 本片段用几格 |
| `clips[].frameMs` | 整数 | 毫秒 | 每帧停留时长（17 号） |
| `clips[].loop` | 布尔 | — | 走路循环，死亡不循环 |

**明确不放进 manifest 的东西：**

| 内容 | 为什么不放 |
| --- | --- |
| 位置、速度、朝向 | 运行时状态，每秒都在变，不是资源属性 |
| 缩放、颜色调制 | 渲染参数，不同实例可能不同 |
| 碰撞盒 | 你有独立的 `CollisionRect`（09 号），且视觉大小 ≠ 碰撞大小 |
| `GetSpriteByFrame` 公式 | **公式属于代码**。数据只提供输入，不描述算法 |
| 调色板、DPI | 属于图像本身或渲染层 |

### 3.2 三个设计决策，每个都有代价

#### ① 用 `startX/startY` 像素坐标，不用 `row` 行号

```text
row: 2        ← 推导：y = row * frameHeight，隐含"行高 == 帧高、行首 x == 0"
startY: 64    ← 事实：直接从 y=64 开始
```

选事实。因为：

- 加 `spacing`（格与格留 1px 防渗色，16 号）后，`row` 公式立刻失效，`startY` 不变。
- 将来接 TexturePacker 输出的是像素坐标，直接照抄。
- "行高一定等于帧高"是**假设**，假设迟早被打破。

#### ② `frameMs` 用整数毫秒，不用 float 秒

```cpp
0.1f 实际是 0.100000001490116119384765625
```

不是不能用，而是**人写的数据不该有这种尾巴**。写 `100` 不会手滑，写 `0.1` 要数小数点。载入时除一次：

```cpp
const float frameTime = clip.frameMs / 1000.0f;   // 误差只发生这一次，可控
```

> 对照 10 号：`dt` 是浮点因为它是**测量值**（每次都不一样）；`frameMs` 是**设计值**（人定的），设计值用整数。

#### ③ 帧沿哪个方向排，也是数据

初稿的 schema 里有个 `columns`，意思是"每行几格"，隐含了一条假设：**帧一定是往右排的**。

```text
横向流（行=方向，列=帧）：  x = startX + i * frameWidth,  y = startY
纵向流（列=方向，行=帧）：  x = startX,                   y = startY + i * frameHeight
```

真实素材把这套假设推翻了（§3.8 是完整复盘）。解法不是加一个 `"axis": "vertical"` 枚举——那只是把"两种排布"写死成二选一，将来出现"隔一格取一帧"又要加字段。真正的最小表达是两个增量：

```json
{ "startX": 0, "startY": 0, "stepX": 0, "stepY": 44, "frames": 4 }
```

| 排布 | `stepX` | `stepY` |
| --- | --- | --- |
| 横向流 | `frameWidth` | `0` |
| 纵向流 | `0` | `frameHeight` |
| 隔一格取一帧 | `2 * frameWidth` | `0` |

**为什么不用"自动绕行"（`x = i % cols`）？** 那个公式同样写死了"行优先"的遍历顺序。而 `i * step` 只是"第 i 帧 = 起点 + i 个步长"，**不含任何方向假设**——极坐标、蛇形排列、甚至一维流式图集（所有帧排成一条长条）都能表达。少一条假设，就少一类永远查不出的 bug。

### 3.3 代码：一份可直接编译的清单

设计稿（假设载体是 `constexpr`）长这样——**最终没有采用这个形态，差异见本节末尾"最终实现"**：

```cpp
/**
 * @file    SpriteSheet.hpp
 * @brief   精灵表清单：描述一张图集怎么切成动画片段（数据，不含算法）。
 */

#pragma once

#include <span>
#include <string_view>

/**
 * @brief 一段动画的静态描述。
 * @note 单位：坐标与尺寸为像素，时长为毫秒。
 */
struct ClipDesc {
    std::string_view name;      ///< 片段名，用于查找与调试打印
    int  startX     = 0;        ///< 首帧左上角 x（源图像素）
    int  startY     = 0;        ///< 首帧左上角 y（源图像素）
    int  frames     = 0;        ///< 本片段占几格
    int  frameMs    = 0;        ///< 每帧停留毫秒（>0）
    bool loop       = true;     ///< true=循环播放，false=播完停住
};

/**
 * @brief 一张精灵表的完整清单。
 * @note 全部字段编译期常量，可在 static_assert 中校验。
 */
struct SheetDesc {
    std::string_view texture;   ///< 图片路径（相对工作目录）
    int frameWidth  = 0;        ///< 单帧宽（像素）
    int frameHeight = 0;        ///< 单帧高（像素）
    int columns     = 0;        ///< 每行格数（表的几何宽度）

    std::span<const ClipDesc> clips;   ///< 全部片段（借用外部数组）

    /**
     * @brief 编译期一致性校验。
     * @return 全部字段合法返回 true；否则 static_assert 会用这个结果报错。
     */
    constexpr bool Valid() const {
        if (frameWidth <= 0 || frameHeight <= 0 || columns <= 0) return false;
        if (texture.empty() || clips.empty()) return false;
        for (const ClipDesc& c : clips) {
            if (c.name.empty())                        return false;
            if (c.frames <= 0 || c.frames > columns)   return false;   // 不许跨行
            if (c.frameMs <= 0)                        return false;   // 防死循环
            if (c.startX < 0 || c.startY < 0)          return false;
            if (c.startX % frameWidth  != 0)           return false;   // 必须格对齐
            if (c.startY % frameHeight != 0)           return false;
        }
        return true;
    }

    /**
     * @brief 按名字查片段。
     * @param[in] name 片段名（如 "walk_down"）。
     * @return 命中返回指针，否则 nullptr。
     * @note 线性查找；片段数是个位数，不值得哈希表（等背包课再上）。
     */
    constexpr const ClipDesc* Find(std::string_view name) const {
        for (const ClipDesc& c : clips) {
            if (c.name == name) return &c;
        }
        return nullptr;
    }
};
```

具体清单 `src/gameobject/PlayerSheet.hpp`：

```cpp
/**
 * @file    PlayerSheet.hpp
 * @brief   玩家的精灵表清单（对应 Assets/player_body_4x4.png，128×128）。
 * @note    行序以 tools/gen_player_sheet.ps1 为准：0=下 1=左 2=右 3=上。
 */

#pragma once

#include "../render/SpriteSheet.hpp"

/// 玩家精灵表的全部动画片段（4 方向 × 4 帧 × 100ms）。
inline constexpr ClipDesc kPlayerClips[] = {
    { "walk_down",   0,  0, 4, 100, true },   ///< 第 0 行：朝下
    { "walk_left",   0, 32, 4, 100, true },   ///< 第 1 行：朝左
    { "walk_right",  0, 64, 4, 100, true },   ///< 第 2 行：朝右
    { "walk_up",     0, 96, 4, 100, true },   ///< 第 3 行：朝上
};

/// 玩家精灵表清单本体。
inline constexpr SheetDesc kPlayerSheet{
    .texture     = "Assets/player_body_4x4.png",
    .frameWidth  = 32,
    .frameHeight = 32,
    .columns     = 4,
    .clips       = kPlayerClips,
};

// 编译期闸门：坐标写歪、帧数越界、时长写 0，都在这里被拦下
static_assert(kPlayerSheet.Valid(), "player sheet manifest is inconsistent");
```

三个语法点，都是你以后天天要用的：

| 写法 | 作用 | 为什么必须这样 |
| --- | --- | --- |
| `inline constexpr` | 头文件里定义变量，全局只有一份 | `constexpr` 变量默认有**内部链接**（每个翻译单元一份，地址不同）；`inline` 让它变成外部链接且可多重定义——这是 C++17 为头文件变量专门加的（03 号的 ODR） |
| `.texture = ...` | **指定初始化器**（designated initializer，C++20） | 字段多时按名字赋值，顺序写反编译器直接报错，比位置初始化安全 |
| `static_assert(...)` | 编译期断言 | 参数必须是常量表达式，所以只能校验 `constexpr` 数据——这正是选它的理由 |

#### 最终实现：换了载体，结构跟着换

真动手时你跳过了 `constexpr`，直接上了 JSON。**载体一换，类型设计必须跟着换**，一共四处：

| 设计稿（`constexpr`） | 最终实现（JSON 载入） | 为什么必须换 |
| --- | --- | --- |
| `std::string_view name` | `std::string name` | 解析用的 JSON 文档用完即销毁，`string_view` 指向的字符立刻悬空 |
| `std::span<const ClipDesc> clips` | `std::vector<ClipDesc> clips` | 数组有几条要到运行期才知道 |
| `columns` | `stepX` / `stepY` | 真实素材是转置的，见 §3.8 |
| `constexpr bool Valid()` | `std::string Validate()` | 没有编译期数据了，改成"返回错误描述，空串=通过" |

```cpp
// src/render/SpriteSheet.hpp（节选）
struct ClipDesc {
    std::string name;     ///< 片段名
    int  startX  = 0;     ///< 首帧左上角 x（源图像素）
    int  startY  = 0;     ///< 首帧左上角 y（源图像素）
    int  stepX   = 0;     ///< 每前进一帧 x 的增量
    int  stepY   = 0;     ///< 每前进一帧 y 的增量
    int  frames  = 0;     ///< 本片段几帧
    int  frameMs = 0;     ///< 每帧停留毫秒
    bool loop    = true;  ///< 是否循环
};

struct SheetDesc {
    std::string texture;              ///< 图片路径
    int frameWidth  = 0;              ///< 单帧宽
    int frameHeight = 0;              ///< 单帧高
    std::vector<ClipDesc> clips;      ///< 全部片段

    std::string Validate() const;                 // 只看清单是否自洽
    std::string ValidateFits(int w, int h) const; // 对照真实贴图尺寸查越界
    const ClipDesc* Find(std::string_view name) const;
};
```

**为什么 `Validate()` 返回字符串而不是 `bool`？**

```cpp
if (!sheet.Validate())                        // 只知道"错了"
if (auto e = sheet.Validate(); !e.empty())    // 知道错在 clips[1] ("walk_left") 的 startY
```

错误信息是写给**三个月后已经忘了这里怎么写的你**看的。代价是几行 `std::to_string`，收益是省掉一整轮 `printf` 调试。

> 那 `inline constexpr` 和 `static_assert` 白学了？没有。上面那张表的结论——"数据与算法分离"——在 JSON 方案里换了个形态继续工作：`Validate()` 就是**搬到运行期的 `static_assert`**，战场从"编译失败"挪到了"启动第一秒失败"。

### 3.4 唯一一处公式：manifest → `srcRect`

```cpp
/**
 * @brief 由清单 + 片段 + 帧索引算出源矩形。
 * @param[in] sheet      所属表的清单。
 * @param[in] clip       当前片段。
 * @param[in] frameIndex 片段内帧号，须在 [0, clip.frames) 内（上游保证）。
 * @return 可直接交给 SDL_RenderTexture 的源矩形。
 */
inline SDL_FRect SrcRect(const SheetDesc& sheet, const ClipDesc& clip, int frameIndex) {
    return SDL_FRect{
        static_cast<float>(clip.startX + frameIndex * clip.stepX),
        static_cast<float>(clip.startY + frameIndex * clip.stepY),
        static_cast<float>(sheet.frameWidth),
        static_cast<float>(sheet.frameHeight),
    };
}
```

三个细节：

- **`static_cast<float>` 不能省**：列表初始化禁止窄化（15 号），`int` 塞进 `float` 字段编译不过。这是好事——它逼你显式承认"像素坐标要变成浮点"。
- **没有 `%`、没有 `-1`、没有范围修补**：`frameIndex` 的边界由 `SpriteAnimator`（17 号）保证。下游不二次修补，出 bug 才能一眼看出是谁没守规矩。
- **`stepX/stepY` 各自乘 `frameIndex`**：横向流是 `(frameWidth, 0)`，纵向流是 `(0, frameHeight)`，将来"隔一格取一帧"是 `(2*frameWidth, 0)`——同一个四行函数全包。

### 3.5 和 `SpriteAnimator` 对接

17 号的 `AnimationClip` 可以退化成"指向 `ClipDesc` 的指针 + 运行时状态"：

```cpp
class SpriteAnimator {
public:
    void Play(const ClipDesc* clip) {          // 传指针：nullptr 表示"查不到片段"
        clip_        = clip;
        frameIndex_  = 0;
        accumulator_ = 0.0f;
        playing_     = (clip != nullptr);      // 查不到就是不能播，别假装在播
    }

    void Update(float dt) {
        if (!playing_ || clip_ == nullptr) return;
        const float frameTime = clip_->frameMs / 1000.0f;   // 设计值 → 秒
        if (frameTime <= 0.0f) return;                      // 双保险
        accumulator_ += dt;
        while (accumulator_ >= frameTime) {                 // while，不是 if
            accumulator_ -= frameTime;                      // 扣减，不是归零
            if (++frameIndex_ >= clip_->frames) {
                if (clip_->loop) frameIndex_ = 0;
                else { frameIndex_ = clip_->frames - 1; playing_ = false; }
            }
        }
    }

    const ClipDesc& Clip() const { return *clip_; }
    int FrameIndex()      const { return frameIndex_; }

private:
    const ClipDesc* clip_ = nullptr;
    int   frameIndex_  = 0;
    float accumulator_ = 0.0f;
    bool  playing_     = false;
};
```

> 注意 `AnimationClip` 里的 `row / frameCount / frameTime` 现在**全部搬进了 `ClipDesc`**。不要再留两份——留着就是两份真相。

`SpriteAnimator` 只**借用** `const ClipDesc*`，不拥有数据：片段的寿命由持有 `SheetDesc` 的一方保证。这条"借用关系"在 `EntityPlayer` 里变成一条硬约束——**成员声明顺序**：

```cpp
SheetDesc      sheet_;     // 必须最先构造：texture_ 要读它的路径
Texture        texture_;   // 必须晚于 Renderer 析构
SpriteAnimator animator_;  // 借用 sheet_ 里的 ClipDesc，指向它的内存
```

C++ 按**声明顺序**初始化，不按构造函数初始化列表的书写顺序。写反了就是"拿着一个还没构造的对象"——`animator_` 指向未初始化的 `sheet_.clips`，`vector` 还没分配内存，取到的是野指针。

> 这也是 `EntityPlayer` 构造函数里那句注释的由来：`texture_(renderer, sheet_.texture)` 能编译、能跑，**只因为 `sheet_` 写在它前面**。把两行调换，是典型的"改了顺序、跑起来乱码"式 bug。

### 3.6 载入层：`nlohmann/json` 只许出现在一个文件里

最终落地时，改动被限制在一层里——这就是"数据与算法分离"兑现的时候：

```text
Assets/player.json ──► SpriteSheetLoader.cpp ──► SheetDesc ──► SrcRect / SpriteAnimator / Draw
                          ↑
                唯一 include <nlohmann/json.hpp> 的地方
```

四类错误，四道闸门，各报各的：

| 层 | 出错时的报错 | 谁负责 |
| --- | --- | --- |
| 语法（少个逗号、括号不配对） | `parse error at line 5, column 11` | JSON 库自带行列号 |
| 结构（缺键、类型不对） | `key 'frameMs' not found` | `doc.at(...)` 抛异常，loader 补上文件名再上抛 |
| 语义（数值不合理） | `clips[1] ("walk_left"): startY is not a multiple of frameHeight` | `SheetDesc::Validate()` |
| 越界（对不上贴图） | `clips[3] ("walk_up"): frame range 120x176 exceeds texture 96x176` | `SheetDesc::ValidateFits(w, h)` |

**为什么 `nlohmann/json.hpp` 只许出现在 `SpriteSheetLoader.cpp`？** 它是个两万多行的 header-only 库。放进头文件，就等于让每个包含 `SpriteSheet.hpp` 的翻译单元都重编两万行——改一行动画逻辑，编译从 2 秒变 20 秒。这是 IWYU 在你自己的代码里制造的第一条边界，也是"头文件应当尽量轻"最直观的理由。

清单本体（`Assets/player.json`，与 `player.png` 同目录同名 = sidecar）：

```json
{
  "texture": "Assets/player.png",
  "frameWidth": 24,
  "frameHeight": 44,
  "clips": [
    { "name": "walk_right", "startX": 0,  "startY": 0, "stepX": 0, "stepY": 44, "frames": 4, "frameMs": 100, "loop": true },
    { "name": "walk_left",  "startX": 24, "startY": 0, "stepX": 0, "stepY": 44, "frames": 4, "frameMs": 100, "loop": true },
    { "name": "walk_down",  "startX": 48, "startY": 0, "stepX": 0, "stepY": 44, "frames": 4, "frameMs": 100, "loop": true },
    { "name": "walk_up",    "startX": 72, "startY": 0, "stepX": 0, "stepY": 44, "frames": 4, "frameMs": 100, "loop": true }
  ]
}
```

> JSON **不支持注释**，所以"列序是 right/left/down/up"这种约定无处可写。只能靠文件名、文章、或加载时打日志。这是 JSON 相对 TOML/Lua 的真实劣势，也是 §3.7 讨论 Lua 的起点之一。

### 3.7 第三选项：Lua —— 能在数据里写函数，值不值得？

一个人了解 Lua 之后，很自然会想："Lua 表既能存数据又能写函数，还能热重载，为什么不直接用 Lua 当 manifest？甚至可以给每种排版写一个自己的 `GetRectByFrame(anim, frame)`。"

这个想法的**一半是对的**：Lua 确实是业界主流选项之一，而且是最适合"配置 + 行为"混合场景的那个。但它要解决的问题和"这张图怎么切"不是同一类问题。先看两个事实。

**事实一：Lua 在业界的两个岗位，职责不同。**

| 岗位 | 代表作品 | 用 Lua 存的是什么 |
| --- | --- | --- |
| **数据定义语言** | Factorio（原型定义）、WoW UI 配置 | 表结构：物品属性、配方、界面布局 |
| **行为脚本** | Balatro、Defold、Roblox、Don't Starve、Garry's Mod | 函数：AI、卡牌效果、事件回调 |

注意：**没有一个主流作品用 Lua 描述图集布局**。Aseprite 导 JSON、TexturePacker 导 JSON/plist、Spine 导 JSON、Tiled 导 TMX/JSON、Unity `.meta` 是 YAML。原因很实在：**导出工具是程序，它只会写字面量，不会生成函数**。资产管线里没有解释器。

**事实二：`GetRectByFrame` 这个诉求，其实是 schema 不够强的症状。**

推理一遍。假设允许每种排版自带一个取帧函数，那么：

```text
两张表排版相同  → 函数内容一样 → 复制 N 份 → 改一处忘一处
两张表排版不同  → 函数内容不同 → C++ 侧怎么知道该调哪个？
                                  配置里存函数名字符串 → 又是一层间接
                                  → 拼错就在运行期崩
```

第一行是"知识被复制"——正是本文要消灭的东西。
第二行更糟：你本来只想要"不同的起点数值"，结果得到了"不同的**代码路径**"，于是 C++ 侧必须再实现一个"函数分发表"，**清单里省掉的 schema，从后门又长回来了，而且更难校验**。

真正的诉求只有一句：**布局可以不一样**。而"布局不一样"是**数据差异**，不是**算法差异**。解法是让 schema 表达差异：

| 差异类型 | schema 怎么表达 | 需要函数吗 |
| --- | --- | --- |
| 每行起点不同 | `startY`（本文已做） | ❌ |
| 某片段不是从第 0 列开始 | `startX` | ❌ |
| 格间有 1px 间隙 | 加 `spacing` 字段，公式变 `startX + i * (w + spacing)` | ❌ |
| 帧尺寸不一致 / 打包器打乱顺序 | 改成逐帧 `rect` 列表（TexturePacker 形态） | ❌ |

**关键认知：schema 本身就是那个"共用函数"。**

```text
"每个表写一个 GetRectByFrame"    = 一份算法 复制 N 份  → 数据里有代码
"一份 schema + N 份数据"         = 一份算法 参数化 N 次 → 代码里有数据
```

后者就是"参数化"这个词的全部意思。函数只写一次，变化的部分通过参数（`startX/startY/frames`）进来。这也是为什么 `SrcRect()` 只有 4 行却什么表都能取。

**那在数据里写函数，代价到底是什么？**

| 代价 | 具体后果 |
| --- | --- |
| 失去编译期校验 | `static_assert` 彻底失效（函数不是常量表达式） |
| 失去加载期校验 | 你只能"调用一下看它崩不崩"，无法判断一个返回 rect 的函数是否越界 |
| 热重载变危险 | 改错一个函数 → 游戏正在跑 → 下一帧崩在渲染线程，且没有栈回溯指向清单行号 |
| 双语言调试 | 画面错了，要先判断是 C++ 算错还是 Lua 算错 |
| 性能模型变模糊 | 每实体每帧跨语言调用一次（C++ ↔ `lua_State`），60Hz × N 个实体 |
| 心智负担 | 两种内存模型（RAII vs GC）、两种错误模型（异常 vs `longjmp`）|

**所以结论分两层，不要混：**

```text
资产布局 = 数据  →  JSON / constexpr        ← 本文讨论的东西，现在做
游戏行为 = 代码  →  Lua / C++              ← 到 D 阶段（怪物 AI、物品效果、技能）再上
```

Lua 会在本项目出现，但它的登场时机是"**开始有行为需要让非程序员改**"的时候——比如你在 `manifest` 里想定义"这把剑命中后 30% 概率触发流血"，那才是 Lua 的主场，因为它需要**条件和函数**，而布局永远只需要**数值**。

> 判断标准一句话：**这段配置需要"如果……那么……"吗？** 需要 → 脚本；只需要"是什么"→ 数据。
> 图集布局永远只需要"是什么"。

**如果将来真的要嵌 Lua，本项目的路线是：**

```text
vcpkg 装 lua（~250 KB）+ sol2（header-only 绑定）
→ 沙箱：只暴露白名单 API（拿属性、加 buff、写日志）
→ 热重载：ReadDirectoryChangesW 监听 .lua 变化 → 重建 lua_State
→ 最难的部分不是语法，是"谁拥有谁"：Lua 的 GC 与 C++ 的 RAII 是两套所有权模型
```

> 408 映射：嵌 VM 是极好的系统级教材——`lua_State` 是一个带栈的虚拟机（组成原理的运行时栈）、GC 是标记清除（内存管理）、`lua_pcall` 的错误跨语言传播靠 `setjmp/longjmp`（OS 的上下文切换思想）、全局表是哈希表（数据结构）。**但它是 D 阶段的甜品，不是 A 阶段的主食**——现在吃会把"让精灵动起来"推迟一周以上。

### 3.8 实战复盘：真实素材是"转置"的

本文初稿假设所有精灵表都长这样：

```text
行 = 方向，列 = 帧
       帧1  帧2  帧3  帧4
  下 |    |    |    |    |     ← startX 往前走
  左 |    |    |    |    |
  右 |    |    |    |    |
  上 |    |    |    |    |
```

拿到真实素材（96×176）一量，是反的：

```text
列 = 方向，行 = 帧
      right  left  down   up
帧1 |      |      |      |      |   ← startY 往前走
帧2 |      |      |      |      |
帧3 |      |      |      |      |
帧4 |      |      |      |      |
```

**怎么确认的？** 缩略图根本看不清，靠的是三个可量化的比对：

| 比对 | 实测结果 | 结论 |
| --- | --- | --- |
| 相邻两列（同格 vs 水平镜像） | 差 12 / 1056 像素 | 两列互为镜像 = 左/右两个朝向 |
| 相邻两行（同列） | 差 278 / 1056 像素 | 行在变 = 行装的是"帧" |
| 全透明行/列索引 | 列空隙 0–3、21–26、44–49、70–73、94–95 | 切成 **4 列 × 24 像素** |

**全透明像素是最好的网格尺子**：美术天然会在格子之间留白，量出留白位置就等于量出了格子边界——比肉眼数格子靠谱得多。这条技巧以后处理任何新素材都通用。

**教训（比这个具体结论重要得多）：**

> 素材怎么排是**美术的产出**，不是你的规定。你能规定的只有"清单怎么写"，不能规定"图怎么排"。

这正是 `columns` 被删掉的原因——它编码的是"帧沿 x 轴排"这个**假设**。换成 `stepX/stepY` 之后，假设从 schema 里消失，变成数据里的两个数字。**schema 里每多一条假设，就多一个将来必然要改的地方。**

**还有一道自动闸门。** `ValidateFits()` 拿 `SDL_GetTextureSize` 的真实宽高，算出每条片段最后一个像素落在哪：

```text
walk_up:  x: 72 + 3×0  + 24 = 96   ≤ 96   ✅
          y:  0 + 3×44 + 44 = 176  = 176  ✅   正好贴边
```

假如还用旧 schema（`startX + i * frameWidth`）读这张图，`walk_up` 会算出 `x = 72 + 3×24 = 144 > 96`——**启动第一秒就报错**，而不是等你在游戏里看见"往上走时人在抖"再去猜是渲染错了还是数据错了。

> 把"画面缺一块"变成"启动失败"，就是加这 20 行函数全部的收益。**错误的发现时机永远值得花钱买。**



---

## 四、和已知知识的挂钩

| 已有知识 | 本文的延伸 |
| --- | --- |
| 16 号：`srcRect` 取一格 | 取哪一格由清单给出，不再硬编码 |
| 17 号：`frameIndex + accumulator` | 帧数与帧时长从 `ClipDesc` 读，代码里不再出现魔数 |
| 15 号：列表初始化禁窄化 | `static_cast<float>` 是编译器逼你写的 |
| 12 号：`std::vector` / `const&` | `std::vector<ClipDesc>` 拥有数据，`SpriteAnimator` 用 `const ClipDesc*` 只借阅它，不复制 |
| 03 号：ODR / 头文件 | `inline constexpr` 让头文件里的变量全局唯一 |
| 09 号：碰撞盒 | 视觉大小 ≠ 碰撞大小，所以 hitbox 不进 manifest |
| 05 号：vcpkg manifest | 同一个词：`vcpkg.json` 描述"项目需要哪些依赖"，本质也是资源清单 |

---

## 五、常见坑

| 坑 | 现象 | 原因 / 解决 |
| --- | --- | --- |
| 用 `row` 代替 `startY` | 加 spacing 后全部错位 | 存像素事实，不存推导 |
| 假设"帧一定沿 x 轴排" | 转置素材上只显示一格、或整段乱帧 | `stepX/stepY` 表达排布，`ValidateFits` 兜底（§3.8） |
| 运行时结构用 `string_view` 存 JSON 字符串 | 加载完立刻乱码或崩溃 | 结构体自己持 `std::string`，别借用已销毁的文档 |
| `frameMs = 0` | `while` 死循环、程序卡死 | `Validate()` 校验 `> 0`，`Update` 再早退一次 |
| 忘写 `inline`（用 `constexpr` 载体时） | 多个 .cpp 各持一份清单，改一处不生效 | `inline constexpr`（头文件变量必备） |
| 文件名里写死 `4x4` | 改列数后文件名撒谎 | 文件名只描述身份，不描述布局 |
| 坐标不是帧尺寸整数倍 | 画面被切掉一条缝 | 校验 `startX % frameWidth == 0` |
| `frameMs` 存 float 秒 | 0.1 变 0.100000001，diff 里看不出 | 存整数毫秒 |
| manifest 里放位置/速度 | 数据文件每秒都在变，无法版本管理 | 资源是**静态**的，运行时状态归代码 |
| 两份 clip 表（旧的 `AnimationClip` + 新的 `ClipDesc`） | 改一处另一处忘了 | 删掉旧结构，只留一份真相 |

**调试方法**：清单写完后先做一次"体检打印"——

```cpp
for (const ClipDesc& c : sheet.clips) {
    SDL_Log("%-12s start=(%d,%d) step=(%d,%d) frames=%d %dms loop=%d",
            c.name.c_str(), c.startX, c.startY, c.stepX, c.stepY,
            c.frames, c.frameMs, static_cast<int>(c.loop));
}
```

对不上就说明清单错了，跟渲染代码无关——**分层排查**，先证明数据对，再怀疑算法。

---

## 六、自测（4 题）

**Q1.** 初稿用 `columns` 表达"每行几格"，最终改成 `stepX/stepY`。请回答：(a) 对一张 4 列横排的表，两种写法各要写几个数字？(b) 对一张 4 行纵向排布的表（本文的真实素材）呢？(c) 如果将来有一张表要求"帧与帧之间空 2 像素"，`columns` 方案和 `stepX/stepY` 方案分别要怎么改？

**Q2.** 如果用 `row: 2` 而不是 `startY: 64`，有哪一种**真实存在**的图集，用 `row` 表达不出来，而 `startY` 能表达？（提示：16 号讲纹理渗色时提到过一种做法）

**Q3.** 有人提议："既然编译期校验这么好，那干脆所有资源都 constexpr 算了，永远不用 JSON。" 请指出这个方案在**加一个新怪物**时会遇到的第一个麻烦（提示：改一个数值要付出什么代价？）。

**Q4.** 假设允许每种排版在 Lua 里写自己的 `GetRectByFrame(anim, frame)`。请回答：C++ 侧怎么知道该调哪一张表的函数？（说出两种做法，并各自指出它新引入的失败模式。）然后说明：这个需求为什么其实不需要函数，只需要一个更强的 schema？

---

## 七、下一课预告

**本文已落地**：`Assets/player.json` + `SpriteSheet.hpp` + `SpriteSheetLoader.cpp` + `SpriteAnimator.hpp` 已接进 `EntityPlayer`，四个方向走路动画跑通，`ValidateFits` 验过（注入四种故障——起点不对齐、帧数越界、缺字段、语法错——每种都被对应那道闸门拦下）。

**19 号《图层与渲染排序》**：地牢里地面、血迹、道具、角色、墙顶要按什么顺序画？为什么"先画后画"决定遮挡关系，以及 `std::sort` 在这里为什么是正席（408 排序算法）。

> 本轮顺带兑现了 16（取哪一格）、17（什么时候换格）、18（布局从哪读）三课。
