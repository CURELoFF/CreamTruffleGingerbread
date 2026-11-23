<#
.SYNOPSIS
    程序化生成玩家"基础身体"与"面部"两张 4x4 精灵表（分层 / paper-doll）。

.DESCRIPTION
    布局统一：4 列 x 4 行，每格 32x32，整表 128x128。
        列 = 动画帧 0..3（行走循环：站立 / 迈步 / 站立 / 迈步）
        行 = 朝向       0=下(正面)  1=左  2=右  3=上(背面)

    两张表共享【同一网格 + 同一套动画偏移】，
    所以绘制时对同一个 dst 矩形先后画身体、画面部，就能严丝合缝地对齐。

    角色刻意保持"框架"状态：
      - 不画五官（留给面部表）
      - 不画衣服样式、不画发型
      - 只用两种色：皮肤色(头/脖) + 身体色(躯干四肢)

    描边采用"外扩算法"：先铺色块，再给所有紧贴非透明像素的透明像素染描边色，
    这样不管形状怎么摆，轮廓都是自动生成的 1px 线。

.PARAMETER OutDir
    输出目录（默认 Assets，相对当前工作目录）。

.EXAMPLE
    powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\gen_player_sheet.ps1
#>
param(
    [string]$OutDir = "Assets"
)

Add-Type -AssemblyName System.Drawing

# ============================ 常量 ============================
$FRAME = 32          # 每格边长（像素）
$COLS  = 4           # 列 = 帧数
$ROWS  = 4           # 行 = 朝向数
$W = $FRAME * $COLS
$H = $FRAME * $ROWS

# 朝向常量：与文件行序一致
$DIR_DOWN = 0
$DIR_LEFT = 1
$DIR_RIGHT = 2
$DIR_UP = 3

# ============================ 调色板（中性，方便日后换色）============================
$colOutline = [System.Drawing.ColorTranslator]::FromHtml("#2E2836")  # 描边
$colSkin    = [System.Drawing.ColorTranslator]::FromHtml("#E8C6A3")  # 头/脖
$colBody    = [System.Drawing.ColorTranslator]::FromHtml("#B9B3C6")  # 躯干四肢
$colFace    = [System.Drawing.ColorTranslator]::FromHtml("#2E2836")  # 五官

