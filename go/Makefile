export CGO_ENABLED := 0

all:
	go generate
	go build

tool:
	go get -tool github.com/cilium/ebpf/cmd/bpf2go