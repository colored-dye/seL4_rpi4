TARGET = build/dencrypt-test
SRCS = src/main.c

minitower_INC = ../comm-smaccm/usercode/libminitower/include

.PHONY: all run clean
all: $(TARGET)

$(TARGET): $(SRCS) $(wildcard ../comm-smaccm/usercode/libminitower/src/*.c)
	@mkdir -p build
	gcc -o $@ $^ -I$(minitower_INC)

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf build
