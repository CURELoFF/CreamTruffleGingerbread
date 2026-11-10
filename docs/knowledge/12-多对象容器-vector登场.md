# 12 · 多对象容器：vector 登场（408·数据结构正席）

> 背景与上下文：`player`、`entity` 直到今天都是"一人一个变量"——敌人要是有
> 99 个，难道手写 99 套？本课 408·数据结构正式落座王座：**动态数组 vector**
> 收编全部敌人。同时兑现 11 号课的承诺：把"插值税"封装成共用小结构
> （Kinematic），一劳永逸。

---

## 一、一句话核心

> **vector = 会自己搬家的连续数组：下标寻址 O(1) 与 08 课的数组同底，
> 增长靠"扩容搬家"，均摊 O(1)。** 对象一多，管理权交给容器——
> 代码从"第 1 个敌人、第 2 个敌人…"升维成"每一个敌人，不管你是第几个"。

## 二、从"给第 99 个敌人手写变量"说起

现在 Run() 里 `entity` 是孤零零一个 `SDL_FRect`。想要敌人军团：手写
`entity1 entity2 … entity99`？写不完，而且每只都免不了复制一遍同样的
移动/判定/渲染代码——**代码在"按个数展开"，这正是需要数据结构的信号**。

需要的东西叫**顺序存储的线性表**（408·数据结构正宗术语）：

```text
一片连续内存，里面并肩躺着 N 个 SDL_FRect（每个 16 字节，元素等长）：
┌────────┬────────┬────────┬────────┐
│ e0(16B) │ e1(16B) │ e2(16B) │ …      │
└────────┴────────┴────────┴────────┘
外加一位管理员，记住两本账：size（实际住的个数）/ capacity（共有多少床位）
```

08 课的键盘快照是**静态数组**（SDL 定长，我们只能读）；vector 是它的
**动态成长版**（我们能增删、能遍历、能随需扩容）。C++ 标准库给的名字：
`std::vector<T>`——`T` 是住客类型，对我们就是 `SDL_FRect` 或 `Kinematic`。

## 三、vector 三件套心智模型

**① 下标 O(1)**——寻址公式与 08 课完全同款：

```text
&e[i] 的地址 = 基址 + i × sizeof(SDL_FRect)   // 16 字节，等长是 O(1) 的前提
```

**② size 与 capacity**——人口与床位的分离：

```text
size()     = 现在住着几个        （恒 O(1)，问一句就答）
capacity() = 在不搬家前提下装得下几个
push_back(x) = 末尾住进一个人：
   size < capacity → 直接住，均摊 O(1)
   size == capacity → 触发【搬家】：
        申请新地（通常 ×2）→ 旧人整队迁居 → 释放旧地
```

**③ 均摊（amortized）O(1)——408 的经典分析点**：搬家次数被翻倍策略
摊薄——从 0 长到 n 个人，累计搬家成本 O(n)，分摊到 n 次 push_back，
**每次只背 O(1) 的行囊**。这就是"为什么 push_back 敢自称均摊 O(1)"
的完整来路（严格说：最坏单次 O(n)，摊薄后均摊 O(1)——两个答案都对，
问法不同）。

## 四、埋雷区：两颗著名的雷（先排雷后上车）

**雷 ①：迭代器/引用失效**。搬家 = 全员换地址——搬家那一刻，
**所有旧地址全部作废**。如果手里还攥着搬家前拿的引用/迭代器继续用，
就是**访问已释放内存**（未定义行为，最阴险的一类 bug）：

```cpp
SDL_FRect* p = &entities[0];
entities.push_back(xxx);   // 若触发搬家，p 指向已被释放的旧地
p->x = 1.0f;               // 💥 UB：写入没主的内存
```

规矩一句话：**扩容性操作（push_back / resize / erase）与"持有引用、
迭代器"绝不同帧混用**。要么先扩后取，要么取了不扩。

**雷 ②：值语义深拷贝**。vector 整体赋值/按值传参 = **整支车队整个复制**
（n × 16 字节的 memcpy）。所以规矩：

```cpp
void Foo(std::vector<SDL_FRect> list);          // ❌ 按值=整队拷贝
void Foo(const std::vector<SDL_FRect>& list);   // ✅ const& 借阅——408 指针语义正用
```

