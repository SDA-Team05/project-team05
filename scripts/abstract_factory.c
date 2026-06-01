
/*
File: epan/proto.h
This line below defines the Product, the structure that represents the created protocol
*/
typedef struct _protocol protocol_t;

/*
File: epan/proto.c
This function below represents the Abstract Factory, the common interface that defines the "contract" that all Concrete Factories have to use in order to create protocols
*/
int proto_register_protocol(const char *name, const char *short_name,
                            const char *filter_name)
{
    protocol_t *protocol;
    header_field_info *hfinfo;

    check_protocol_filter_name_or_fail(filter_name);

    /* Here we can find the allocation and configuration of the Product (protocol_t) */

    protocol = g_new(protocol_t, 1);
    protocol->name = name;
    protocol->short_name = short_name;
    protocol->filter_name = filter_name;
    protocol->fields = NULL;
    protocol->is_enabled = true;
    protocol->enabled_by_default = true;
    protocol->can_toggle = true;
    protocol->parent_proto_id = -1;
    protocol->heur_list = NULL;

    /* ... */

    return protocol->proto_id; /* It returns the unique ID of the created product */
}

/*
File: epan/dissectors/packet-tcp.c (Example for TCP)
proto_register_tcp is the specific registration function for this dissectors and it represents the Concrete Factory. The function calls the Abstract Factory in order to create
and register its own specific protocol.
*/
void proto_register_tcp(void)
{
    /* ... */

    /* It invokes the factory using as parameters TCP specific's details */
    proto_tcp = proto_register_protocol("Transmission Control Protocol", "TCP", "tcp");

    /* ... */
}
