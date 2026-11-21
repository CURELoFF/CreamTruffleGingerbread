# 14 · Surface 与 Texture：像素的上传之路

> 背景：B 阶段（资源层）开张。`sdl3-image` 已在 vcpkg 里装好，玩家还是个纯色矩形。
> 这一课要回答一个问题：**一张 PNG 文件，是怎么变成屏幕上那块会动的图的？**
> 答案是一条三段流水线：文件 → CPU 内存（Surface）→ GPU 显存（Texture）→ 屏幕。

---

## 一、一句话核心

> **Surface 是 CPU 内存里的像素矩阵，Texture 是 GPU 显存里的贴图句柄。**
> 加载 = 解码成 Surface（一次性），渲染 = 把 Surface 上传成 Texture（一次性），
> 之后每帧只重复"采样 Texture 画到目标矩形"——**上传一次，采样万次**。

## 二、三段流水线全景

```text
  "player.png"（磁盘上的压缩像素）
        │  IMG_Load()            ← 解码：PNG 的 zlib 压缩流 → 原始 RGBA 字节
        ▼
   SDL_Surface（CPU 内存：w/h/pitch/format/pixels）
        │  SDL_CreateTextureFromSurface()   ← 上传：内存 → 显存（DMA/总线拷贝）
        ▼
   SDL_Texture（GPU 显存：不透明句柄，住在 Renderer 的上下文里）
        │  SDL_RenderTexture()   ← 每帧：按 srcrect 采样、按 dstrect 落位
        ▼
   屏幕（backbuffer → Present → 显示器）
```

**为什么必须分两段？** 因为两边的“世界规则”不同：

| | Surface（CPU 侧） | Texture（GPU 侧） |
| --- | --- | --- |
| 你能干什么 | 逐像素读写、画线、做图像处理 | 只能整张贴、指定采样区域 |
| 内存归谁 | 系统内存（可读可写） | 显存（只能 GPU 高效读） |
| 访问速度 | 随机访问慢、可通过缓存优化 | 采样快、但读写受限 |
| 生命周期 | 临时工（上传完即可扔） | 常驻（每帧都要用） |

用你的老朋友关系说：**Surface 是“原材料”，Texture 是“搬到车间的半成品”**。

## 三、Surface 解剖（408·组成原理：帧缓冲与 pitch）

`SDL_Surface` 的公开字段就是它的全部秘密：

```text
w, h      宽高（像素数）
pitch     一行有多少【字节】——注意不是 w*4！每行可能带填充字节用于对齐
format    像素格式（RGBA8888 / RGB24 / 索引色…）
pixels    指向像素数据的裸指针（unsigned char* 家族）
```

**pitch 是这一课最“系统”的知识点**：

```text
一行像素 3 字节/像素 × 宽 100 = 300 字节
但显卡/内存更喜欢 4 字节对齐 → pitch 可能被补齐成 304 或 512
于是取第 y 行第 x 列的地址 = pixels + y * pitch + x * 每像素字节数
                      ↑ 用 pitch，不能用 w*3
```

408 对照：这正是**帧缓冲（frame buffer）**与**扫描线（scanline）**的模型——显存按行组织、
按行扫描输出，行与行之间留对齐空隙能显著提速（对齐到缓存行/DRAM 突发长度的同款思想）。
`pitch` 的存在就是“硬件对齐要求”泄露到 API 的痕迹。

> 坑预警：任何“逐像素处理 Surface”的代码，**必须用 pitch 算行偏移**，
> 用 `w * 4` 会在某些宽度上悄悄错位。SDL 提供了 `SDL_ReadSurfacePixel` 等工具，
> 但理解 pitch 机制比背 API 重要。

## 四、Texture 解剖：它为什么“必须和 Renderer 绑定”

`SDL_Texture` 是个**不透明句柄**（不透明 = 你拿不到内部字段），原因很实在：它住在 GPU
的显存里，CPU 根本没法直接访问——跨设备的内存，只能通过图形 API 间接操作。

关键结论（记进“依赖注入”账本）：

```text
Texture 出生必须带 Renderer：SDL_CreateTextureFromSurface(renderer, surface)
Texture 只能被【创建它的那个 Renderer】画：SDL_RenderTexture(renderer, texture, ...)
Texture 必须【先于 Renderer】销毁：若 Renderer 先死，Texture 的句柄成孤儿
```

