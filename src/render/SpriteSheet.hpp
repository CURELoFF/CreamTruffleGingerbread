/**
 * @file    SpriteSheet.hpp
 * @brief   精灵表清单的运行时结构：由 JSON 载入，自己持有字符串副本。
 * @note    容器持有 std::string 而非 string_view：解析用的 JSON 文档用完即销毁，
 *          string_view 指向的字符会立刻悬空。
 */

#pragma once

#include <SDL3/SDL_rect.h>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

/**
 * @brief 一段动画的静态描述。
 * @note 单位：坐标与尺寸为像素，时长为毫秒。
 *       帧的排布由 stepX/stepY 表达：横向流用 (frameWidth, 0)，纵向流用 (0, frameHeight)。
 */
struct ClipDesc {
    std::string name;     ///< 片段名，用于查找与日志
    int  startX  = 0;     ///< 首帧左上角 x（源图像素）
    int  startY  = 0;     ///< 首帧左上角 y（源图像素）
    int  stepX   = 0;     ///< 每前进一帧 x 的增量（像素）
    int  stepY   = 0;     ///< 每前进一帧 y 的增量（像素）
    int  frames  = 0;     ///< 本片段共几帧（必须 > 0）
    int  frameMs = 0;     ///< 每帧停留毫秒（必须 > 0）
    bool loop    = true;  ///< true=循环播放，false=播完停住
};

/**
 * @brief 一张精灵表的完整清单。
 */
struct SheetDesc {
    std::string texture;         ///< 图片路径（相对工作目录）
    int frameWidth  = 0;         ///< 单帧宽（像素）
    int frameHeight = 0;         ///< 单帧高（像素）

    std::vector<ClipDesc> clips; ///< 全部动画片段

    /**
     * @brief 清单自洽性校验（只看清单，不看图片）。
     * @return 空串表示通过；否则是"第几条片段、哪个字段错了"的描述。
     */
    std::string Validate() const {
        if (texture.empty())  return "texture is empty";
        if (frameWidth <= 0)  return "frameWidth must be > 0";
        if (frameHeight <= 0) return "frameHeight must be > 0";
        if (clips.empty())    return "clips is empty";

        for (std::size_t i = 0; i < clips.size(); ++i) {
            const ClipDesc& c    = clips[i];
            const std::string at = "clips[" + std::to_string(i) + "] (\"" + c.name + "\")";
            if (c.name.empty())  return at + ": name is empty";
            if (c.frames <= 0)   return at + ": frames must be > 0";
            if (c.frameMs <= 0)  return at + ": frameMs must be > 0";
            if (c.startX < 0 || c.startY < 0) return at + ": startX/startY must be >= 0";
            if (c.stepX  < 0 || c.stepY  < 0) return at + ": stepX/stepY must be >= 0";
            if (c.stepX == 0 && c.stepY == 0 && c.frames > 1) {
                return at + ": stepX and stepY are both 0, all frames would overlap";
            }
            // 起点与步长都必须是整格的：否则帧会切在像素中间
            if (c.startX % frameWidth  != 0) return at + ": startX is not a multiple of frameWidth";
            if (c.startY % frameHeight != 0) return at + ": startY is not a multiple of frameHeight";
            if (c.stepX  != 0 && c.stepX % frameWidth  != 0) return at + ": stepX is not a multiple of frameWidth";
            if (c.stepY  != 0 && c.stepY % frameHeight != 0) return at + ": stepY is not a multiple of frameHeight";
        }
        return {};
    }

    /**
     * @brief 清单与实际贴图尺寸的匹配校验（需要图片的宽高）。
     * @param[in] textureW 贴图宽（像素）。
     * @param[in] textureH 贴图高（像素）。
     * @return 空串表示全部片段都在贴图范围内；否则指出越界的片段与实际算出的范围。
     * @note 这一步专抓"改了帧尺寸忘了改坐标""横向表当纵向读"这类错——它们语法全对、只有算出来才越界。
     */
    std::string ValidateFits(int textureW, int textureH) const {
        for (std::size_t i = 0; i < clips.size(); ++i) {
            const ClipDesc& c    = clips[i];
            const int endX = c.startX + (c.frames - 1) * c.stepX + frameWidth;
            const int endY = c.startY + (c.frames - 1) * c.stepY + frameHeight;
            if (endX > textureW || endY > textureH) {
                const std::string at = "clips[" + std::to_string(i) + "] (\"" + c.name + "\")";
                return at + ": frame range " + std::to_string(endX) + "x" + std::to_string(endY) +
                       " exceeds texture " + std::to_string(textureW) + "x" + std::to_string(textureH);
            }
        }
        return {};
    }

    /**
     * @brief 按名字查找片段。
     * @param[in] name 片段名（如 "walk_up"）。
     * @return 命中返回指针，否则 nullptr。
     */
    const ClipDesc* Find(std::string_view name) const {
        for (const ClipDesc& c : clips) {
            if (c.name == name) return &c;
        }
        return nullptr;
    }
};

/**
 * @brief 由清单 + 片段 + 帧索引算出源矩形。
 * @param[in] sheet      所属表清单。
 * @param[in] clip       当前片段。
 * @param[in] frameIndex 片段内帧号，须在 [0, clip.frames) 内（上游保证，此处不做修补）。
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
