#include "sample.skel.c"
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <xdp/libxdp.h>
#include <linux/bpf.h>
#include <stdio.h>
#include <unistd.h>
#include <net/if.h>
#include "bpf_types.h"

unsigned
get_if_idx(const char *if_name)
{
	unsigned idx = if_nametoindex(if_name);
	if (idx == 0 && errno != 0) {
		fprintf(stderr, "Can't find interface %s: %s\n", if_name, strerror(errno));
		exit(1);
	}
	return idx;
}

int main()
{
	struct sample_bpf *prog = sample_bpf__open();
	// bpf_map__set_max_entries(prog->maps.stats, 10);
	// int r = sample_bpf__load(prog);
	int m = bpf_map__fd(prog->maps.stats);
	struct bpf_link *l = bpf_link__open("/sys/fs/bpf/xample");
	if (l) {
		struct bpf_map *mm = bpf_object__find_map_by_name(prog->obj, "stats");
		printf("map_by_name => %p\nstats =>       %p\n", mm, prog->maps.stats);
		// int fd = bpf_map_get_fd_by_id(42);
		// printf("find_by_id => %d\n", fd);
		// bpf_map__reuse_fd(prog->maps.stats, fd);
		int r = sample_bpf__load(prog);
#if 0
		int r = sample_bpf__attach(prog);
		if (r == 0 && errno != 0) {
			fprintf(stderr, "Error attaching: %s\n", strerror(errno));
			return 1;
		}
		printf("attach => %d\n", r);
		__u32 e = bpf_map__max_entries(prog->maps.stats);
#endif
		int zero = 0;
		struct my_stats v;
		bpf_map__lookup_elem(prog->maps.stats, &zero, sizeof(zero), &v, sizeof(v), 0);
		printf("pkts => %llu\n", v.pkts);
		// m = bpf_map__fd(prog->maps.stats);
		// printf("max_entries = %u, fd = %d\n", -1, m);

#if 0
		if (bpf_link__update_program(l, prog->progs.ip_filter) == 0) {
			fprintf(stderr, "BPF program updated\n");
		} else {
			bpf_link__unpin(l);
			fprintf(stderr, "BPF unloaded due to unable of being replaced\n");
		}
#endif
		return 0;
	} else {
		__u32 e = bpf_map__max_entries(prog->maps.stats);
		printf("max_entries = %u, fd = %d\n", e, m);

		printf("links: %p\n", prog->links.ip_filter);

		int if_idx = get_if_idx("lan25g");
		printf("attaching to %d\n", if_idx);
		l = bpf_program__attach_xdp(prog->progs.ip_filter, if_idx);
		if (!l) {
			fprintf(stderr, "Failed to attach bpf program\n");
			exit(1);
		}
		bpf_link__pin(l, "/sys/fs/bpf/xample");
	}

	for (;;) {

		sleep(1);
	}
	return 0;
}
