/**
 * @file    SpriteSheetLoader.hpp
 * @brief   精灵表清单的 JSON 载入入口（实现里隐藏了 nlohmann/json）。
 */

#pragma once

#include <string_view>

#include "SpriteSheet.hpp"

/**
 * @brief 从 JSON 文件载入并校验一份精灵表清单。
 * @param[in] path JSON 路径，相对当前工作目录。
 * @return 已通过语义校验的清单。
 * @throw std::runtime_error 文件打不开、JSON 语法错、字段缺失/类型不符、校验失败。
 *        错误信息带文件路径与字段名，可直接打印给用户看。
 */
SheetDesc LoadSheetFromJson(std::string_view path);
