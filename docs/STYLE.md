# 注释规范（CreamTruffleGingerbread）

> 本文件是项目内所有 `src/**` 代码的注释标准。新写类时照抄格式，不必每次再问。
> 原则：**注释回答"为什么/约束是什么"，代码回答"怎么做"**。不要用注释复述代码。

---

## 0. 总则

| 规则 | 说明 |
| --- | --- |
| 语言 | 中文叙述，术语保留英文（RAII、handle、所有权…） |
| 格式 | 公开接口用 Doxygen 块 `/** ... */`；实现细节用行内 `//` |
| 位置 | **声明处写文档**（`.hpp`），**实现处只写"为什么这样做"**（`.cpp`） |
| 禁止 | 空壳注释（`@brief Update`）、复述代码（`/// 把 x 设为 1`）、过期注释 |
| 修改代码时 | 同一次提交内同步更新受影响的注释 |

Doxygen 命令只用这 6 个：

```text
@file  @brief  @param[in] @param[out] @return  @note / @warning
```

---

## 1. 文件头

每个 `.hpp` / `.cpp` 第一行之前放文件块，**只写两条**：

```cpp
/**
 * @file    Renderer.hpp
 * @brief   SDL_Renderer 的 RAII 封装（帧缓冲绘制入口）。
 */
```

- `@file` 必须与真实文件名一致。
- `@brief` 一句话说清"这个文件对外提供什么"。
- **不写** `@author` / `@date` / `@version`：作者是 Git 的作者，日期是 Git 的提交时间，版本号会忘记改反而误导。

---

## 2. 类注释

一句话职责 + 所有权/生命周期约束。

```cpp
/**
 * @brief SDL_Texture 的 RAII 封装：像素已上传显存。
 *
 * 句柄独占所有权，析构时销毁纹理。
 *
 * @warning 必须先于创建它的 SDL_Renderer 析构，否则句柄悬空。
 * @note 禁用拷贝：纹理句柄不可复制，复制会导致重复释放。
 */
class Texture {
```

要点：

1. 第一行 `@brief` 必须能让没看过代码的人知道"这是干什么的"。
2. **所有权**（谁拥有、谁借用）写进类注释，不写进构造函数注释。
3. 生命周期陷阱用 `@warning`；使用前提用 `@note`。

---

## 3. 函数注释

### 3.1 公开接口（`.hpp` 中声明）

```cpp
/**
 * @brief 从磁盘图片创建纹理。
 * @param[in] renderer 目标渲染器（借用，不接管所有权）。
 * @param[in] path     图片路径，相对当前工作目录。
 * @note 失败时内部句柄为 nullptr，调用方需用 IsValid() 检查。
 */
explicit Texture(SDL_Renderer* renderer, std::string_view path);
```

参数方向统一用：

| 标签 | 含义 |
| --- | --- |
| `@param[in]` | 只读输入 |
| `@param[out]` | 由函数写入 |
| `@param[in,out]` | 读入并改写 |

### 3.2 访问器 / 一行实现（可压成单行）

```cpp
/** @brief 借用底层句柄；所有权仍归本类，调用方不得销毁。 */
SDL_Texture* Get() const {return texture_;}
```

### 3.3 实现文件（`.cpp`）

**不要重复 `.hpp` 里已经写过的 `@brief/@param`。** 只在需要解释"为什么"时加行内注释：

```cpp
// 固定步长循环：把真实流逝时间切成 1/60 秒的逻辑拍，
// 渲染端再用 acc/STEP 做插值，消除帧率抖动。
while (acc >= STEP) {
    ...
}
```

### 3.4 契约型成员（必须写清"调用顺序"）

像 `InterpProperty` 这种"必须在 Update 第一行调用"的接口，契约写在**类注释**里编号列出，方法注释只做指向：

```cpp
/**
 * @brief 拍首契约：新的一拍开始，快照旗复位。
 * @note 会写状态的对象的 Update() 第一行必须调用它，见类注释使用契约。
 */
void BeginStep();
```

---

## 4. 成员变量

行尾 `///<`，写**单位或语义**，不写类型：

```cpp
float moveSpeed_ = 1024.f;          ///< 移动速度（像素/秒）
SDL_Window* window_ = nullptr;      ///< 窗口句柄；nullptr = 创建失败
InterpProperty<SDL_FRect> rect_;    ///< 位置唯一权威：prev/curr 两拍状态
```

- 私有成员统一后缀 `_`，已有代码保持一致。
- 有"魔数"含义的常量必须注明来源。

---

## 5. 教学性说明怎么放

学习项目里有些注释是**给学生看的原理讲解**（如 `explicit` 与拷贝列表初始化）。规则：

```text
先写 Doxygen 接口文档（机器/他人看）
再写 // 行内讲义（自己看），用 // 而不是 /** */
```

两者不混：`/** */` 块内只放接口事实，讲义放在块外的 `//` 里。

---

## 6. 反例对照

| ❌ 不好的写法 | ✅ 改成 |
| --- | --- |
| `/** @brief Update */` | `/** @brief 推进一拍的逻辑：读键盘、按 dt 更新位置并被世界边界裁剪。 */` |
| `// 调用 SDL_DestroyWindow` | `// 先销毁窗口再退出 SDL：SDL_Quit 之后句柄不再有效` |
| `/// 宽度` | `///< 客户区宽度（像素，不含边框）` |
| `@param[in] x 参数 x` | `@param[in] worldW 世界宽度（像素），用于右边界裁剪` |
| 类里写 `@author me @version ALPHA` | 删掉，交给 Git |

---

## 7. 提交前自查清单

- [ ] 每个 `.hpp/.cpp` 有 `@file` + `@brief`，且文件名一致
- [ ] 每个类有 `@brief`，涉及所有权/生命周期时有 `@note`/`@warning`
- [ ] 每个公开函数有 `@brief`，非显然参数有 `@param`，返回非 void 有 `@return`
- [ ] 成员变量有 `///<` 且带单位/语义
- [ ] 没有空壳注释与复述代码的注释
- [ ] 注释与代码行为一致（改代码时同步改注释）
- [ ] 构建通过：`cmake --build --preset debug`
