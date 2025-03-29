#include "utils.h"
#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void get_mac(char* mac_address, const char* interface);
void send_message(amqp_connection_state_t conn, char* name, double value);
// host port value
int main(int argc, char const* const* argv)
{

    char const* hostname;
    char const* user;
    char const* pass;
    char const* interface;
    int port = 5672, status;
    int rate_limit;
    int message_count;
    amqp_socket_t* socket = NULL;
    amqp_connection_state_t conn;

    if (argc < 6) {
        fprintf(stderr, "Usage: usagepkg host port user pass interface\n");
        return 1;
    }
    char mac[15] = {};
    get_mac(mac, interface);
    printf("%s", mac);

    hostname = argv[1];
    port = atoi(argv[2]);
    user = argv[3];
    pass = argv[4];
    interface = argv[5];

    long cpu_count = sysconf(_SC_NPROCESSORS_ONLN);

    // todo:
    //         write the service
    //     set up build server
    //     test
    //     figure out deb hosting

    conn = amqp_new_connection();

    socket = amqp_tcp_socket_new(conn);
    if (!socket) {
        die("died creating TCP socket");
    }
    status = amqp_socket_open(socket, hostname, port);
    if (status) {
        die("died opening TCP socket");
    }

    die_on_amqp_error(
        amqp_login(conn, "/", 0, 4096, 0, AMQP_SASL_METHOD_PLAIN, user, pass),
        "Logging in");

    double loads[3] = {};
    getloadavg(loads, 3);

    amqp_channel_open(conn, 1);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

    send_message(conn, mac, loads[0]);
    die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS),
        "Closing channel");
    die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS),
        "Closing connection");
    die_on_error(amqp_destroy_connection(conn), "Ending connection");
}

void send_message(amqp_connection_state_t conn, char* name, double value)
{
    char message[255];
    amqp_basic_properties_t props;

    sprintf(message, "{'HostName':'%s','Usage':'%f'}", name, value);

    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_literal_bytes("text/plain");
    props.delivery_mode = 2; /* persistent delivery mode */
    die_on_error(amqp_basic_publish(conn, 1, amqp_literal_bytes("InterTopic"),
                     amqp_cstring_bytes("node.usage"), 0, 0, &props,
                     amqp_cstring_bytes(message)),
        "Publishing");
}

static void get_mac(char* mac_address, const char* interface)
{
    FILE* fptr;
    char unfiltered[19] = {};
    char file_name[100];

    sprintf(file_name, "/sys/class/net/%s/address", interface);

    fptr = fopen(file_name, "r");

    if (fptr) {
        fgets(unfiltered, 18, fptr);
        fclose(fptr);
    } else {
        printf("Interface %s doesn't exist", interface);
        exit(-1);
    }
    int output_index = 0;
    for (int i = 0; i < 18; i++) {
        if ((i % 3) == 2) {
            i++;
        }
        mac_address[output_index++] = unfiltered[i];
    }
}
