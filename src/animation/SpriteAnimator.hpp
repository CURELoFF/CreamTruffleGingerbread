/**
 * @file    SpriteAnimator.hpp
 * @brief   精灵动画播放器：用累加器把逻辑步长换算成帧索引。
 * @note    帧索引是离散量，绝不能插值；位置的连续插值仍由 InterpProperty 负责。
 */

#pragma once

#include "../render/SpriteSheet.hpp"

/**
 * @brief 角色朝向。
 * @note 枚举值与图集位置无关：方向→片段的对应关系只存在于 ClipNameOf() 和 JSON 里。
 *       实测素材的方向列序为 right, left, down, up，与这里的声明顺序无关。
 */
enum class Direction : int {
    Down  = 0,  ///< 朝下（面向镜头）
    Left  = 1,  ///< 朝左
    Right = 2,  ///< 朝右
    Up    = 3,  ///< 朝上（背对镜头）
};

/**
 * @brief 由朝向取清单中的走路片段名。
 * @param[in] dir 朝向。
 * @return 片段名（如 "walk_left"）。
 * @note 用名字查找而不是用行号，是为了让"行序"只存在于 JSON 里。
 */
constexpr const char* ClipNameOf(Direction dir) {
    switch (dir) {   // 枚举 + switch：将来加方向时编译器能提醒漏分支
        case Direction::Down:  return "walk_down";
        case Direction::Left:  return "walk_left";
        case Direction::Right: return "walk_right";
        case Direction::Up:    return "walk_up";
    }
    return "walk_down";   // 兜底，正常不可达
}

/**
 * @brief 单个动画片段的播放状态机。
 * @note 只借用 ClipDesc 指针、不拥有数据：片段生命周期由持有 SheetDesc 的一方保证。
 */
class SpriteAnimator {
public:
    /**
     * @brief 切到指定片段并复位播放状态。
     * @param[in] clip 片段描述；传 nullptr 表示无动画可播。
     */
    void Play(const ClipDesc* clip) {
        clip_        = clip;
        frameIndex_  = 0;
        accumulator_ = 0.0f;
        playing_     = (clip != nullptr);
    }

    /**
     * @brief 推进动画。
     * @param[in] dt 逻辑步长（秒）。必须是固定步长，不能是渲染帧间隔。
     * @note 用 while 补帧、用 -= 扣减余数：卡顿后能一次追平，且长时间运行不漂移。
     */
    void Update(float dt) {
        if (!playing_ || clip_ == nullptr) return;

        const float frameTime = static_cast<float>(clip_->frameMs) / 1000.0f;
        if (frameTime <= 0.0f) return;   // 清单已校验，这里防除零

        accumulator_ += dt;
        while (accumulator_ >= frameTime) {
            accumulator_ -= frameTime;   // 扣减而不是归零：保留不足一帧的余数
            if (++frameIndex_ >= clip_->frames) {
                if (clip_->loop) {
                    frameIndex_ = 0;
                } else {
                    frameIndex_ = clip_->frames - 1;
                    playing_    = false;   // 一次性动画停在最后一帧
                }
            }
        }
    }

    /**
     * @brief 立即回到片段第 0 帧（站立姿势）。
     * @note 本表没有独立 Idle 片段，所以"站住"= 走路动画的第 0 帧。
     */
    void Reset() {
        frameIndex_  = 0;
        accumulator_ = 0.0f;
    }

    /**
     * @brief 是否已绑定片段。
     * @return 绑定成功返回 true；Draw 前必须检查，否则 Clip() 会解引用空指针。
     */
    bool HasClip() const { return clip_ != nullptr; }

    /**
     * @brief 当前片段。
     * @return 片段引用；调用前须确认 HasClip()。
     */
    const ClipDesc& Clip() const { return *clip_; }

    /**
     * @brief 片段内当前帧号。
     * @return 帧号，范围 [0, frames)。
     */
    int FrameIndex() const { return frameIndex_; }

private:
    const ClipDesc* clip_ = nullptr;  ///< 当前片段（借用，不拥有）
    int   frameIndex_  = 0;           ///< 片段内帧号，只在 Update 里越界回绕
    float accumulator_ = 0.0f;        ///< 帧内已累计时间（秒）
    bool  playing_     = false;       ///< false = 一次性动画已播完或未开始
};
