# Level 1 - Dirt
Back to Normal Mode               - Esc / ctl+[
Insert Mode Insert at front/back  - i/a 
New Line at up/down               - o/O
Save(Write)                       - :w
Quit                              - :q
Combine upper two S'n'Q           - :wq
Quit without save                 - :q!
Move cursor:
    k          up
  h + l = left + right
    j         down

Delete a character                - x
Delete a line                     - dd
Copy a line                       - yy
Paste at back/front               - p/P
Undo                              - u
Redo                              - ctl+r
Replace a character               - r
character/line view Mode          - v/V
  Delete - x
  Copy   - y
  Modify - c

Search(Grope)                     - /foo
  Next/Previous one - n/N

# Level 2 - Combine Syntax
\[计数\] + 操作符 + \[计数\] + 动作 / 文本对象
或
动词 + 名词
d   + w   - 删除下一个词
c   + iw  - 修改当前值
y   + $   - 复制到行尾

操作符
d - delete(Normal Mode)
c - change(Insert Mode)
y - yank(duplicate - Normal Mode)

动作
w - begin of next char
b - begin of prev char
e - current/next char's tail
0 - line begin
^ - first non-empty char in line
$ - line end
gg- file start
G - file end
f{char} - Jump to next char of this line
t{char} - Jump to this char's front
;       - Repeat last f/t
,       - Reverse Repeat
%       - Jump to matched bracket


Ex:
  dw    - delete (1) word
  d2w   - delete (2) word
  dd    - delete (1) line
  2dd   - delete (2) line
  d$    - delete from cursor to line end
  d0    - delete from cursor to line begin
  df,   - delete from cursor to next',' in this line(include ',')
  dt,   - delete from cursor to prev',' in this line(exclude ',')
  d%    - delete to matched bracket
  dG    - delete to file end
  ciw   - change char, enter Insert Mode
  ci"   - change content inside ""
  y$    - yank from cursor to line end
  yy    - yank whole line
