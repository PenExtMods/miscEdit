#include <iostream>
#include <cstring>
#include <cstdlib>
#include "misc.h"

const int version[3] = {1, 0, 1};

const char *helpMsg = "\
Usage: miscedit [option]\
Option:\
  -f (miscFile)               specify the file to edit. if not specify it will use \"/dev/block/by-name/misc\".\
  -r                          read all info from misc.\
  -w (name)=(value)           write value to property not for solt.\
  -ws (slot),(name)=(value)   write value to property for slot.\
  --no-crc32                  no update crc32 value after modify misc.\
Example:\
  miscedit -f ./misc.bin -ws 2,priority=14\
  miscedit -w update.info=hello\\ world!\
  miscedit --no-crc32 -w crc32=112233ff\
  miscedit -w update.info=test -ws 0,priority=15 -ws 0,tries_remaining=3\
";

void printHelpMsg(bool exitAfterPrint = true)
{
    printf("miscedit\nVersion %d.%d.%d", version[0], version[1], version[2]);
    printf(helpMsg);
    if (exitAfterPrint)
        exit(0);
}

bool handleHelp(int n, char *arg[])
{
    if (n == 1)
        return true;
    for (int i = 1; i < n; i++)
    {
        if (!strcmp(arg[i], "--help") || !strcmp(arg[i], "-h") || !strcmp(arg[i], "-?"))
            return true;
    }
    return false;
}

bool handleFile(const char **str, int n, char *arg[])
{
    for (int i = 1; i < n; i++)
    {
        if (!strcmp(arg[i], "-f") && i < n - 1)
        {
            *str = arg[i + 1];
            return true;
        }
    }
    *str = MISC_DEFAULT;
    return false;
}

bool handleRead(miscData_t *data, int n, char *arg[])
{
    for (int i = 1; i < n; i++)
    {
        if (!strcmp(arg[i], "-r"))
        {
            displayMisc(data);
            return true;
        }
    }
    return false;
}

bool handleNoCrc(int n, char *arg[])
{
    for (int i = 1; i < n; i++)
    {
        if (!strcmp(arg[i], "--no-crc32"))
            return true;
    }
    return false;
}

void cutStr(char *str, char **left, char **right, const char *spliter)
{
    int len = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        len++;
        // printf("%s %s\n",((char*)(str+i)),spliter);
        if (*((char *)(str + i)) == *((char *)spliter))
            break;
    }
    char *out = (char *)malloc(len);
    if (out == 0)
        throw "malloc failed.";
    memset(out, 0, len);
    memcpy(out, str, len - 1);
    *left = out;
    *right = str + len;
}

char hexstr2str(char *input)
{
    char out = 0;
    for (int i = 0; i < 2; i++)
    {
        if (*(input + i) == 0)
            break;
        out *= 16;
        if (*(input + i) >= '0' && *(input + i) <= '9')
            out += (*(input + i) - '0');
        if (*(input + i) >= 'A' && *(input + i) <= 'F')
            out += (*(input + i) - 'A' + 10);
        if (*(input + i) >= 'a' && *(input + i) <= 'f')
            out += (*(input + i) - 'a' + 10);
    }
    return out;
}

char octstr2str(char *input)
{
    char out = 0;
    long len = strlen(input);
    for (int i = 0; i < len; i++)
    {
        if (*(input + i) == 0)
            break;
        out *= 10;
        if (*(input + i) >= '0' && *(input + i) <= '9')
            out += (*(input + i) - '0');
    }
    return out;
}

void doWrite(miscData_t *data, void *map[], int mapLen, char *name, char *value)
{
    for (int i = 0; i < mapLen; i += 4)
    {
        if (!strcmp(name, ((char *)map[i])))
        {
            memset(map[i + 1], 0, (unsigned long)map[i + 2]);
            switch ((long)map[i + 3])
            {
            case 0:
            {
                memcpy(map[i + 1], value, (strlen(value) < (unsigned long)map[i + 2] ? strlen(value) : (unsigned long)map[i + 2]));
                break;
            }
            case 1:
            {
                long len = strlen(value);
                for (int ii = 0; ii < len; ii += 2)
                {
                    if (ii / 2 == (long)map[i + 2])
                        break;
                    *(char *)((long)map[i + 1] + ii / 2) = hexstr2str((char *)(value + ii));
                }
                break;
            }
            case 2:
            {
                *(char *)((long)map[i + 1]) = octstr2str((char *)(value));
                break;
            }
            }
            return;
        }
    }
    printf("Not match any property name: %s.\n", name);
}