`const&` 借阅的纪律到你写类时同一招吃遍天：**大对象一律借阅不拖库**。

## 五、插值税的一次性封印：Kinematic 小结构

11 号课补录③的承诺兑现：把（prev + 插值公式）封装成所有会动的角色共用的
最小结构——**新文件 `src/core/Kinematic.hpp`（header-only，ODR 特例上岗）**：

```cpp
// Kinematic.hpp —— 会动的东西的最小封装：一份状态 + 一份快照 + 一条插值公式
#pragma once
#include <SDL3/SDL_rect.h>   // SDL_FRect 的家（IWYU：自己开门）

/**
 * @brief 运动学状态对：双份位置（prev/curr）自动封好插值
 */
struct Kinematic {
    SDL_FRect prev{};   // 上一拍快照（插值原料）
    SDL_FRect curr{};   // 当前拍状态（update 的动手对象）

    /** @brief 开拍快照：update 动手前调用一次（11 号课的 prev = player） */
    void CachePrev() { prev = curr; }

    /**
     * @brief 渲染用：两拍之间按 α 脑补的展示位置
     * @param[in] alpha ∈ [0,1)：距上一拍走过的间隔比例（acc/STEP）
     * @return 插值后的展示矩形（局部量，画完即弃——display 无需持久成员）
     */
    SDL_FRect Rendered(float alpha) const {
        SDL_FRect shown = curr;
        shown.x = prev.x + (curr.x - prev.x) * alpha;
        shown.y = prev.y + (curr.y - prev.y) * alpha;
        return shown;
    }
};
```

注意两点：**display 不进结构**（它是渲染瞬间的局部量，进结构等于白背一份重复状态——
11 号课补录②的知识点在结构设计里落地）；敌人这种"瞬移事件型"角色可以
**不插值直接画 curr**——插不插值变成每个角色自己的选择权。

## 六、实战蓝图（新 1 文件 + 改造 Application.cpp）

**① 新建 `src/core/Kinematic.hpp`**（第五节蓝图，CMake **不用动**——header-only）。

**② Application.cpp 的改动（拼图式）**：

```cpp
// include 区新增：
#include <vector>          // std::vector 的家
#include "Kinematic.hpp"   // 同目录相对包含（与 "Application.hpp" 同写法）

// Run() 里的状态区：三变量 → 一只主角 + 一支敌军小队
Kinematic player{{50.f, 50.f, 100.f, 100.f}, {50.f, 50.f, 100.f, 100.f}};
std::vector<Kinematic> entities(3);          // 3 只敌人（住进连续内存）
for (auto& e : entities) {                   // 出生位置全随机（保证不出窗）
    e.curr.w = 50.f; e.curr.h = 50.f;
    e.curr.x = static_cast<float>(std::rand() % (1280 - 50));   // 1280/720 按你的窗口改；严格版可先取一次窗尺寸再用
    e.curr.y = static_cast<float>(std::rand() % (720 - 50));
}
```

```cpp
// ============ 账本 A：逻辑步进（内层 while 内部） ============
player.CachePrev();                                        // 拍前快照（Kinematic 一键）
/* …keys / ESC / 四方向移动，全部作用于 player.curr… */
player.curr.x = std::clamp(player.curr.x, ...);            // clamp 也改对 curr

// 军团判定：每个敌人，不管你是第几个
for (auto& e : entities) {
    if (Intersects(player.curr, e.curr)) {
        e.curr.x = static_cast<float>(std::rand() % (winw - 50));
        e.curr.y = static_cast<float>(std::rand() % (winh - 50));
    }
}

acc -= STEP;

// ============ 账本 B：渲染（外层 while 尾部） ============
const float alpha = static_cast<float>(acc / STEP);
const SDL_FRect display = player.Rendered(alpha);           // 显示员替你插值

/* 清屏后： */
SDL_RenderFillRect(renderer_.Get(), &display);              // 玩家：插值（运动）
for (const auto& e : entities) {
    SDL_RenderFillRect(renderer_.Get(), &e.curr);           // 敌人：curr 直画（瞬移=事件）
}
```

**③ 自由加菜（可选，动态数组看增长）**：按 R 生成一只新敌人——
**这次必须走事件路线**（"单击触发"是事件的专长，正好回扣 08 课两路线分工）：

