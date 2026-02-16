RATE := 1m
CLANG := clang

SUDO := pwn
IF_IN := ingress
IF_IN_IP := 192.168.69.69
IF_OUT := egress
IF_OUT_IP := 192.168.69.96
NETMASK := 24
OUT_NS := xdp_prot
BUILD_DIR := /tmp/ratelim


PORT := 1234
RCV_EXE := $(BUILD_DIR)/flowrcv
SND_EXE := $(BUILD_DIR)/flowsnd
XDP_OBJ := $(BUILD_DIR)/xdp_prog.bpf.o

all:
	@/bin/echo -e "  Prepare with\n\tmake setup"
	@/bin/echo -e "  Then start receiver\n\tmake run-rcv"
	@/bin/echo -e "  Receiver will print amount of bytes received per second"
	@/bin/echo -e "  Then start senders specifying rate\n\tmake run-snd RATE=12k"
	@/bin/echo -e "  Senders will spawn pv for rate limiting"
	@/bin/echo -e "  Remove all interfaces, namespaces etc with\n\tmake teardown"

# Go userspace programs
run-rcv: $(RCV_EXE)
	$(SUDO) ip netns exec $(OUT_NS) $(RCV_EXE)

run-snd: $(SND_EXE)
	$(SND_EXE) $(IF_OUT_IP):$(PORT) $(RATE)

$(BUILD_DIR)/%: %/main.go | $(BUILD_DIR)
	go build -o $@ $<

# XDP stuff
run-xdp: $(XDP_OBJ)
	-$(SUDO) ip netns exec $(OUT_NS) ip link set dev $(IF_OUT) xdpgeneric off
	$(SUDO) ip netns exec $(OUT_NS) ip link set dev $(IF_OUT) xdpgeneric obj $(XDP_OBJ) sec xdp

stop-xdp:
	$(SUDO) ip netns exec $(OUT_NS) ip link set dev $(IF_OUT) xdpgeneric off

$(BUILD_DIR)/%.bpf.o: %.bpf.c | $(BUILD_DIR)
	$(CLANG) -target bpf -g -O2 -Wall -c -o$@ $<

setup:
	$(SUDO) ip netns add $(OUT_NS)
	$(SUDO) ip link add dev $(IF_IN) type veth peer name $(IF_OUT)
	$(SUDO) ifconfig $(IF_IN) $(IF_IN_IP)/$(NETMASK) up
	$(SUDO) ip link set $(IF_OUT) netns $(OUT_NS)
	$(SUDO) ip netns exec $(OUT_NS) ifconfig $(IF_OUT) $(IF_OUT_IP)/$(NETMASK) up

teardown:
	-$(SUDO) ip link delete $(IF_IN)
	-$(SUDO) ip netns del $(OUT_NS)

$(BUILD_DIR):
	mkdir $@
