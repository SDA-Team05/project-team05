/*
File: wiretap/wtap.h
This struct represents the Strategy role.
*/

struct file_type_subtype_info {
    const char *description;
    const char *name;
    const char *default_file_extension;
    const char *additional_file_extensions;
    bool writing_must_seek;
    size_t num_supported_blocks;
    const struct supported_block_type *supported_blocks;
    int (*can_write_encap)(int);
    bool (*dump_open)(wtap_dumper *, int *, char **);
    wtap_wslua_file_info_t *wslua_info;
};

/*The two following examples show the Concrete Strategy role
File: wiretap/pcapng.c*/

static const struct file_type_subtype_info wireshark_pcapng_info = {
    "Wireshark/... - pcapng", "pcapng", "pcapng", "ntar",
    false, BLOCKS_SUPPORTED(pcapng_blocks_supported),
    pcapng_dump_can_write_encap, pcapng_dump_open, NULL
};

/*File: wiretap/json.c*/

static const struct file_type_subtype_info json_info = {
    "JavaScript Object Notation", "json", "json", NULL,
    false, BLOCKS_SUPPORTED(json_blocks_supported),
    NULL, NULL, NULL
};

/*The Context role is developed into wiretap/file_access.c
First, strategies are added to global table*/

int
wtap_register_file_type_subtype(const struct file_type_subtype_info* fi)
{
    ...
    file_type_subtype = file_type_subtype_table_arr->len;
    g_array_append_val(file_type_subtype_table_arr, *fi);
    file_type_subtype_table = (const struct file_type_subtype_info*)(void *)file_type_subtype_table_arr->data;
    return file_type_subtype;
}

/*Then, strategies are initialized*/

void
wtap_init_file_type_subtypes(const char* app_env_var_prefix)
{
    ...
    register_pcapng(app_env_var_prefix);
    register_pcap();

    for (unsigned i = 0; i < wtap_module_count; i++)
        wtap_module_reg[i].cb_func();
    ...
}

/*try_open examines strategies and invokes the correct one by try_one_open*/

static int
try_open(wtap *wth, unsigned int type, int *err, char **err_info)
{
    ...
    for (i = 0; i < heuristic_open_routine_idx && result == WTAP_OPEN_NOT_MINE; i++) {
        result = try_one_open(wth, &open_routines[i], err, err_info);
    }
    ...
}

static int
try_one_open(wtap *wth, const struct open_info *candidate, int *err, char **err_info)
{
    ...
    return candidate->open_routine(wth, err, err_info);
}

/*wtap_dump_open_finish invokes dump_open of the selected strategy and wtap_dump delegates the writing of the strategy*/

static bool
wtap_dump_open_finish(wtap_dumper *wdh, int *err, char **err_info)
{
    ...
    if (!(*file_type_subtype_table[wdh->file_type_subtype].dump_open)(wdh, err,
        err_info)) {
        return false;
    }
    return true;
}

bool
wtap_dump(wtap_dumper *wdh, const wtap_rec *rec, int *err, char **err_info)
{
    *err = 0;
    *err_info = NULL;
    return (wdh->subtype_write)(wdh, rec, err, err_info);
}
