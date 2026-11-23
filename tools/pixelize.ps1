<#
.SYNOPSIS
    通用"像素化"后处理：任意图 → 目标分辨率 + 有限调色板。

.DESCRIPTION
    这是 AI 生图 / 参考截图 与项目素材之间的"出口闸门"：

        任意来源的大图（1024x1024 生图、参考截图、网上素材）
          → 缩到目标分辨率（整数块取样 或 块内平均）
          → 量化到 N 色（频次取色 + 最小色距去重）
          → alpha 二值化（清理半透明毛边，可关）
          → 输出 PNG + 调色板色卡 + 放大预览

    两种缩小模式：
      -Div N   整数倍还原。输入若是"N 倍最近邻放大"的素材，取块中心即可无损还原。
      -Size S  缩到最长边 S。用块内平均（area average），适合任意尺寸的生图/截图。

    注意：本脚本不预设 32x32。测试阶段请用 -Size 指定与参考图同量级的分辨率
    （如 96 / 128 / 160），确定风格后再压到最终的 32x32。

.PARAMETER Path
    输入图片。

.PARAMETER Div
    整数缩小倍数（与 -Size 二选一）。1 = 不缩小。

.PARAMETER Size
    目标最长边（像素）。与 -Div 二选一。

.PARAMETER Colors
    调色板上限（默认 16）。实际颜色数可能更少——图里本来就没那么多可分的颜色。

.PARAMETER MinSep
    调色板最小色距（RGB 欧氏距离）。新颜色与已选颜色太近就跳过，
    避免"24 色里 20 种都是同一片灰"。默认 12；设 0 = 纯按频次取色。

.PARAMETER AlphaThreshold
    alpha 二值化阈值（1~255）。低于它 → 全透明，否则全不透明。
    设 0 = 保留原始 alpha（做带柔边特效时用）。

.PARAMETER Preview
    额外输出最近邻放大预览（倍数，0 = 不输出）。

.PARAMETER OutDir
    输出目录。

.EXAMPLE
    # 还原 3 倍放大的参考帧（102x108 → 34x36，14 色）
    pwsh -File .\tools\pixelize.ps1 -Path .\Test\Generated\CasualWalk_frame00.png -Div 3 -Colors 14

.EXAMPLE
    # 把生图压到与参考图同量级（128px，24 色）
    pwsh -File .\tools\pixelize.ps1 -Path .\Test\Generated\ai_char.png -Size 128 -Colors 24

.EXAMPLE
    # 场景截图：压到 160px，保留柔边
    pwsh -File .\tools\pixelize.ps1 -Path .\Test\Reference\SceneRef1.png -Size 160 -Colors 32 -AlphaThreshold 0
#>
param(
    [Parameter(Mandatory = $true)][string]$Path,
    [int]$Div = 0,
    [int]$Size = 0,
    [int]$Colors = 16,
    [int]$MinSep = 12,
    [int]$AlphaThreshold = 128,
    [int]$Preview = 4,
    [string]$OutDir = "Test/Generated"
)

$ErrorActionPreference = 'Stop'   # 出错立即停，避免刷屏式重复报错

Add-Type -AssemblyName System.Drawing
try { [Console]::OutputEncoding = [System.Text.Encoding]::UTF8 } catch { }

if (-not (Test-Path $Path)) { throw "Input not found: $Path" }
if ($Div -le 0 -and $Size -le 0) { throw "Specify either -Div or -Size" }
if (-not (Test-Path $OutDir)) { New-Item -ItemType Directory -Path $OutDir -Force | Out-Null }

# ------------------------------------------------------------------
# 小工具：像素打包 / 拆包 / 色距
# 注意：PowerShell 的 -shl 不会做整数提升，[byte] -shl 16 会截回 0，
#       所以所有通道读取一律先 [int] 强转。
# ------------------------------------------------------------------
function Pack([int]$r, [int]$g, [int]$b) { (($r -shl 16) -bor ($g -shl 8)) -bor $b }
function GetR([int]$p) { ($p -shr 16) -band 0xFF }
function GetG([int]$p) { ($p -shr 8) -band 0xFF }
function GetB([int]$p) { $p -band 0xFF }

# ---------------- 1) 读源图（LockBits → BGRA 字节数组） ----------------
$src = [System.Drawing.Bitmap]::FromFile((Resolve-Path $Path))
$W = $src.Width
$H = $src.Height
$rect = New-Object System.Drawing.Rectangle(0, 0, $W, $H)
$lockData = $src.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$stride = $lockData.Stride
$srcBytes = New-Object byte[] ($stride * $H)
[System.Runtime.InteropServices.Marshal]::Copy($lockData.Scan0, $srcBytes, 0, $srcBytes.Length)
$src.UnlockBits($lockData)
$src.Dispose()

function SrcA([int]$x, [int]$y) { [int]$srcBytes[$y * $stride + $x * 4 + 3] }
function SrcR([int]$x, [int]$y) { [int]$srcBytes[$y * $stride + $x * 4 + 2] }
function SrcG([int]$x, [int]$y) { [int]$srcBytes[$y * $stride + $x * 4 + 1] }
function SrcB([int]$x, [int]$y) { [int]$srcBytes[$y * $stride + $x * 4] }

