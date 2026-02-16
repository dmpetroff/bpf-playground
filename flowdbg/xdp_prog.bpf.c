#include <linux/types.h>
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

struct edt_state {
	__u64 pkt_no;
};

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, __u32);
	__type(value, struct edt_state);
	__uint(max_entries, 1);
} ip_state SEC(".maps");

static __u32 zero = 0;

SEC("xdp")
int shaper(struct xdp_md *xdp)
{
	struct edt_state *s = bpf_map_lookup_elem(&ip_state, &zero);
	if (s == NULL) {
		struct edt_state val = {0};
		bpf_map_update_elem(&ip_state, &zero, &val, BPF_NOEXIST);
		s = bpf_map_lookup_elem(&ip_state, &zero);
	}
	if (s != NULL) {
		__u64 no = __sync_add_and_fetch(&s->pkt_no, 1);
		if ((no % 5) == 1)
			return XDP_DROP;
	}
	return XDP_PASS;
}

