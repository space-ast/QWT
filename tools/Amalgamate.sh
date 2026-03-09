#!/bin/bash
# 设置控制台编码为UTF-8
chcp.com 65001 > /dev/null
# 设置输出编码
export LANG=zh_CN.UTF-8
export LC_ALL=zh_CN.UTF-8

DEST=../src
OPTS='-i "../src" -i "../src/3rdparty/gl2ps" -w "*.cpp;*.h;*.hpp;*.c" -s'
./Amalgamate.exe $OPTS ./amalgamate/QWTAmalgamTemplate.h $DEST/../src-amalgamate/QwtPlot.h
./Amalgamate.exe $OPTS ./amalgamate/QWTAmalgamTemplate.cpp $DEST/../src-amalgamate/QwtPlot.cpp

# 删除 QwtPlot.cpp 中的 #include "gl2ps.h" 行
if [ -f "$DEST/../src-amalgamate/QwtPlot.cpp" ]; then
    sed -i '/#include "gl2ps.h"/d' "$DEST/../src-amalgamate/QwtPlot.cpp"
    echo "已从 QwtPlot.cpp 中删除 #include \"gl2ps.h\""
    
    # 将换行符从 LF 转换为 CRLF
    convert_to_crlf() {
        local file="$1"
        if [ -f "$file" ]; then
            awk '{sub(/$/, "\r"); print}' "$file" > "${file}.tmp"
            mv "${file}.tmp" "$file"
            echo "已将 $file 的换行符转换为 CRLF"
        fi
    }
    
    convert_to_crlf "$DEST/../src-amalgamate/QwtPlot.cpp"
    convert_to_crlf "$DEST/../src-amalgamate/QwtPlot.h"
else
    echo "警告: QwtPlot.cpp 文件不存在"
fi

#  使用read命令达到类似bat中的pause命令效果
echo 按任意键继续
read -n 1
echo 继续运行