#pragma once

#include <SDL3/SDL_rect.h>   // SDL_FRect（下方重载用；IWYU：自己开门）
#include <cmath>             // std::lerp（C++20 标准库线性插值）

// ======== 按类型分派的插值重载（必须写在模板定义【之前】——两段式查找的教训） ========
// 规矩：模板里调用的自由函数，重载要么在模板定义前可见，
//       要么靠 ADL（参数依赖查找）——但 float 等基本类型没有 ADL 资格。

/**
 * @brief float 的插值：一行，标准库代劳
 */
inline float Interpolate(float a, float b, float alpha) {
    return std::lerp(a, b, alpha);
}

/**
 * @brief SDL_FRect 的插值：只有位置是"运动量"，宽高取当前值
 */
inline SDL_FRect Interpolate(const SDL_FRect& a, const SDL_FRect& b, float alpha) {
    SDL_FRect shown = b;
    shown.x = std::lerp(a.x, b.x, alpha);
    shown.y = std::lerp(a.y, b.y, alpha);
    return shown;
}

/**
 * @brief 插值属性(Interpolation Property)
 * @tparam T float,SDL_FRect ...
 *
 * 使用契约：
 *   1) 会用 Write() 写状态的对象，其 Update() 第一行必须调用 BeginStep()；
 *   2) 只读/只 Snap 的对象无需 BeginStep；
 *   3) 瞬移等"事件"走 Snap()（不插值），连续运动走 Write()（插值）。
 */
template <typename T>
class InterpProperty {
public:
    /**
     * @brief 出生即合法：prev = curr = 初值（第一帧无可插值）
     */
    explicit InterpProperty(const T& initial = T{})
        : prev_(initial), curr_(initial) {}

    /**
     * @brief 拍首契约：新的一拍开始，快照旗复位（由会写状态的对象的 Update 第一行调用）
     */
    void BeginStep() {snapShotDone_ = false;}

    /**
     * @brief 唯一合法写通道：本拍第一个写手触发快照（prev=curr），返回 curr 可写引用
     * @return curr_ 的引用——改它就是改本拍状态
     */
    T& Write() {
        if(!snapShotDone_) {
            prev_ = curr_;
            snapShotDone_ = true;
        }
        return curr_;
    }
    /**
     * @brief 事件通道：瞬移/直接赋值——prev、curr 同步，渲染端无插值过渡
     * @param[in] value 事件后的新状态
     */
    void Snap(const T& value) {
        prev_ = value;
        curr_ = value;
        snapShotDone_ = true;   // 本拍后续 Write 不再覆盖这份"事件快照"
    }

    /** @brief 读通道：当前逻辑状态（碰撞判定用这个，不用插值视图） */
    const T& Current() const {return curr_;}
    /** @brief 读通道：上一拍快照（调试/特殊用途） */
    const T& Previous() const {return prev_;}

    /**
     * @brief 渲染视图：按 alpha 在两拍之间插值（自动分派给 T 对应的 Interpolate 重载）
     * @param[in] alpha ∈ [0,1)：距上一拍走过的间隔比例（acc/STEP）
     */
    T Rendered(float alpha) const {
        return Interpolate(prev_,curr_,alpha);
    }

private:
    T prev_{};
    T curr_{};
    bool snapShotDone_ = true; //  出生时prev_=curr_ 算一次快照
};
