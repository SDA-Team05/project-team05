/* Subject role
File: epan/tap.h
Register a new tap point; returns a tap ID used by the dissector. */
WS_DLL_PUBLIC int register_tap(const char *name);
 
/* Called by a dissector after parsing a packet to push data to every listener registered on this tap. */

WS_DLL_PUBLIC void tap_queue_packet(int tap_id,
                                    packet_info *pinfo,
                                    const void *tap_specific_data);



/*Observer role (function pointer typedefs)
File: epan/tap.h*/

typedef void (*tap_reset_cb)(void *tapdata);
typedef tap_packet_status (*tap_packet_cb)(void *tapdata, packet_info *pinfo, epan_dissect_t *edt, const void *data, tap_flags_t flags);
typedef void (*tap_draw_cb)(void *tapdata);
typedef void (*tap_finish_cb)(void *tapdata);



/*Example of Concrete Subject 
File: epan/dissectors/packet-ip.c*/

/* Static handle: one per dissector, initialised at startup. */
static int ip_tap;
 
void proto_register_ip(void) {
    /*...*/
  
    ip_tap = register_tap("ip");
}

/*notify observers after dissection*/

static int dissect_ip_v4(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
    /*...*/
 
    p_add_proto_data(pinfo->pool, pinfo, proto_ip,
                     pinfo->curr_layer_num,
                     GUINT_TO_POINTER((unsigned)iph->ip_proto));
 
    tap_queue_packet(ip_tap, pinfo, iph); 
 
    /*...*/
}




/*Example of Concrete Observer
File: ui/cli/tap-icmpv6stat.c*/

static bool
icmpv6stat_init(const char *opt_arg, void *userdata _U_)
{
    icmpv6stat_t *icmpv6stat;
    const char *filter = NULL;
    GString *error_string;

    if (strstr(opt_arg, "icmpv6,srt,"))
        filter = opt_arg + strlen("icmpv6,srt,");

    icmpv6stat = (icmpv6stat_t *)g_try_malloc(sizeof(icmpv6stat_t));
    if (icmpv6stat == NULL) {
        cmdarg_err("Couldn't register icmpv6,srt tap: Out of memory");
        return false;
    }
    memset(icmpv6stat, 0, sizeof(icmpv6stat_t));
    icmpv6stat->min_msecs = 1.0 * UINT_MAX;

    icmpv6stat->filter = g_strdup(filter);

    error_string = register_tap_listener("icmpv6", icmpv6stat, icmpv6stat->filter,
        TL_REQUIRES_NOTHING, icmpv6stat_reset, icmpv6stat_packet, icmpv6stat_draw, icmpv6stat_finish);
    if (error_string) {

        g_free(icmpv6stat->filter);
        g_free(icmpv6stat);

        cmdarg_err("Couldn't register icmpv6,srt tap: %s", error_string->str);
        g_string_free(error_string, TRUE);
        return false;
    }

    return true;
}