```cpp
// 事件循环里追加：
if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_R) {
    Kinematic fresh;
    fresh.curr.w = 50.f; fresh.curr.h = 50.f;
    fresh.curr.x = static_cast<float>(std::rand() % (1280 - 50));
    fresh.curr.y = static_cast<float>(std::rand() % (720 - 50));
    entities.push_back(fresh);       // 数组当场长大（第十节自测第 2 题照它出题）
}
```

术语提前排雷：`SDLK_R` 是**字符键编码**（keycode），08 课的
`SDL_SCANCODE_*` 是**物理位置编码**（scancode）——SDL3 把字母 keycode
改成了大写形式（`SDLK_A`…），老的 SDL2 教程写 `SDLK_r` 会编不过，别怀疑自己。

验收标准：

```text
3 只（或更多）敌人同屏，方向键追击；碰到谁谁瞬移重生（无滑行）
玩家移动依旧插值平滑；可选加菜按 R，军团实时壮大
关窗安全退出
```

## 七、常见坑

1. **在 range-for 里 push_back 这支军团自己**——若触发搬家，正在遍历的引用
   全部悬空（雷 ① 亲爹）。按 R 加兵**放事件循环**（遍历还没开始）才是安全姿势；
2. **忘写 `CachePrev()`**——prev 停在好几拍前的旧位置，与 curr 差距巨大，
   插值画出"跨帧拖影"疯子速滑。症状对称记忆：快照忘拍 = 位置鬼畜；
3. **update 改动了 `prev`**——插值公式的原料只有 curr 一个变量了，
   画面抖动（prev 不许进 update 的手；update 只对 curr 有动武权）；
4. **下标越界**——非要 `entities[i]` 时先问 `size()`；本轮全用 range-for
   就自然免疫（容器"自己知道边界"正是它的卖点之一）。

## 八、概念词典

| 术语 | 一句话解释 |
| --- | --- |
| std::vector | 顺序存储的动态数组：连续内存 + 自管 size/capacity |
| 顺序表（408） | 逻辑相邻的元素物理也相邻的线性表，支持 O(1) 随机访问 |
| size / capacity | 实际人口 / 搬家前总床位 |
| 均摊 O(1)（amortized） | 偶发 O(n) 搬家摊薄到每次 push_back 只 O(1) |
| 迭代器/引用失效 | 扩容搬家后旧地址全部作废，再使用 = UB |
| const& 借阅 | 传参不拷贝车队；大对象传参的默认礼貌 |
| range-for | `for (auto& e : v)` 逐只访客——容器自带边界不越界 |
| Kinematic | prev/curr 双份状态 + 快照/插值两动作的最小封装 |
| keycode / scancode | 字符编码 / 物理位置编码（SDL3 字母 keycode 大写） |

## 九、记忆锚点 🎯

> vector 是一支会迁居的连续部队：下标报号 O(1)；size 是人口 capacity 是床位，
> 翻倍搬家均摊 O(1)；**搬家那天，旧引用全体阵亡**。
> 传参 const& 借阅不拖卡车；插值税封进 Kinematic，
> 每个会动的角色自带 prev——update 只准碰 curr，prev 是插值的原料。

## 十、自测问题（回收后开对象课）

1. capacity 8 的 vector 塞进第 9 个元素时会发生什么？把整套动作列出；
   并解释：为什么 push_back 敢自称"均摊 O(1)"而搬家机械本身是 O(n)？
2. `for (auto& e : entities) { if (…) entities.push_back(fresh); }`
   会遇上什么灾难？用"搬家→旧地址作废"机制层讲清楚，再给一条编码规矩。
3. 渲染敌军的循环写 `for (const auto& e : entities)`，渲染循环理论上
   不改状态——`const` 在这里除了"更礼貌"，还有哪一层工程价值？
   （提示：哪天你或未来的你不小心在 render 里写了改动，编译器会说什么？）

## 相关文章

- 08《输入》——键盘快照数组：O(1) 寻址公式的出处，vector 的同族前情
- 09《碰撞》——从单挑 Intersects 到军团循环遍历
- 11《游戏循环成人式》——插值债主；Kinematic 是它补录③承诺的兑现
- 下一篇《从矩形到游戏对象》——行为的归宿：Kinematic 家族继续演化，
  408·面向对象思想第一次在框架里正式安家
