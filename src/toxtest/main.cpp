#include "stdafx.h"
#include "tox/tox.h"
#include "utils.h"

static void handle_friend_request(Tox *tox, const uint8_t *public_key, const uint8_t *message, size_t length, void *user_data)
{
    unsigned char tox_printable_id[TOX_PUBLIC_KEY_SIZE * 2 + 1];
    memset(tox_printable_id, '\0', sizeof(tox_printable_id));
    id_to_string(tox_printable_id, (unsigned char*)public_key, TOX_PUBLIC_KEY_SIZE);

    TOX_ERR_FRIEND_ADD err_friend_add;
    uint32_t friend_number = tox_friend_add_norequest(tox, public_key, &err_friend_add);
    if (err_friend_add != TOX_ERR_FRIEND_ADD_OK)
    {
        printf("unable to add friend: %d\n", err_friend_add);
    }
    else
    {
        printf("friend[%d] %s join in\n", friend_number, tox_printable_id);
    }
}

static void handle_friend_message(Tox *tox, uint32_t friend_number, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, void *user_data)
{
    TOX_ERR_FRIEND_GET_PUBLIC_KEY err_getpublickey;
    uint8_t public_key[TOX_PUBLIC_KEY_SIZE];
    unsigned char tox_printable_id[TOX_PUBLIC_KEY_SIZE * 2 + 1];
    memset(tox_printable_id, '\0', sizeof(tox_printable_id));
    
    tox_friend_get_public_key(tox, friend_number, public_key, &err_getpublickey);
    id_to_string(tox_printable_id, (unsigned char*)public_key, TOX_PUBLIC_KEY_SIZE);

    printf("friend[%d] %s said: %s\n", friend_number, tox_printable_id, message);
}

void send_message_to_all_friend(Tox *tox, uint8_t* msg, int length)
{
    int size = tox_self_get_friend_list_size(tox);
    uint32_t* friend_array = (uint32_t*)malloc(sizeof(uint32_t) * size);
    tox_self_get_friend_list(tox, friend_array);

    printf("you say: %s\n", msg);
    for (int i = 0; i < size; i++)
    {
        TOX_ERR_FRIEND_SEND_MESSAGE err_send;
        tox_friend_send_message(tox, friend_array[i], TOX_MESSAGE_TYPE_NORMAL, msg, length, &err_send);
    }
}

void add_bootstrap_by_dht_id(Tox *tox, unsigned char* dhtid_str)
{
    uint8_t dhtid[TOX_PUBLIC_KEY_SIZE];
    TOX_ERR_BOOTSTRAP err;

    string_to_id(dhtid, dhtid_str, TOX_PUBLIC_KEY_SIZE);

    tox_bootstrap(tox, "192.168.45.76", 33445, dhtid, &err);

    if (err == TOX_ERR_BOOTSTRAP_OK)
    {
        printf("send bootstrap to 192.168.45.76 ok\n");
    }
    else
    {
        printf("send bootstrap to 192.168.45.76 failed %d\n", err);
    }
}

void add_friend_by_toxid(Tox *tox, unsigned char* toxstr)
{
    TOX_ERR_FRIEND_ADD err;
    char message[32];
    strcpy(message, "hello!");

    uint8_t toxid[TOX_ADDRESS_SIZE];

    string_to_id(toxid, toxstr, TOX_ADDRESS_SIZE);

    tox_friend_add(tox, toxid, (uint8_t*)message, strlen(message) + 1, &err);

    if (err != TOX_ERR_FRIEND_ADD_OK)
    {
        printf("friend %s add failed %d\r\n", toxstr, err);
    }
    else
    {
        printf("friend %s add OK\r\n", toxstr);
    }
}

void set_tox_username(Tox *tox)
{
    char hostname[1024];
    TOX_ERR_SET_INFO error;

    gethostname((char*)hostname, 1024);
    hostname[1023] = '\0';

    tox_self_set_name(tox, (uint8_t *)hostname, strlen(hostname), &error);
}