# ============================ 基础工具 ============================
function New-Cell {
    New-Object System.Drawing.Bitmap($FRAME, $FRAME, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
}

function Fill-Rect($bmp, [int]$x, [int]$y, [int]$w, [int]$h, $color) {
    for ($yy = $y; $yy -lt $y + $h; $yy++) {
        for ($xx = $x; $xx -lt $x + $w; $xx++) {
            if ($xx -ge 0 -and $yy -ge 0 -and $xx -lt $bmp.Width -and $yy -lt $bmp.Height) {
                $bmp.SetPixel($xx, $yy, $color)
            }
        }
    }
}

# 外扩描边：所有"透明但与实心像素相邻"的像素染成描边色
function Add-Outline($cell) {
    $marks = New-Object 'System.Collections.Generic.List[System.Drawing.Point]'
    $neighbors = @(@(1, 0), @(-1, 0), @(0, 1), @(0, -1))
    for ($y = 0; $y -lt $FRAME; $y++) {
        for ($x = 0; $x -lt $FRAME; $x++) {
            if ($cell.GetPixel($x, $y).A -ne 0) { continue }
            $hit = $false
            foreach ($d in $neighbors) {
                $nx = $x + $d[0]
                $ny = $y + $d[1]
                if ($nx -lt 0 -or $ny -lt 0 -or $nx -ge $FRAME -or $ny -ge $FRAME) { continue }
                if ($cell.GetPixel($nx, $ny).A -ne 0) { $hit = $true; break }
            }
            if ($hit) { $marks.Add((New-Object System.Drawing.Point($x, $y))) }
        }
    }
    foreach ($p in $marks) { $cell.SetPixel($p.X, $p.Y, $colOutline) }
}

# 水平镜像整格（右向 = 左向翻转，保证左右绝对对称）
function Mirror-Cell($cell) {
    $m = New-Cell
    for ($y = 0; $y -lt $FRAME; $y++) {
        for ($x = 0; $x -lt $FRAME; $x++) {
            $m.SetPixel($FRAME - 1 - $x, $y, $cell.GetPixel($x, $y))
        }
    }
    $m
}

function Copy-Cell-To($sheet, $cell, [int]$dstX, [int]$dstY) {
    for ($y = 0; $y -lt $FRAME; $y++) {
        for ($x = 0; $x -lt $FRAME; $x++) {
            $sheet.SetPixel($dstX + $x, $dstY + $y, $cell.GetPixel($x, $y))
        }
    }
}

# ============================ 身体绘制 ============================
# 头部：正面/背面用宽头（12），侧面用窄头（10）
function Draw-Head($cell, [int]$dy, [bool]$wide) {
    if ($wide) {
        Fill-Rect $cell 12 (4 + $dy)  8 1 $colSkin
        Fill-Rect $cell 11 (5 + $dy) 10 1 $colSkin
        Fill-Rect $cell 10 (6 + $dy) 12 8 $colSkin
        Fill-Rect $cell 11 (14 + $dy) 10 1 $colSkin
        Fill-Rect $cell 12 (15 + $dy)  8 1 $colSkin
    } else {
        Fill-Rect $cell 13 (4 + $dy)  6 1 $colSkin
        Fill-Rect $cell 12 (5 + $dy)  8 1 $colSkin
        Fill-Rect $cell 11 (6 + $dy) 10 8 $colSkin
        Fill-Rect $cell 12 (14 + $dy)  8 1 $colSkin
        Fill-Rect $cell 13 (15 + $dy)  6 1 $colSkin
    }
}

# 正面 / 背面：腿部交替改变长度（迈步 → 一条腿短、一条腿长）
function Draw-Body-Front($cell, [int]$dy, [int]$legL, [int]$legR, [int]$armL, [int]$armR) {
    Draw-Head $cell $dy $true
    Fill-Rect $cell 15 (16 + $dy) 2 2 $colSkin            # 脖子
    Fill-Rect $cell 12 (18 + $dy) 8 7 $colBody            # 躯干（与手臂之间留 1px 空，描边会把它变成分割线）
    Fill-Rect $cell  8 (19 + $armL) 3 6 $colBody          # 左臂
    Fill-Rect $cell 21 (19 + $armR) 3 6 $colBody          # 右臂
    # 腿部：脚底固定在 y=29，靠高度差表现前后
    $hL = 5 + $legL
    $hR = 5 + $legR
    Fill-Rect $cell 12 (30 - $hL) 3 $hL $colBody
    Fill-Rect $cell 17 (30 - $hR) 3 $hR $colBody
}

# 侧面：只露一条腿的伸展差 + 一条近侧腿，取消手臂（3px 宽的角色手臂会和躯干糊成一团）
function Draw-Body-Side($cell, [int]$dy, [int]$step) {
    Draw-Head $cell $dy $false
    Fill-Rect $cell 15 (16 + $dy) 2 2 $colSkin            # 脖子
    Fill-Rect $cell 12 (18 + $dy) 8 7 $colBody            # 躯干（更窄）
    # 后腿固定，前腿靠长度变化表现迈步；两腿之间留 2px 空
    Fill-Rect $cell 11 25 3 5 $colBody
    $h = 5 + $step
    Fill-Rect $cell 16 (30 - $h) 3 $h $colBody
}

# ============================ 面部绘制 ============================
# 只画五官，其余保持透明；dy 必须和身体完全一致，否则眼睛会"漂移"
function Draw-Face-Front($cell, [int]$dy) {
    Fill-Rect $cell 12 (9 + $dy) 2 2 $colFace   # 左眼
    Fill-Rect $cell 18 (9 + $dy) 2 2 $colFace   # 右眼
    Fill-Rect $cell 15 (13 + $dy) 2 1 $colFace  # 嘴（中性一条线）
}

function Draw-Face-Side($cell, [int]$dy) {
    Fill-Rect $cell 12 (9 + $dy) 2 2 $colFace   # 单眼（远侧被挡住）
    Fill-Rect $cell 12 (13 + $dy) 2 1 $colFace  # 嘴
}

# ============================ 生成 ============================
$bodySheet = New-Object System.Drawing.Bitmap($W, $H, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$faceSheet = New-Object System.Drawing.Bitmap($W, $H, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
for ($y = 0; $y -lt $H; $y++) {
    for ($x = 0; $x -lt $W; $x++) {
        $bodySheet.SetPixel($x, $y, [System.Drawing.Color]::Transparent)
        $faceSheet.SetPixel($x, $y, [System.Drawing.Color]::Transparent)
    }
}

# 每帧的两拍参数：0=站立 1=迈步(左) 2=站立 3=迈步(右)
$legSteps  = @(0, 1, 0, -1)      # 腿长差
$armSteps  = @(0, -1, 0, 1)      # 手臂摆动（与腿反相）
$bobs      = @(0, -1, 0, -1)     # 整体上下浮动 1px（迈步时抬起）
$swings    = @(0, 1, 0, -1)      # 侧面腿部伸展差（正 = 前腿更长）

for ($dir = 0; $dir -lt $ROWS; $dir++) {
    for ($f = 0; $f -lt $COLS; $f++) {
        $dx = $f * $FRAME
        $dyp = $dir * $FRAME

        $body = New-Cell
        $face = New-Cell

        switch ($dir) {
            $DIR_DOWN {
                $dy = $bobs[$f]
                Draw-Body-Front $body $dy $legSteps[$f] (-$legSteps[$f]) $armSteps[$f] (-$armSteps[$f])
                Draw-Face-Front $face $dy
            }
            $DIR_UP {
                # 背面：看不到五官，面部格整格透明
                $dy = $bobs[$f]
                Draw-Body-Front $body $dy $legSteps[$f] (-$legSteps[$f]) $armSteps[$f] (-$armSteps[$f])
            }
            $DIR_LEFT {
                $dy = $bobs[$f]
                Draw-Body-Side $body $dy $swings[$f]
                Draw-Face-Side $face $dy
            }
            $DIR_RIGHT {
                # 先生成"左向"，再镜像；面部整体镜像后与身体自然对齐
                $dy = $bobs[$f]
                Draw-Body-Side $body $dy $swings[$f]
                Draw-Face-Side $face $dy
                $mirroredBody = Mirror-Cell $body
                $mirroredFace = Mirror-Cell $face
                $body.Dispose()
                $face.Dispose()
                $body = $mirroredBody
                $face = $mirroredFace
            }
        }

        Add-Outline $body
        Copy-Cell-To $bodySheet $body $dx $dyp
        Copy-Cell-To $faceSheet $face $dx $dyp

        $body.Dispose()
        $face.Dispose()
    }
}

# 预览图：身体 + 面部叠加（检查对齐用）
$preview = New-Object System.Drawing.Bitmap($W, $H, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$g = [System.Drawing.Graphics]::FromImage($preview)
$g.Clear([System.Drawing.Color]::Transparent)
$g.DrawImage($bodySheet, 0, 0, $W, $H)
$g.DrawImage($faceSheet, 0, 0, $W, $H)
$g.Dispose()

# 放大版预览（最近邻 x4）：1:1 的 32px 太小，肉眼验对齐很吃力
$scale = 4
$previewBig = New-Object System.Drawing.Bitmap(($W * $scale), ($H * $scale), [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$g2 = [System.Drawing.Graphics]::FromImage($previewBig)
$g2.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
$g2.PixelOffsetMode   = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
$g2.Clear([System.Drawing.Color]::FromArgb(255, 40, 40, 52))   # 深色底，方便看描边
$g2.DrawImage($preview, 0, 0, $W * $scale, $H * $scale)
$g2.Dispose()

# ============================ 保存 ============================
if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
}

$bodyPath    = Join-Path $OutDir "player_body_4x4.png"
$facePath    = Join-Path $OutDir "player_face_4x4.png"
$previewPath = Join-Path $OutDir "player_preview_4x4.png"
$previewBigPath = Join-Path $OutDir "player_preview_4x4_x4.png"

$bodySheet.Save($bodyPath, [System.Drawing.Imaging.ImageFormat]::Png)
$faceSheet.Save($facePath, [System.Drawing.Imaging.ImageFormat]::Png)
$preview.Save($previewPath, [System.Drawing.Imaging.ImageFormat]::Png)
$previewBig.Save($previewBigPath, [System.Drawing.Imaging.ImageFormat]::Png)

$bodySheet.Dispose()
$faceSheet.Dispose()
$preview.Dispose()
$previewBig.Dispose()

Write-Output "Done: $bodyPath (${W}x${H})"
Write-Output "Done: $facePath (${W}x${H})"
Write-Output "Done: $previewPath (${W}x${H})"
Write-Output "Done: $previewBigPath ($($W * $scale)x$($H * $scale))"
