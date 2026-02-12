#include <linux/types.h>
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include "bpf_types.h"

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, __u32);
	__type(value, struct my_stats);
	__uint(max_entries, 1);
} stats SEC(".maps");

static int zero = 0;

SEC("xdp")
int ip_filter(struct xdp_md *xdp)
{
	struct my_stats *s = bpf_map_lookup_elem(&stats, &zero);
	if (s != NULL) {
		__sync_add_and_fetch(&s->bytes, xdp->data_end - xdp->data);
		__sync_add_and_fetch(&s->pkts, 1);
	}
	return XDP_PASS;
}
