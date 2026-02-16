BUILD_DIR := /tmp/build-xdp
BPF_TOOL := bpftool
BPF_CC := clang -target bpf
BPF_CFLAGS := -Wall

all: $(BUILD_DIR)/loader
	pwn $<

$(BUILD_DIR)/loader: $(BUILD_DIR)/loader.o
	$(CC) -o $@ $^ -lbpf

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -c -o $@ $<

%.skel.c: %.bpf.o
	$(BPF_TOOL) gen skeleton $< > $@.tmp
	mv $@.tmp $@

$(BUILD_DIR)/%.bpf.o: %.bpf.c | $(BUILD_DIR)
	$(BPF_CC) -c -g -O2 -o $@ $(BPF_CFLAGS) $<

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/loader.o: $(BUILD_DIR)/sample.skel.c bpf_types.h
$(BUILD_DIR)/sample.bpf.o: sample.bpf.c bpf_types.h

$(BUILD_DIR)/pwn: pwn.c | $(BUILD_DIR)
	$(CC) -Wall -Os -o $@ $^
	strip $@

pwn: $(BUILD_DIR)/pwn
	sudo install -m 06711 $< /usr/local/bin
