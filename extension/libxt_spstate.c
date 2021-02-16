#include <xtables.h>
#include <string.h>
#include <stdio.h>

struct xt_spstate_mtinfo {
	uint8_t state;
	uint8_t invert;
};
#define XT_SPSTATE_NONE		0
#define XT_SPSTATE_IN_PROGRESS	1
#define XT_SPSTATE_FINISH	2

enum {
	O_NONE = 0,
	O_IN_PROGRESS,
	O_FINISH
};

static void spstate_mt_help(void)
{
	printf(
"spstate match options:\n"
" --none    Recheck connection by timer expired\n"
" --finish    Recheck connection by timer expired\n"
" --in-progress  Check related connection\n");
}

static const struct xt_option_entry spstate_mt_opts[] = {
	{.name = "none", .id = O_NONE, .type = XTTYPE_NONE,
	 .flags = XTOPT_INVERT},
	{.name = "finish", .id = O_FINISH, .type = XTTYPE_NONE,
	 .flags = XTOPT_INVERT},
	{.name = "in-progress", .id = O_IN_PROGRESS, .type = XTTYPE_NONE,
	 .flags = XTOPT_INVERT},
	XTOPT_TABLEEND,
};

static void spstate_mt_print(const void *ip, const struct xt_entry_match *match,
                       int numeric)
{

	const struct xt_spstate_mtinfo *info = (struct xt_spstate_mtinfo *)match->data;

	if (info->invert)
		printf(" !");
	switch(info->state) {
	case XT_SPSTATE_NONE:
		printf(" none");
		break;
	case XT_SPSTATE_IN_PROGRESS:
		printf(" in-progress");
		break;
	case XT_SPSTATE_FINISH:
		printf(" finish");
		break;
	}
}

static void spstate_mt_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_spstate_mtinfo *info = (struct xt_spstate_mtinfo *)match->data;

	if (info->invert)
		printf(" !");
	switch(info->state) {
	case XT_SPSTATE_NONE:
		printf(" --none");
		break;
	case XT_SPSTATE_IN_PROGRESS:
		printf(" --in-progress");
		break;
	case XT_SPSTATE_FINISH:
		printf(" --finish");
		break;
	}
}

static void spstate_mt_parse(struct xt_option_call *cb)
{
	struct xt_spstate_mtinfo *info = cb->data;

	xtables_option_parse(cb);
	if (cb->invert)
		info->invert = 1;
	switch (cb->entry->id) {
	case O_NONE:
		info->state = XT_SPSTATE_NONE;
		break;
	case O_IN_PROGRESS:
		info->state = XT_SPSTATE_IN_PROGRESS;
		break;
	case O_FINISH:
		info->state = XT_SPSTATE_FINISH;
		break;
	}
}

static struct xtables_match spstate_mt_reg[] = {
	{
		.version       = XTABLES_VERSION,
		.name          = "spstate",
		.revision      = 0,
		.family        = NFPROTO_IPV4,
		.size          = XT_ALIGN(sizeof(struct xt_spstate_mtinfo)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_spstate_mtinfo)),
		.help          = spstate_mt_help,
		.print         = spstate_mt_print,
		.save          = spstate_mt_save,
		.x6_parse      = spstate_mt_parse,
		.x6_options    = spstate_mt_opts,
	},
};

void _init(void)
{
	xtables_register_matches(spstate_mt_reg, ARRAY_SIZE(spstate_mt_reg));
}
