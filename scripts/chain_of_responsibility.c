/*Handler role
File: epan/packet.h
dissector_t */

typedef int (*dissector_t)(tvbuff_t *, packet_info *, proto_tree *, void *);

/*File: epan/packet.c
dissector_handle*/

struct dissector_handle {
    const char      *name;          /* dissector name */
    const char      *description;   /* dissector description */
    char            *pref_suffix;
    enum dissector_e dissector_type;
    union {
        dissector_t    dissector_type_simple;
        dissector_cb_t dissector_type_callback;
    } dissector_func;
    void        *dissector_data;
    protocol_t  *protocol;
};

/*the dissector_table connects values to the next handler*/

struct dissector_table {
    GHashTable  *hash_table;        
    GSList      *dissector_handles;
    GHashTable  *da_descriptions;
    const char  *ui_name;
    ftenum_t     type;
    int          param;
    protocol_t  *protocol;
    GHashFunc    hash_func;
    bool         supports_decode_as;
};



/*Examples of Concrete Handler role
File: epan/dissectors/packet-igmp.c*/

static int
dissect_igmp(tvbuff_t *tvb, packet_info *pinfo, proto_tree *parent_tree, void* data _U_)
{
    int offset = 0;
    unsigned char type;

    type = tvb_get_uint8(tvb, offset);

    if (!dissector_try_uint(subdissector_table, type, tvb, pinfo, parent_tree))
    {
        dissect_igmp_unknown(tvb, pinfo, parent_tree);
    }
    return tvb_captured_length(tvb);
}

/*File: epan/dissectors/packet-eth.c*/
call_dissector(fw1_handle, tvb, pinfo, parent_tree);



/*Client role
File: epan/epan.c
Send the request*/

void
epan_dissect_run(epan_dissect_t *edt, int file_type_subtype,
    wtap_rec *rec, frame_data *fd, column_info *cinfo)
{
#ifdef HAVE_LUA
    wslua_prime_dfilter(edt);
#endif
    dissect_record(edt, file_type_subtype, rec, fd, cinfo);

    wtap_block_unref(rec->block);
    rec->block = NULL;
}

/*File: epan/packet.c
Chain building*/

void
dissect_record(epan_dissect_t *edt, int file_type_subtype, wtap_rec *rec,
    frame_data *fd, column_info *cinfo)
{
    /*...*/

    TRY {
        
        edt->tvb = tvb_new_real_data(ws_buffer_start_ptr(&rec->data),
                        fd->cap_len, fd->pkt_len);
        add_new_data_source(&edt->pi, edt->tvb, rec->rec_type_name);

        call_dissector_with_data(frame_handle, edt->tvb, &edt->pi,
                                 edt->tree, &frame_dissector_data);
    }
    CATCH(BoundsError) {
		    ws_assert_not_reached();
	  }
    CATCH2(FragmentBoundsError, ReportedBoundsError) {
        proto_tree_add_protocol_format(edt->tree, proto_malformed,
            edt->tvb, 0, 0, "[Malformed %s: Packet Length]",
            rec->rec_type_name);
    }
    ENDTRY;
	wtap_block_unref(rec->block);
	rec->block = NULL;

	fd->visited = 1;
}