这是你项目里**第二次遇到“资源依赖环境”**：第一次是窗口尺寸注入（EntityPlayer 构造收
worldW/worldH），这次是 Texture 依赖 Renderer。以后还会遇到第三次、第四次——这条线
就是挂账中的《依赖注入与上下文》文章的主线。

## 五、API 地图（SDL3 + SDL_image 3.x 实测版）

```cpp
#include <SDL3_image/SDL_image.h>   // IMG_Load / IMG_LoadTexture

// —— 路线 A：完整三段（教学/需要像素处理时用）——
SDL_Surface* surface = IMG_Load("assets/player.png");   // 失败返回 nullptr
if (!surface) { /* SDL_GetError() */ }
SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
SDL_DestroySurface(surface);            // 上传完成，原材料立即释放

// —— 路线 B：一步到位（只渲染不处理像素时用）——
SDL_Texture* tex2 = IMG_LoadTexture(renderer, "assets/player.png");
// 内部帮你建了临时 Surface 并上传；你拿不到像素数据

// —— 渲染 ——
SDL_FRect dst{100.f, 50.f, 128.f, 128.f};
SDL_RenderTexture(renderer, tex, nullptr, &dst);   // srcrect=nullptr 表示整张
// srcrect 非空 = 只画图集里的一块 —— 精灵/动画帧的地基

// —— 销毁 ——
SDL_DestroyTexture(tex);   // 必须在 Renderer 活着时调用
```

**两个版本差异（别照抄旧教程）**：

1. **SDL_image 3.x 没有 `IMG_Init` 了**——格式在首次使用时按需初始化。网上大量
   SDL2 教程里的 `IMG_Init(IMG_INIT_PNG)` / `IMG_Quit()` 在 3.x 里已删除（实测本机
   vcpkg 头文件：`IMG_Init` 零匹配）；
2. **SDL3 把 `SDL_RenderCopy` 改名成 `SDL_RenderTexture`**，且矩形统一用 `SDL_FRect`。

**多余功能的启动开关**（前两个以后会用到）：

```cpp
SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);  // 像素画必选：放大不糊
SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);    // 透明 PNG 需要
```

## 六、RAII 落地（家族 №5 与 №6）

按项目纪律，两个句柄都必须 RAII 封装。设计要点（代码你自己写，骨架思路）：

```text
class Surface {                 // RAII №5
    SDL_Surface* surface_ = nullptr;
    构造：IMG_Load(path)
    析构：SDL_DestroySurface
    禁拷贝（两个对象不能同时拥有同一份像素内存）
    可移动？（过户语义，和 unique_ptr 同款）
    Get() 返回裸句柄给 SDL API 用（借阅，不转移所有权）
    IsValid() 供 Application 检查
};

class Texture {                 // RAII №6
    SDL_Texture* texture_ = nullptr;
    构造：收 (SDL_Renderer*, SDL_Surface*) 或 (SDL_Renderer*, 路径)
    析构：SDL_DestroyTexture
    禁拷贝；Get() 借阅
};
```

**三个设计陷阱，先想清楚再写**：

1. **Texture 的析构顺序**——它必须先于 Renderer 死。如果 Texture 是 Application 的成员、
   Renderer 也是成员，成员析构顺序 = **声明顺序的逆序**；把 `texture_` 声明在 `renderer_`
   之后，才能保证 texture 先死（04 课 Rule of 0 的规则再次上岗）；
2. **路径错误要“响亮地失败”**——`IMG_Load` 返回 nullptr 时要打 `SDL_GetError()`，
   否则你只会看到“图没出来”的哑谜；
3. **别每帧加载**——`IMG_Load` 是磁盘 IO + 解码，每帧调用 = 卡顿 + 内存泄漏
   （若忘了 Destroy）。加载一次，循环里只画。

## 七、在本项目落地：把玩家矩形换成精灵

改动面很小，但有个**路径坑**必须先知道：

```text
相对路径 "assets/player.png" 是相对【工作目录】解析的，不是相对 exe！
· 从项目根目录启动（.\build\debug\xxx.exe）→ 能读到 assets/
· 在 build/debug 里双击 exe → 工作目录变成 build/debug → 读不到
稳妥解：用 SDL_GetBasePath() 拿到 exe 所在目录再拼接；或 CMake 把 assets 拷到输出目录。
```

渲染端接线（EntityPlayer::Draw 的大概形状，具体你写）：

```text
Draw(renderer, alpha):
    shown = collisionRect_.Rendered(alpha)     // 你已有的插值视图
    SDL_RenderTexture(renderer, texture_.Get(), nullptr, &shown)
```

