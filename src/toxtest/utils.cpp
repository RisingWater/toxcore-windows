#include "stdafx.h"
#include "tox/tox.h"

static void to_hex(unsigned char *a, unsigned char *p, int size)
{
    unsigned char b, c;
    unsigned char *end = p + size;

    while (p != end)
    {
        b = *p++;

        c = (b & 0xF);
        b = (b >> 4);

        if (b < 10)
        {
            *a++ = b + '0';
        }
        else
        {
            *a++ = b - 10 + 'A';
        }

        if (c < 10)
        {
            *a++ = c + '0';
        }
        else
        {
            *a++ = c - 10 + 'A';
        }
    }
    *a = '\0';
}

void id_to_string(unsigned char *dest, unsigned char *src, int size)
{
    to_hex(dest, src, size);
}

int char_to_int(char c)
{
    if (c >= '0' && c <= '9')
    {
        return (c - '0');
    }

    if (c >= 'A' && c <= 'F')
    {
        return (c - 'A') + 10;
    }

    return 0;
}

void string_to_id(unsigned char* dest, unsigned char* src, int size)
{
    src = (unsigned char*)strupr((char*)src);

    for (int i = 0; i < size; i++)
    {
        dest[i] = char_to_int(src[i * 2]) << 4 | char_to_int(src[i * 2 + 1]);
    }
}

static void* file_raw(char *path, uint32_t *size)
{
    FILE *file;
    char *data;
    int len;

    file = fopen(path, "rb");
    if (!file)
    {
        printf("File not found (%s)\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    len = ftell(file);
    if (len <= 0)
    {
        fclose(file);
        return NULL;
    }
    
    data = (char*)malloc(len);
    if (!data)
    {
        fclose(file);
        return NULL;
    }

    fseek(file, 0, SEEK_SET);

    if (fread(data, len, 1, file) != 1)
    {
        printf("Read error (%s)\n", path);
        fclose(file);
        free(data);
        return NULL;
    }

    fclose(file);

    printf("Read %u bytes (%s)\n", len, path);

    if (size) {
        *size = len;
    }
    return data;
}

size_t load_save(uint8_t **out_data)
{
    void *data;
    uint32_t size;
    uint8_t path_real[512], *p;
    char config_path[512];
    memset(config_path, 0, 512);
    GetTempPath(512, config_path);

    strncpy((char *)path_real, config_path, 512);

    p = path_real + strlen((char *)path_real);
    memcpy(p, "tox_save", sizeof("tox_save"));

    data = file_raw((char *)path_real, &size);

    if (data)
    {
        *out_data = (uint8_t*)data;
        return size;
    }
    else
    {
        printf("Could not open save file\n");
        return 0;
    }
}

void write_save(Tox *tox)
{
    void *data;
    uint32_t size;
    uint8_t path_tmp[512], path_real[512], *p;
    FILE *file;
    char config_path[512];
    memset(config_path, 0, 512);
    GetTempPath(512, config_path);

    size = tox_get_savedata_size(tox);
    data = malloc(size);
    tox_get_savedata(tox, (uint8_t*)data);

    strncpy((char *)path_real, config_path, 512);

    p = path_real + strlen((char *)path_real);
    memcpy(p, "tox_save", sizeof("tox_save"));

    unsigned int path_len = (p - path_real) + sizeof("tox_save");
    memcpy(path_tmp, path_real, path_len);
    memcpy(path_tmp + (path_len - 1), ".tmp", sizeof(".tmp"));

    file = fopen((char*)path_tmp, "wb");
    if (file) {
        fwrite(data, size, 1, file);
        fflush(file);
        fclose(file);
        if (rename((char*)path_tmp, (char*)path_real) != 0)
        {
            printf("Failed to rename file. %s to %s deleting and trying again\n", path_tmp, path_real);
            if (remove((const char *)path_real) < 0)
            {
                printf("Failed to remove old save file %s\n", path_real);
            }

            if (rename((char*)path_tmp, (char*)path_real) != 0)
            {
                printf("Saving Failed\n");
            }
            else
            {
                printf("Saved data\n");
            }
        }
        else
        {
            printf("Saved data\n");
        }
    }
    else
    {
        printf("Could not open save file\n");
    }

    free(data);
}