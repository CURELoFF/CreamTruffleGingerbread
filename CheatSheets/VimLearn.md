Esc - 退回普通模式
u   - 撤回
U   - 撤回整行
ctrl + r - 重做
a/i - 从当前指针后/前编辑模式
  A   - 指针跳转到行尾再进入编辑模式
  o/O - 在下/上一行插入行再进入编辑模式
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

指针移动
    ^
    J
< H   L >
    k
    v

文件操作
:q  - 退出
:q! - 强制退出
:w  - 写入

=组合=
:wq - 写入并退出

字符删除
x   - 删除一个字符
d 删除命令使用方式
  syntax : d \[Number] \[motion]
  motion : w e $
  dd  - 剪切 当前行
  2dd - 剪切 2行
  dw  - 删除 从当前指针到下一个词尾(包括空格)
  d$  - 删除 从当前指针到行尾
  de  - 删除 从当前指针到当前词尾
  d2e - 删除两个词

字符替换
r\[char] - 替换一个字符
指针指向 "e" 输入 ra
Heppy -> Happy

与delete不同, change 会在删除后自动切换为插入模式
Syntax : c \[Motion]
Motion : 与delete相同
ce - 替换一个单词(当前指针到词尾)

word word word word word word word word word word word word word word word word word word

指针移动 Ver.2
Syntax : \[Number] motion
Motion : e w 
  2e - 移动两个词到词尾
  2w - 移动两个词到词首
0   - 移动到行首
$   - 移动到行尾
