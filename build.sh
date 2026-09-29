#!/bin/bash

# ==================== 配置区 ====================
TARGET_USER="orangepi"
TARGET_IP="10.42.0.48"
TARGET_DIR="/home/orangepi/Linux_learn"
CC="gcc"
CFLAGS="-Wall -g -O0"
# ===============================================

# 用法: ./build.sh <目标文件夹> [源文件.c | 可执行文件名] [运行参数...]
# 例如: ./build.sh 01_file_io read_temp
#       ./build.sh 01_file_io main.c
#       ./build.sh 01_file_io            （默认编译该目录下的 main.c）

# 1. 解析参数：第一个参数必须是要编译的本地目录
DIR_ARG="$1"
if [ -z "$DIR_ARG" ]; then
    echo "❌ 错误: 请指定要编译的目录！"
    echo "👉 用法: $0 <目标文件夹> [源文件.c | 可执行文件名] [运行参数...]"
    echo "   例如: $0 01_file_io read_temp"
    exit 1
fi

if [ ! -d "$DIR_ARG" ]; then
    echo "❌ 错误: 目录 '$DIR_ARG' 不存在！"
    exit 1
fi

# 第二个参数可选：不传则默认编译 main.c
FIRST_ARG="$2"
if [ -n "$FIRST_ARG" ]; then
    shift 2
else
    FIRST_ARG="main.c"
    shift 1
fi
# 收集脚本后续的所有参数，透传给目标板上的程序
APP_ARGS="$@"

# 2. 进入目标目录，并以目录名作为远程存放的子目录名
cd "$DIR_ARG" || exit 1
DIR_NAME="$(basename "$(pwd)")"
REMOTE_DIR="$TARGET_DIR/$DIR_NAME/"

# 3. 确定源文件和输出文件名
if [[ "$FIRST_ARG" == *.c ]]; then
    # 传入的是源文件: main.c -> 可执行文件 main
    SRC_FILE="$FIRST_ARG"
    BIN_NAME="$(basename "${FIRST_ARG%.c}")"
else
    # 传入的是可执行文件名: 优先找同名 .c，找不到就退回 main.c
    BIN_NAME="$(basename "$FIRST_ARG")"
    if [ -f "$BIN_NAME.c" ]; then
        SRC_FILE="$BIN_NAME.c"
    elif [ -f "main.c" ]; then
        SRC_FILE="main.c"
    else
        echo "❌ 错误: 本地未找到源文件 '$BIN_NAME.c' 或 'main.c'！"
        exit 1
    fi
fi

if [ ! -f "$SRC_FILE" ]; then
    echo "❌ 错误: 源文件 '$SRC_FILE' 不存在！"
    exit 1
fi

if ! command -v "$CC" >/dev/null 2>&1; then
    echo "❌ 错误: 未找到编译器 '$CC'，请先安装 gcc！"
    exit 1
fi

# 4. 本地编译
echo "🔨 正在编译 $DIR_NAME/$SRC_FILE → $BIN_NAME ..."
if ! $CC $CFLAGS "$SRC_FILE" -o "$BIN_NAME"; then
    echo "❌ 编译失败，请检查代码！"
    exit 1
fi
echo "✅ 编译成功！"

echo "📦 正在推送 [$BIN_NAME] 到香橙派 ($TARGET_IP:$REMOTE_DIR)..."

# 5. 确保香橙派上有目标目录，并只传输这一个二进制文件
ssh "$TARGET_USER@$TARGET_IP" "mkdir -p $REMOTE_DIR"
scp -q "$BIN_NAME" "$TARGET_USER@$TARGET_IP:$REMOTE_DIR$BIN_NAME"

if [ $? -ne 0 ]; then
    echo "❌ 传输失败，请检查网络。"
    exit 1
fi

echo "✅ 传输成功！"
echo "----------------------------------------"
echo "🚀 正在香橙派上运行 [$BIN_NAME $APP_ARGS]..."
echo "----------------------------------------"

# 6. 远程赋予执行权限并立刻运行，终端直接回显香橙派的输出
ssh -t "$TARGET_USER@$TARGET_IP" "chmod +x $REMOTE_DIR$BIN_NAME && $REMOTE_DIR$BIN_NAME $APP_ARGS"