注意：**逻辑矩形=位置+尺寸（100×100），贴图的源尺寸可能是 32×32 或 256×256**——
两者靠 `dstrect` 的宽高解耦，这就是“世界空间尺寸”和“资源原始尺寸”的第一次分离
（C 阶段 UI 层还会再分一次屏幕空间，伏笔记账）。

## 八、常见坑清单

1. **用 `w * 4` 算行偏移**——忽略 pitch，某些宽度下逐像素处理全错位；
2. **忘记销毁 Surface**——上传后 Surface 就该死；不销毁 = 每加载一张图漏几十 KB 到几 MB；
3. **Texture 比 Renderer 活得久**——Renderer 先析构后，Texture 句柄悬空（UB）；
4. **每帧 IMG_Load**——卡顿 + 泄漏双杀；
5. **相对路径依赖工作目录**——换启动方式就找不到文件；
6. **像素画没设 NEAREST**——默认线性过滤会把 32×32 的小图糊成油画；
7. **透明 PNG 忘开 BLEND**——透明区变黑块（或背景色）；
8. **SDL_image 3.x 照抄 SDL2 的 IMG_Init**——编译直接报“找不到标识符”。

## 九、408 挂钩汇总

| 现象 | 408 对应 |
| --- | --- |
| Surface 的 `pitch`（行对齐填充） | 组成原理：显存/帧缓冲按行组织、对齐到突发长度提升带宽利用率 |
| 上传（内存→显存拷贝） | 组成原理：DMA / 总线传输；操作系统的“设备内存映射” |
| Texture 是不透明句柄 | OS：内核对象/句柄模型（用户态拿到的只是编号） |
| 加载一次缓存复用 | OS：缓存命中与 IO 成本；将来资源管理器 = 哈希表选型（数据结构） |
| 析构顺序（Texture 先于 Renderer） | 语言/系统：资源依赖图的拓扑序（同 OS 的死锁预防：资源申请顺序约束） |

## 十、概念词典

| 术语 | 一句话 |
| --- | --- |
| Surface | CPU 内存里的像素矩阵（w/h/pitch/format/pixels） |
| pitch | 每行字节数，可能大于 w×每像素字节数（对齐填充） |
| Texture | GPU 显存里的贴图，不透明句柄，与创建它的 Renderer 绑定 |
| 上传（upload） | Surface → Texture 的一次性拷贝 |
| 采样（sampling） | 渲染时从 Texture 取像素，按 dstrect 缩放落位 |
| srcrect / dstrect | 源区域 / 目标区域（图集切帧 = 改 srcrect） |
| IMG_LoadTexture | SDL_image 3.x 一步到位的便捷路径（跳过 Surface） |
| ScaleMode | 缩放过滤：NEAREST 像素风 / LINEAR 平滑 |
| SDL_GetBasePath | 拿 exe 所在目录，解决工作目录漂移 |

## 十一、记忆锚点 🎯

> 文件 → Surface（CPU 像素，pitch 记账）→ Texture（显存贴图，认 Renderer 当爹）→
> RenderTexture 每帧采样。**上传一次，采样万次**；Surface 用完即焚，Texture 常驻；
> 谁先死谁后死看声明顺序的逆序。像素画用 NEAREST，透明图开 BLEND，
> 路径别赌工作目录。

## 十二、自测问题（回收后开精灵图/动画帧）

1. Surface 的 `pitch` 为什么可能不等于 `w × 每像素字节数`？举一个会产生填充的场景，
   并写出用 pitch 计算第 y 行地址的公式。
2. Texture 为什么必须“先于 Renderer 销毁”？如果 Application 里 `renderer_` 声明在
   `texture_` 之前，构造/析构顺序分别是什么？谁先死？
3. `IMG_LoadTexture` 一步到位和 `IMG_Load + CreateTextureFromSurface` 两段式，
   分别在什么场景下更合适？各说出一个必须用两段式的理由。

## 相关文章

- 02《RAII 与资源所有权》——Surface/Texture 是家族 №5/№6
- 04《C++ 类初见》——成员声明顺序决定析构顺序（本课第六节陷阱①）
- 06《vcpkg》——`sdl3-image` 就是这篇文章理论的实物兑现
- 13《多态与虚函数》——Draw 接口所在的虚函数家族
- 下一篇：精灵图与动画帧（srcrect 的正式登场）
