TARGET_USER ?= orangepi
TARGET_IP   ?= 10.42.0.49

# 远程存放目录：自动跟随当前文件夹名（如果想平铺到 Linux_learn 根目录： make run TARGET_DIR=/home/orangepi/Linux_learn
TARGET_DIR  ?= /home/$(TARGET_USER)/Linux_learn/$(notdir $(CURDIR))

# ================= 编译选项配置 =================
# 生成的可执行文件名（可自由修改）
TARGET      := Sensor_temp

# 自动扫描当前目录下所有的 .c 源文件
SRCS        := $(wildcard *.c)
# 将 .c 文件映射为 .o 目标文件
OBJS        := $(SRCS:.c=.o)

# 编译器与选项：
# -Wall -Wextra: 开启详尽警告（嵌入式开发必备，消灭隐患）
# -O2: 二级性能优化
# -g: 保留调试符号（方便后续使用 gdb 调试cu
# -pthread: 启用 POSIX 多线程支持
CC          := gcc
CFLAGS      := -Wall -Wextra -O2 -g -pthread
LDFLAGS     := -pthread

# ================= 伪目标声明 =================
.PHONY: all run clean

# 默认目标：直接敲 make 时执行
all: $(TARGET)

# 链接规则：将所有的 .o 链接成最终二进制文件
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "✅ 本地构建成功: $@"

# 编译规则：把每个 .c 文件编译成 .o 文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ================= 部署与运行目标 =================
# 依赖 $(TARGET)：如果源码被修改，敲 make run 会自动先触发重新编译
# 传参示例: make run ARGS="123"
run: $(TARGET)
	@echo "📦 正在推送 $< 到香橙派 [$(TARGET_IP):$(TARGET_DIR)]..."
	@ssh $(TARGET_USER)@$(TARGET_IP) "mkdir -p $(TARGET_DIR)"
	@scp -q $(TARGET) $(TARGET_USER)@$(TARGET_IP):$(TARGET_DIR)/$(TARGET)
	@echo "🚀 正在香橙派上执行 $(TARGET)..."
	@echo "--------------------------------------------------"
	@ssh -t $(TARGET_USER)@$(TARGET_IP) "chmod +x $(TARGET_DIR)/$(TARGET) && $(TARGET_DIR)/$(TARGET) $(ARGS)"
	@echo "--------------------------------------------------"

# 清理构建产物
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "🧹 清理完成"