# ---------------- 2) 决定目标尺寸 ----------------
if ($Div -gt 0) {
    if (($W % $Div) -ne 0 -or ($H % $Div) -ne 0) {
        Write-Output "[warn] source ${W}x${H} is not divisible by $Div; floored"
    }
    $tw = [int][math]::Floor($W / $Div)
    $th = [int][math]::Floor($H / $Div)
    $modeText = "nearest-block (div $Div)"
} else {
    $scale = [double]$Size / [math]::Max($W, $H)
    $tw = [math]::Max(1, [int][math]::Round($W * $scale))
    $th = [math]::Max(1, [int][math]::Round($H * $scale))
    $modeText = "area-average (longest edge $Size)"
}
if ($tw -lt 1 -or $th -lt 1) { throw "Target size collapsed to ${tw}x${th}" }

# ---------------- 3) 采样到目标尺寸 ----------------
$rgbArr = New-Object int[] ($tw * $th)
$alphaArr = New-Object int[] ($tw * $th)

if ($Div -gt 0) {
    $half = [int][math]::Floor($Div / 2)
    for ($y = 0; $y -lt $th; $y++) {
        for ($x = 0; $x -lt $tw; $x++) {
            $sx = $x * $Div + $half
            $sy = $y * $Div + $half
            $i = $y * $tw + $x
            $rgbArr[$i] = Pack (SrcR $sx $sy) (SrcG $sx $sy) (SrcB $sx $sy)
            $alphaArr[$i] = SrcA $sx $sy
        }
    }
} else {
    $sxStep = [double]$W / $tw
    $syStep = [double]$H / $th
    for ($y = 0; $y -lt $th; $y++) {
        $y0 = [int][math]::Floor($y * $syStep)
        $y1 = [math]::Min($H - 1, [int][math]::Floor(($y + 1) * $syStep) - 1)
        if ($y1 -lt $y0) { $y1 = $y0 }
        for ($x = 0; $x -lt $tw; $x++) {
            $x0 = [int][math]::Floor($x * $sxStep)
            $x1 = [math]::Min($W - 1, [int][math]::Floor(($x + 1) * $sxStep) - 1)
            if ($x1 -lt $x0) { $x1 = $x0 }
            $sr = 0.0; $sg = 0.0; $sb = 0.0; $sa = 0.0; $n = 0
            for ($yy = $y0; $yy -le $y1; $yy++) {
                for ($xx = $x0; $xx -le $x1; $xx++) {
                    $sr += SrcR $xx $yy
                    $sg += SrcG $xx $yy
                    $sb += SrcB $xx $yy
                    $sa += SrcA $xx $yy
                    $n++
                }
            }
            if ($n -eq 0) { $n = 1 }
            $i = $y * $tw + $x
            $rgbArr[$i] = Pack ([int][math]::Round($sr / $n)) ([int][math]::Round($sg / $n)) ([int][math]::Round($sb / $n))
            $alphaArr[$i] = [int][math]::Round($sa / $n)
        }
    }
}

# ---------------- 4) alpha 处理 ----------------
if ($AlphaThreshold -gt 0) {
    for ($i = 0; $i -lt $alphaArr.Length; $i++) {
        $alphaArr[$i] = if ($alphaArr[$i] -ge $AlphaThreshold) { 255 } else { 0 }
    }
}

# ---------------- 5) 调色板：按频次取色 + 最小色距去重 ----------------
$hist = [System.Collections.Generic.Dictionary[int, int]]::new()
$opaqueCount = 0
for ($i = 0; $i -lt $rgbArr.Length; $i++) {
    if ($alphaArr[$i] -eq 0) { continue }
    $c = $rgbArr[$i]
    if ($hist.ContainsKey($c)) { $hist[$c] = $hist[$c] + 1 } else { $hist[$c] = 1 }
    $opaqueCount++
}
$uniqueColors = $hist.Count

$ranked = @($hist.GetEnumerator() | Sort-Object -Property Value -Descending)
$sepSq = $MinSep * $MinSep
$palette = [System.Collections.Generic.List[int]]::new()
foreach ($kv in $ranked) {
    if ($palette.Count -ge $Colors) { break }
    $c = [int]$kv.Key
    $ok = $true
    if ($sepSq -gt 0) {
        $cr = GetR $c; $cg = GetG $c; $cb = GetB $c
        foreach ($q in $palette) {
            $dr = $cr - (GetR $q); $dg = $cg - (GetG $q); $db = $cb - (GetB $q)
            if (($dr * $dr + $dg * $dg + $db * $db) -lt $sepSq) { $ok = $false; break }
        }
    }
    if ($ok) { $palette.Add($c) }
}
if ($palette.Count -eq 0) { $palette.Add(0) }

