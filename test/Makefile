TARGET_USER = fly
TARGET_IP =  192.168.137.57
TARGET_DIR  = /home/$(TARGET_USER)/workspace
CC = aarch64-linux-gnu-gcc
CFLAGS = -Wall -O2

all: hello_arm

hello_arm: main.cpp
	$(CC) $(CFLAGS) -o $@ $<

deploy: hello_arm
	ssh $(TARGET_USER)@$(TARGET_IP) "mkdir -p $(TARGET_DIR)"
	scp hello_arm $(TARGET_USER)@$(TARGET_IP):$(TARGET_DIR)/
	ssh -t $(TARGET_USER)@$(TARGET_IP) "$(TARGET_DIR)/hello_arm"

clean:
	rm -f hello_arm

.PHONY: all deploy clean
