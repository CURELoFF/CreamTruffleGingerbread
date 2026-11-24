/**
 * @file    SpriteSheetLoader.cpp
 * @brief   精灵表清单 JSON 载入实现。
 * @note    nlohmann/json 只在本翻译单元包含：改动画逻辑时不会重编这两万行。
 */

#include "SpriteSheetLoader.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

SheetDesc LoadSheetFromJson(std::string_view path) {
    const std::string file{path};

    // 1) 打开文件：失败基本是路径或工作目录问题，与 JSON 内容无关
    std::ifstream in{file};
    if (!in) throw std::runtime_error("cannot open sprite sheet manifest: " + file);

    // 2) 语法解析：parse_error 自带行号与列号
    nlohmann::json doc;
    try {
        in >> doc;
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error(file + ": JSON parse error: " + e.what());
    }

    // 3) 逐字段取值：缺字段/类型不符都会抛，补上文件与键名再上抛
    SheetDesc sheet;
    try {
        sheet.texture     = doc.at("texture").get<std::string>();
        sheet.frameWidth  = doc.at("frameWidth").get<int>();
        sheet.frameHeight = doc.at("frameHeight").get<int>();

        const nlohmann::json& clips = doc.at("clips");
        if (!clips.is_array()) throw std::runtime_error("\"clips\" must be an array");

        sheet.clips.reserve(clips.size());
        for (std::size_t i = 0; i < clips.size(); ++i) {
            const nlohmann::json& node = clips.at(i);
            ClipDesc clip;
            clip.name    = node.at("name").get<std::string>();
            clip.startX  = node.at("startX").get<int>();
            clip.startY  = node.at("startY").get<int>();
            clip.stepX   = node.at("stepX").get<int>();
            clip.stepY   = node.at("stepY").get<int>();
            clip.frames  = node.at("frames").get<int>();
            clip.frameMs = node.at("frameMs").get<int>();
            clip.loop    = node.value("loop", true);   // 可选字段：缺省视为循环
            sheet.clips.push_back(std::move(clip));
        }
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(file + ": " + e.what());
    }

    // 4) 语义校验：语法正确不等于内容正确（与贴图尺寸的匹配在拿到图片后另行校验）
    if (const std::string err = sheet.Validate(); !err.empty()) {
        throw std::runtime_error(file + ": " + err);
    }
    return sheet;
}
