RATE := 1m
CLANG := clang

SUDO := sudo
# Идентификатор сетевого неймспейса
OUT_NS := xdp_prot
# Маска виртуальной подсети
NETMASK := 24
# Это virtual ethernet, который будет виден в "корневом" сетевом неймспейсе
IF_IN := ingress
IF_IN_IP := 192.168.69.69
# Это virtual ethernet, который будет виден в сетевом неймспейсе $(OUT_NS)
IF_OUT := egress
IF_OUT_IP := 192.168.69.96

# В наше время дорогих SSD лучше билдить в tmpfs
BUILD_DIR := /tmp/ratelim
# Флаги для bpf-программы
BPF_CFLAGS := -target bpf -Wall -g -O2

# Порт, на который будет лететь UDP-трафик
PORT := 1234
# Корневой исходник XDP-программы. Его зависимости будут автоматически отслеживаться.
XDP_SRC := xdp_prog.bpf.c

-include Makefile.local

RCV_EXE := $(BUILD_DIR)/flowrcv
SND_EXE := $(BUILD_DIR)/flowsnd
XDP_OBJ := $(patsubst %.c,%.o,$(BUILD_DIR)/$(notdir $(XDP_SRC)))
XDP_DEP := $(patsubst %,%.d,$(XDP_OBJ))

SUDO_NETNS := $(SUDO) ip netns exec $(OUT_NS)

all:
	@/bin/echo -e "  Prepare with\n\tmake setup"
	@/bin/echo -e "  Then start receiver\n\tmake run-rcv"
	@/bin/echo -e "  Receiver will print amount of bytes received per second"
	@/bin/echo -e "  Then start senders specifying rate\n\tmake run-snd RATE=12k"
	@/bin/echo -e "  Senders will spawn pv for rate limiting"
	@/bin/echo -e "  Remove all interfaces, namespaces etc with\n\tmake teardown"

build-xdp: $(XDP_OBJ)

check-route:
	$(SUDO) ip route get $(IF_OUT_IP) | grep --color=auto $(IF_IN)

# Go userspace programs
run-rcv: $(RCV_EXE)
	$(SUDO_NETNS) $(RCV_EXE)

run-snd: $(SND_EXE)
	$(SND_EXE) $(IF_OUT_IP):$(PORT) $(RATE)

$(BUILD_DIR)/%: %/main.go | $(BUILD_DIR)
	go build -o $@ $<

# XDP stuff
run-xdp: $(XDP_OBJ)
	-$(SUDO_NETNS) ip link set dev $(IF_OUT) xdpgeneric off
	$(SUDO_NETNS) ip link set dev $(IF_OUT) xdpgeneric obj $(XDP_OBJ) sec xdp

stop-xdp:
	$(SUDO_NETNS) ip link set dev $(IF_OUT) xdpgeneric off

$(XDP_OBJ) $(XDP_DEP): $(XDP_SRC) | $(BUILD_DIR)
	$(CLANG) $(BPF_CFLAGS) -MT $(XDP_OBJ) -MT $(XDP_OBJ) -MP -MD -MF $(XDP_DEP) -c -o $(XDP_OBJ) $<

setup:
	$(SUDO) ip netns add $(OUT_NS)
	$(SUDO) ip link add dev $(IF_IN) type veth peer name $(IF_OUT)
	$(SUDO) ifconfig $(IF_IN) $(IF_IN_IP)/$(NETMASK) up
	$(SUDO) ip link set $(IF_OUT) netns $(OUT_NS)
	$(SUDO_NETNS) ifconfig $(IF_OUT) $(IF_OUT_IP)/$(NETMASK) up

teardown:
	-$(SUDO) ip link delete $(IF_IN)
	-$(SUDO) ip netns del $(OUT_NS)

$(BUILD_DIR):
	mkdir -p $@

dep: $(XDP_DEP)

ifeq (,$(filter dep,$(MAKECMDGOALS)))
-include $(XDP_DEP)
endif