void doWriteSlot(miscData_t *data, void *map[], int mapLen, char *name, char *value, int slot)
{
    for (int i = 0; i < mapLen; i += 3)
    {
        if (!strcmp(((char *)map[i]), name))
        {
            void *ptr = (void *)((long)(&(data->main.slots)) + sizeof(AvbABSlotData) * slot + (long)map[i + 1]);
            memset(ptr, 0, (unsigned long)map[i + 2]);
            *(char *)((long)ptr) = octstr2str((char *)(value));
            return;
        }
    }
    printf("Not match any property name: slot[%d]->%s.\n", slot, name);
}

void handleWrite(miscData_t *data, int n, char *arg[])
{
    void *wMap[] = {
        (void *)"magic", &(data->main.magic), (void *)4, (void *)1,
        (void *)"version_major", &(data->main.version_major), (void *)1, (void *)2,
        (void *)"version_min", &(data->main.version_minor), (void *)1, (void *)2,
        (void *)"crc32", &(data->main.crc32), (void *)4, (void *)1,
        (void *)"last_boot", &(data->main.last_boot), (void *)1, (void *)2,
        (void *)"bootloader.command", &(data->bootloaderMessage.command), (void *)32, (void *)0,
        (void *)"bootloader.status", &(data->bootloaderMessage.status), (void *)32, (void *)0,
        (void *)"bootloader.recovery", &(data->bootloaderMessage.recovery), (void *)768, (void *)0,
        (void *)"bootloader.needUpdate", &(data->bootloaderMessage.needUpdate), (void *)4, (void *)0,
        (void *)"bootloader.systemFlag", &(data->bootloaderMessage.systemFlag), (void *)252, (void *)0,
        (void *)"cmdline", (data->wipeCmdline), (void *)MISC_WIPE_CMDLINE_LENGTH, (void *)0,
        (void *)"update.info", (data->updateInfo.info), (void *)504, (void *)0,
        (void *)"update.crc32", &(data->updateInfo.crc32), (void *)4, (void *)1
    };
    void *wsMap[] = {
        (void *)"priority", (void *)((long)(&(data->main.slots[0].priority)) - (long)(&(data->main.slots))), (void *)1,
        (void *)"tries_remaining", (void *)((long)(&(data->main.slots[0].tries_remaining)) - (long)(&(data->main.slots))), (void *)1,
        (void *)"successful_boot", (void *)((long)(&(data->main.slots[0].successful_boot)) - (long)(&(data->main.slots))), (void *)1
    };
    for (int i = 1; i < n; i++)
    {
        if (!strcmp(arg[i], "-w") && i < n - 1)
        {
            char *name = 0;
            char *value = 0;
            cutStr(arg[i + 1], &name, &value, "=");
            doWrite(data, wMap, sizeof(wMap) / sizeof(void *), name, value);
            free(name);
            i++;
            continue;
        }
        if (!strcmp(arg[i], "-ws") && i < n - 1)
        {
            char *name = 0;
            char *tmp = 0;
            char *value = 0;
            int slot = 0;
            cutStr(arg[i + 1], &tmp, &value, ",");
            if (*tmp != *"1" && *tmp != *"0")
            {
                printf("No such a slot: slot[%s].\n", tmp);
                continue;
            }
            slot = (*tmp == *"1");
            free(tmp);
            tmp = 0;
            cutStr(value, &name, &value, "=");
            doWriteSlot(data, wsMap, sizeof(wsMap) / sizeof(void *), name, value, slot);
            free(name);
            i++;
            continue;
        }
    }
}

int main(int argc, char *argv[])
{

    if (handleHelp(argc, argv))
    {
        printHelpMsg(true);
    }

    const char *fileName = 0;
    bool noCrc = false;

    handleFile(&fileName, argc, argv);
    if (handleFile(&fileName, argc, argv) && argc == 3)
        printHelpMsg(true);
    noCrc = handleNoCrc(argc, argv);

    miscData_t miscData;
    if (readMisc(&miscData, fileName) != 0)
    {
        printf("readMisc failed.\n");
        return 1;
    }

    if (handleRead(&miscData, argc, argv))
        return 0;

    handleWrite(&miscData, argc, argv);

    if (writeMisc(&miscData, fileName, !noCrc) != 0)
    {
        printf("writeMisc failed.\n");
        return 1;
    }

    return 0;
}