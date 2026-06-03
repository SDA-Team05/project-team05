/*
File: epan/packet.c
The 'static' keyword keeps the global instance of the dissector tables hidden and inaccessible directly from outside this file, preventing accidental clones.
*/
static GHashTable *dissector_tables = NULL;

/*
This is the only public function allowed to safely access the Singleton instance.
*/
dissector_table_t
find_dissector_table(const char *name)
{
    /* Looks up and returns the value from the Single Source of Truth */
    dissector_table_t dissector_table = (dissector_table_t)g_hash_table_lookup(dissector_tables, name);

    /* Handle legacy aliases if the protocol name has changed */
    if (!dissector_table)
    {
        const char *new_name = (const char *)g_hash_table_lookup(dissector_table_aliases, name);
        if (new_name)
        {
            dissector_table = (dissector_table_t)g_hash_table_lookup(dissector_tables, new_name);
        }
        if (dissector_table)
        {
            ws_warning("%s is now %s", name, new_name);
        }
    }
    return dissector_table;
}