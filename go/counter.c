
// go:build ignore

#include <linux/types.h>
#include <bpf/bpf_helpers.h>
#include <linux/bpf.h>

typedef struct {
	__u64 packets;
	__u64 bytes;
} ingress_t;

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, __u32);
	__type(value, ingress_t);
	__uint(max_entries, 1);
} pkt_count SEC(".maps");

// count_packets atomically increases a packet counter on every invocation.
SEC("xdp")
int
count_packets(struct xdp_md *ctx)
{
	__u32      key   = 0;
	ingress_t *count = bpf_map_lookup_elem(&pkt_count, &key);
	if (count) {
		__sync_fetch_and_add(&count->packets, 1);
		__sync_fetch_and_add(&count->bytes, ctx->data_end - ctx->data);
	}

	return XDP_PASS;
}

char __license[] SEC("license") = "Dual MIT/GPL";
