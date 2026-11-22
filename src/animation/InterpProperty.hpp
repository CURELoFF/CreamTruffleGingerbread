/**
 * @file    InterpProperty.hpp
 * @brief   固定步长下的双拍状态容器：逻辑读写 + 帧间插值视图。
 */

#pragma once

#include <SDL3/SDL_rect.h>   // SDL_FRect（下方重载用；IWYU：自己开门）
#include <cmath>             // std::lerp（C++20 标准库线性插值）

// ======== 按类型分派的插值重载（必须写在模板定义【之前】——两段式查找的教训） ========
// 规矩：模板里调用的自由函数，重载要么在模板定义前可见，
//       要么靠 ADL（参数依赖查找）——但 float 等基本类型没有 ADL 资格。

/**
 * @brief float 的线性插值。
 * @param[in] a     上一拍的值。
 * @param[in] b     当前拍的值。
 * @param[in] alpha 间隔比例 ∈ [0,1)。
 * @return a 到 b 之间的插值结果。
 */
inline float Interpolate(float a, float b, float alpha) {
    return std::lerp(a, b, alpha);
}

/**
 * @brief SDL_FRect 的插值：只插值位置，宽高取当前值。
 * @param[in] a     上一拍的矩形。
 * @param[in] b     当前拍的矩形。
 * @param[in] alpha 间隔比例 ∈ [0,1)。
 * @return 位置插值后的矩形。
 * @note 尺寸不是"运动量"，插值它会产生形变。
 */
inline SDL_FRect Interpolate(const SDL_FRect& a, const SDL_FRect& b, float alpha) {
    SDL_FRect shown = b;
    shown.x = std::lerp(a.x, b.x, alpha);
    shown.y = std::lerp(a.y, b.y, alpha);
    return shown;
}

/**
 * @brief 插值属性（Interpolation Property）。
 * @tparam T 可插值类型，如 float、SDL_FRect（需提供对应 Interpolate 重载）。
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
     * @brief 出生即合法：prev = curr = 初值（第一帧无可插值）。
     * @param[in] initial 初始状态。
     */
    explicit InterpProperty(const T& initial = T{})
        : prev_(initial), curr_(initial) {}

    /**
     * @brief 拍首契约：新的一拍开始，快照旗复位。
     * @note 由会写状态的对象的 Update() 第一行调用，见类注释使用契约。
     */
    void BeginStep() {snapShotDone_ = false;}

    /**
     * @brief 唯一合法写通道：本拍第一个写手触发快照（prev = curr）。
     * @return curr_ 的引用——改它就是改本拍状态。
     * @note 后续写手拿到的仍是同一引用，不会再次覆盖 prev_。
     */
    T& Write() {
        if(!snapShotDone_) {
            prev_ = curr_;
            snapShotDone_ = true;
        }
        return curr_;
    }
    /**
     * @brief 事件通道：瞬移/直接赋值——prev、curr 同步，渲染端无插值过渡。
     * @param[in] value 事件后的新状态。
     */
    void Snap(const T& value) {
        prev_ = value;
        curr_ = value;
        snapShotDone_ = true;   // 本拍后续 Write 不再覆盖这份"事件快照"
    }

    /** @brief 读通道：当前逻辑状态（碰撞判定用这个，不用插值视图）。 */
    const T& Current() const {return curr_;}
    /** @brief 读通道：上一拍快照（调试/特殊用途）。 */
    const T& Previous() const {return prev_;}

    /**
     * @brief 渲染视图：按 alpha 在两拍之间插值。
     * @param[in] alpha 距上一拍走过的间隔比例（acc / STEP），∈ [0,1)。
     * @return 插值后的显示状态。
     */
    T Rendered(float alpha) const {
        return Interpolate(prev_,curr_,alpha);
    }

private:
    T prev_{};                 ///< 上一拍的状态快照
    T curr_{};                 ///< 当前拍的状态（逻辑唯一权威）
    bool snapShotDone_ = true; ///< 本拍是否已快照；出生时 prev == curr，视为已完成
};