# 映射到最近色（带缓存，避免重复计算）
$palArr = $palette.ToArray()
$palR = New-Object int[] $palArr.Length
$palG = New-Object int[] $palArr.Length
$palB = New-Object int[] $palArr.Length
for ($p = 0; $p -lt $palArr.Length; $p++) {
    $palR[$p] = GetR $palArr[$p]
    $palG[$p] = GetG $palArr[$p]
    $palB[$p] = GetB $palArr[$p]
}
$usage = New-Object int[] $palArr.Length
# 缓存：源颜色 -> 调色板【下标】（不是颜色值，否则没法累计用量）
$cache = [System.Collections.Generic.Dictionary[int, int]]::new()
for ($i = 0; $i -lt $rgbArr.Length; $i++) {
    if ($alphaArr[$i] -eq 0) { continue }
    $c = $rgbArr[$i]
    if ($cache.ContainsKey($c)) {
        $pi = $cache[$c]
        $rgbArr[$i] = $palArr[$pi]
        $usage[$pi]++
        continue
    }
    $cr = GetR $c; $cg = GetG $c; $cb = GetB $c
    $best = 0; $bestD = [double]::MaxValue
    for ($p = 0; $p -lt $palArr.Length; $p++) {
        $dr = $cr - $palR[$p]; $dg = $cg - $palG[$p]; $db = $cb - $palB[$p]
        $d = $dr * $dr + $dg * $dg + $db * $db
        if ($d -lt $bestD) { $bestD = $d; $best = $p }
    }
    $cache[$c] = $best
    $rgbArr[$i] = $palArr[$best]
    $usage[$best]++
}

# ---------------- 6) 输出 PNG ----------------
$outBmp = New-Object System.Drawing.Bitmap($tw, $th, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
for ($y = 0; $y -lt $th; $y++) {
    for ($x = 0; $x -lt $tw; $x++) {
        $i = $y * $tw + $x
        if ($alphaArr[$i] -eq 0) {
            $outBmp.SetPixel($x, $y, [System.Drawing.Color]::Transparent)
        } else {
            $p = $rgbArr[$i]
            $outBmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb($alphaArr[$i], (GetR $p), (GetG $p), (GetB $p)))
        }
    }
}

$base = [System.IO.Path]::GetFileNameWithoutExtension($Path)
$outPath = Join-Path $OutDir ("{0}_px{1}x{2}.png" -f $base, $tw, $th)
$outBmp.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)

# 放大预览
$previewPath = ""
if ($Preview -gt 0) {
    $pb = New-Object System.Drawing.Bitmap(($tw * $Preview), ($th * $Preview), [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($pb)
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
    $g.Clear([System.Drawing.Color]::FromArgb(255, 30, 30, 38))
    $g.DrawImage($outBmp, 0, 0, ($tw * $Preview), ($th * $Preview))
    $g.Dispose()
    $previewPath = Join-Path $OutDir ("{0}_px{1}x{2}_x{3}.png" -f $base, $tw, $th, $Preview)
    $pb.Save($previewPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $pb.Dispose()
}
$outBmp.Dispose()

# 色卡：每个颜色一块 32x32，按使用量从左到右排列
$swatchH = 32
$swatch = New-Object System.Drawing.Bitmap(($palArr.Length * $swatchH), $swatchH, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$sw = [System.Drawing.Graphics]::FromImage($swatch)
$sw.Clear([System.Drawing.Color]::FromArgb(255, 30, 30, 38))
$order = 0..($palArr.Length - 1) | Sort-Object { -$usage[$_] }
for ($k = 0; $k -lt $order.Count; $k++) {
    $p = $palArr[$order[$k]]
    $brush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(255, (GetR $p), (GetG $p), (GetB $p)))
    $sw.FillRectangle($brush, ($k * $swatchH), 0, $swatchH, $swatchH)
    $brush.Dispose()
}
$sw.Dispose()
$swatchPath = Join-Path $OutDir ("{0}_px{1}x{2}_palette.png" -f $base, $tw, $th)
$swatch.Save($swatchPath, [System.Drawing.Imaging.ImageFormat]::Png)
$swatch.Dispose()

# ---------------- 7) 报告 ----------------
Write-Output "input      : $W x $H"
Write-Output "output     : $tw x $th  ($modeText)"
Write-Output "unique     : $uniqueColors raw colors sampled"
Write-Output "opaque     : $opaqueCount / $($rgbArr.Length) px"
Write-Output "palette    : $($palArr.Length) colors (MinSep=$MinSep)"
Write-Output "file       : $outPath"
if ($previewPath) { Write-Output "preview    : $previewPath" }
Write-Output "swatch     : $swatchPath"
Write-Output ""
Write-Output "PALETTE (hex, mapped px, share):"
foreach ($idx in $order) {
    $p = $palArr[$idx]
    $share = if ($opaqueCount -gt 0) { 100.0 * $usage[$idx] / $opaqueCount } else { 0 }
    Write-Output ("  #{0:X2}{1:X2}{2:X2}  {3,6} px  {4,5:N1}%" -f (GetR $p), (GetG $p), (GetB $p), $usage[$idx], $share)
}
