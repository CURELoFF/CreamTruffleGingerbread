<#
.SYNOPSIS
    程序化生成一枚带透明背景的像素风史莱姆精灵（PNG）。

.DESCRIPTION
    思路：先在一张 16x16 的"逻辑像素画布"上逐像素上色（用字符地图描述形状），
    再用【最近邻放大】把每个逻辑像素放大成 Scale x Scale 的方块。
    全程 SetPixel 精确控制，不依赖任何外部图片资源。

    字符地图约定（注意：PowerShell 哈希表键不区分大小写，
    所以描边用 '#' 而不是 'B'，避免和身体 'b' 撞键）：
        .  透明（alpha = 0）
        #  描边色（深绿）
        b  身体色（亮绿）
        h  高光色（浅绿，脚本里单独点）
    眼睛/瞳孔在解析完地图后单独绘制（比塞进地图更好调整）。

.PARAMETER OutPath
    输出 PNG 路径（相对路径按当前工作目录解析）。

.PARAMETER Scale
    每个逻辑像素放大成多少物理像素（1 = 16x16 原尺寸，8 = 128x128）。

.EXAMPLE
    pwsh -NoProfile -ExecutionPolicy Bypass -File .\tools\gen_sprite.ps1
    pwsh -NoProfile -ExecutionPolicy Bypass -File .\tools\gen_sprite.ps1 -OutPath assets\slime.png -Scale 8
#>
param(
    [string]$OutPath = "assets/player.png",
    [int]$Scale = 8
)

Add-Type -AssemblyName System.Drawing

# ---------- 16x16 逻辑像素地图（每行必须 16 个字符） ----------
$map = @(
    "................",
    "................",
    ".....######.....",
    "....##bbbb##....",
    "...##bbbbbb##...",
    "..##bbbbbbbb##..",
    ".#bbbbbbbbbbbb#.",
    ".#bbbbbbbbbbbb#.",
    "#bbbbbbbbbbbbbb#",
    "#bbbbbbbbbbbbbb#",
    "#bbbbbbbbbbbbbb#",
    "#bbbbbbbbbbbbbb#",
    ".#bbbbbbbbbbbb#.",
    "..##bbbbbbbb##..",
    "...##bbbbbb##...",
    "....########...."
)

$grid = $map.Count
if ($grid -ne 16) { throw "地图必须是 16 行" }
foreach ($row in $map) {
    if ($row.Length -ne 16) { throw "地图每行必须是 16 列：'$row' 长度为 $($row.Length)" }
}

# ---------- 调色板 ----------
$colors = @{
    '#' = [System.Drawing.ColorTranslator]::FromHtml("#1B5E33")   # 描边
    'b' = [System.Drawing.ColorTranslator]::FromHtml("#58C86E")   # 身体
    'h' = [System.Drawing.ColorTranslator]::FromHtml("#A8F0B0")   # 高光
}
$eyeWhite = [System.Drawing.ColorTranslator]::FromHtml("#FFFFFF")
$pupil    = [System.Drawing.ColorTranslator]::FromHtml("#1B1B1B")

# ---------- 1) 画 16x16 逻辑画布（32bpp 带 alpha） ----------
$logical = New-Object System.Drawing.Bitmap($grid, $grid, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)

for ($y = 0; $y -lt $grid; $y++) {
    for ($x = 0; $x -lt $grid; $x++) {
        $ch = [string]$map[$y][$x]
        if ($ch -eq '.') {
            $logical.SetPixel($x, $y, [System.Drawing.Color]::Transparent)
        } else {
            $logical.SetPixel($x, $y, $colors[$ch])
        }
    }
}

# 高光：左上角斜三点
$logical.SetPixel(4, 5, $colors['h'])
$logical.SetPixel(5, 5, $colors['h'])
$logical.SetPixel(4, 6, $colors['h'])

# 眼睛：两块 2x2 眼白 + 各一个瞳孔
foreach ($ex in @(4, 10)) {
    $logical.SetPixel($ex,     9, $eyeWhite)
    $logical.SetPixel($ex + 1, 9, $eyeWhite)
    $logical.SetPixel($ex,    10, $eyeWhite)
    $logical.SetPixel($ex + 1,10, $eyeWhite)
    $logical.SetPixel($ex + 1,10, $pupil)   # 瞳孔压在右下角
}

# ---------- 2) 最近邻放大到输出尺寸 ----------
$size = $grid * $Scale
$output = New-Object System.Drawing.Bitmap($size, $size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$g = [System.Drawing.Graphics]::FromImage($output)
$g.Clear([System.Drawing.Color]::Transparent)
$g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
$g.PixelOffsetMode   = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
$g.SmoothingMode     = [System.Drawing.Drawing2D.SmoothingMode]::None
$g.DrawImage($logical, 0, 0, $size, $size)
$g.Dispose()

# ---------- 3) 保存 ----------
$dir = Split-Path -Parent $OutPath
if ($dir -and -not (Test-Path $dir)) {
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
}
$output.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)

$logical.Dispose()
$output.Dispose()

Write-Output "Done: $OutPath (${size}x${size}, PNG 32bpp with alpha)"
