/*
 * CAmkES tutorial part 2: events and dataports
 */

#include <stdio.h>
#include <string.h>

#include <camkes/dataport.h>

/* generated header for our component */
#include <camkes.h>

/* strings to send across to the other component */
char *s_arr[NUM_STRINGS] = { "hello", "world", "how", "are", "you?" };

/* run the control thread */
int run(void) {
    printf("%s: Starting the client\n", get_instance_name());

    /* TASK 9: copy strings to an untyped dataport */
    /* hint 1: use the "Buf" dataport as defined in the Client.camkes file
     * hint 2: to access the dataport use the interface name as defined in Client.camkes.
     * For example if you defined it as "dataport Buf d" then you would use "d" to refer to the dataport in C.
     * hint 3: first write the number of strings (NUM_STRINGS) to the dataport
     * hint 4: then copy all the strings from "s_arr" to the dataport.
     * hint 5: look at https://github.com/seL4/camkes-tool/blob/master/docs/index.md#an-example-of-dataports
     */
    *(int*)c1 = NUM_STRINGS;
    // c1_release();

    char *p = (char*) c1 + 4;
    for (int i=0; i < NUM_STRINGS; i++) {
        strncpy(p, s_arr[i], STR_LEN - 1);
        p += strlen(s_arr[i]) + 1;
        // c1_release();
    }
    c1_release();

    /* TASK 10: emit event to signal that the data is available */
    /* hint 1: use the function <interface_name>_emit
     * hint 2: look at https://github.com/seL4/camkes-tool/blob/master/docs/index.md#an-example-of-events
     */
    puts("Client emit 1");
    emit_Client_emit();

    /* TASK 11: wait to get an event back signalling that the reply data is avaialble */
    /* hint 1: use the function <interface_name>_wait
     * hint 2: look at https://github.com/seL4/camkes-tool/blob/master/docs/index.md#an-example-of-events
     */
    // consume_Client_poll();
    puts("Client waiting");
    consume_Client_wait();
    puts("Client waited");

    /* TASK 12: read the reply data from a typed dataport */
    /* hint 1: use the "str_buf_t" dataport as defined in the Client.camkes file
     * hint 2: to access the dataport use the interface name as defined in Client.camkes.
     * For example if you defined it as "dataport str_buf_t d_typed" then you would use "d_typed" to refer to the dataport in C.
     * hint 3: for the definition of "str_buf_t" see "str_buf.h".
     * hint 4: use the "n" field to determine the number of strings in the dataport
     * hint 5: print out the specified number of strings from the "str" field
     * hint 6: look at https://github.com/seL4/camkes-tool/blob/master/docs/index.md#an-example-of-dataports
     */
    c2_acquire();
    str_buf_t client_str_buf = *(str_buf_t*) c2;
    for (int i=0; i < client_str_buf.n - 1; i++) {
        printf("%s ", client_str_buf.str[i]);
    }
    printf("%s\n", client_str_buf.str[client_str_buf.n - 1]);

    /* TASK 13: send the data over again, this time using two dataports, one
     * untyped dataport containing the data, and one typed dataport containing
     * dataport pointers pointing to data in the untyped, dataport.
     */
    /* hint 1: for the untyped dataport use the "Buf" dataport as defined in the Client.camkes file
     * hint 2: for the typed dataport use the "ptr_buf_t" dataport as defined in the Client.camkes file
     * hint 3: for the definition of "ptr_buf_t" see "str_buf.h".
     * hint 4: copy all the strings from "s_arr" into the untyped dataport
     * hint 5: use the "n" field of the typed dataport to specify the number of dataport pointers (NUM_STRINGS)
     * hint 6: use the "ptr" field of the typed dataport to store the dataport pointers
     * hint 7: use the function "dataport_wrap_ptr()" to create a dataport pointer from a regular pointer
     * hint 8: the dataport pointers should point into the untyped dataport
     * hint 9: for more information about dataport pointers see: https://github.com/seL4/camkes-tool/blob/master/docs/index.md
     */
    ptr_buf_t client_ptr_buf;
    client_ptr_buf.n = NUM_STRINGS;
    client_ptr_buf.ptr[0] = dataport_wrap_ptr(c1);
    p = (char*)c1;
    for (int i=0; i < NUM_STRINGS; i++) {
        strncpy(p, s_arr[i], STR_LEN - 1);
        client_ptr_buf.ptr[i+1] = dataport_wrap_ptr(p);
        p += strlen(s_arr[i]) + 1;
    }
    memcpy(c3, &client_ptr_buf, sizeof(client_ptr_buf));
    c3_release();

    /* TASK 14: emit event to signal that the data is available */
    /* hint 1: we've already done this before */
    puts("Client emit 2");
    emit_Client_emit();

    /* TASK 15: wait to get an event back signalling that data has been read */
    /* hint 1: we've already done this before */
    // consume_Client_poll();
    puts("Client waiting");
    consume_Client_wait();
    puts("Client waited");

    printf("%s: the next instruction will cause a vm fault due to permissions\n", get_instance_name());

    /* TASK 16: test the read and write permissions on the dataport.
     * When we try to write to a read-only dataport, we will get a VM fault.
     */
    /* hint 1: try to assign a value to a field of the "str_buf_t" dataport */

    *(int*)c2 = 0;
    c2_release();

    return 0;
}