uint8_t bootstrap_key[] = {
    0x21, 0xF1, 0x30, 0x6A, 0xC9, 0xD8, 0x89, 0x6F,
    0x1B, 0x86, 0x9A, 0xD9, 0xDB, 0xD8, 0x23, 0x35,
    0x13, 0xD0, 0x3E, 0x42, 0xBC, 0x77, 0x33, 0xA8,
    0x37, 0x39, 0x8C, 0x49, 0x61, 0x64, 0x3E, 0x1F
};

void on_tox_log(Tox *tox, TOX_LOG_LEVEL level, const char *path, uint32_t line, const char *func,
		const char *message, void *user_data)
{
    uint32_t index = user_data ? *(uint32_t *)user_data : 0;
    const char *file = strrchr(path, '/');

    file = file ? file + 1 : path;
    printf("[#%d] %s:%d\t%s:\t%s\n", index, file, line, func, message);
}

static DWORD WINAPI ToxMainThread(LPVOID Lp)
{
    Tox* tox = (Tox*)Lp;

    while (true) {
        Sleep(tox_iteration_interval(tox));
        tox_iterate(tox, NULL);
    }

    return 0;
}

int main(int argc, char** argv)
{
    TOX_ERR_NEW err_new;
    Tox_Options tox_options;
    DWORD threadid;
    char szMessage[512];
    unsigned char tox_id[TOX_ADDRESS_SIZE];
    unsigned char tox_printable_id[TOX_ADDRESS_SIZE * 2 + 1];
    unsigned char dht_key[TOX_PUBLIC_KEY_SIZE];
    unsigned char readable_dht_key[2 * TOX_PUBLIC_KEY_SIZE + 1];
    size_t save_size = 0;
    uint8_t *save_data = NULL;
    BOOL debug = FALSE;

    if (argc > 1 && strcmp((char*)argv[1], "-d") == 0)
    {
        debug = TRUE;
    }

    tox_options_default(&tox_options);
    tox_options.ipv6_enabled = false;
    tox_options.local_discovery_enabled = false;
    if (debug)
    {
        tox_options.log_callback = on_tox_log;
    }
   /* save_size = load_save(&save_data);
    if (save_data && save_size)
    {
        tox_options.savedata_type = TOX_SAVEDATA_TYPE_TOX_SAVE;
        tox_options.savedata_data = save_data;
        tox_options.savedata_length = save_size;
    }*/

    Tox *tox = tox_new(&tox_options, &err_new);
    if (err_new != TOX_ERR_NEW_OK) {
        printf("tox_new failed with error code %d\n", err_new);
        return 0;
    }

    tox_callback_friend_request(tox, handle_friend_request);
    tox_callback_friend_message(tox, handle_friend_message);

    //write_save(tox);

    set_tox_username(tox);
    tox_self_get_address(tox, (uint8_t*)tox_id);
    memset(tox_printable_id, '\0', sizeof(tox_printable_id));
    id_to_string(tox_printable_id, tox_id, TOX_ADDRESS_SIZE);
    tox_printable_id[TOX_ADDRESS_SIZE * 2] = '\0';
    printf("Tox ID: %s\n", tox_printable_id);

    tox_self_get_dht_id(tox, dht_key);
    id_to_string(readable_dht_key, dht_key, TOX_PUBLIC_KEY_SIZE);
    printf("DHT key: %s\n", readable_dht_key);

    CreateThread(NULL, 0, ToxMainThread, tox, 0, &threadid);


    do {
       gets_s(szMessage, 511);
       if (szMessage[0] == '#' && szMessage[1] == ' ' && strlen(szMessage) == 2 + TOX_ADDRESS_SIZE * 2)
       {
           add_friend_by_toxid(tox, (unsigned char*)&szMessage[2]);
       }
       else if (szMessage[0] == '!' && szMessage[1] == ' ' && strlen(szMessage) == 2 + TOX_PUBLIC_KEY_SIZE * 2)
       {
           add_bootstrap_by_dht_id(tox, (unsigned char*)&szMessage[2]);
       }
       else
       {
           send_message_to_all_friend(tox, (uint8_t*)szMessage, strlen(szMessage) + 1);
       }
    } while(TRUE);

}