#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>		
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <linux/ip.h>
#include <linux/tcp.h>

static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
	u_int32_t id;
	const struct iphdr *iph;
	int ret = 0;
	uint32_t data_len;
	int packet_id;
	struct nfqnl_msg_packet_hdr* nfq_ph = nfq_get_msg_packet_hdr(nfa);
	packet_id = ntohl(nfq_ph->packet_id);

	ret = nfq_get_payload(nfa, (unsigned char**) &iph);
	id = ntohl(nfq_ph->packet_id);

	if (iph->protocol == IPPROTO_TCP) {
		struct tcphdr *tcp = (struct tcphdr *)((uint8_t *)iph + iph->ihl * 4);
		char *data = (char *)tcp + tcp->doff * 4;
		uint16_t data_len = ntohs(iph->tot_len) - (data - (char *)iph);

		/* This is just an example, so there are no sanity checks here. */
		if (data_len > 4) {
			if (!memcmp(data, "GET ", 4)) {
				printf("catch TCP -> HTTP\n");
				return nfq_set_verdict2(qh, id, NF_ACCEPT, 11, 0, NULL);
			}
		}
	}
	return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

int main(int argc, char **argv)
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	int fd;
	int rv;
	char buf[4096] __attribute__ ((aligned));

	h = nfq_open();
	if (!h) {
		fprintf(stderr, "error during nfq_open()\n");
		exit(1);
	}

	if (nfq_unbind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_unbind_pf()\n");
		exit(1);
	}

	if (nfq_bind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_bind_pf()\n");
		exit(1);
	}

	qh = nfq_create_queue(h,  0, &cb, NULL);
	if (!qh) {
		fprintf(stderr, "error during nfq_create_queue()\n");
		exit(1);
	}

	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet_copy mode\n");
		exit(1);
	}

	fd = nfq_fd(h);

	while ((rv = recv(fd, buf, sizeof(buf), 0)))
	{
		nfq_handle_packet(h, buf, rv);
	}

	nfq_destroy_queue(qh);

#ifdef INSANE
	nfq_unbind_pf(h, AF_INET);
#endif

	nfq_close(h);

	exit(0);
}
