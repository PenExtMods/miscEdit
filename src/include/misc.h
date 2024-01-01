#pragma once

#define AB_SLOT_NUM  2

/* Magic for the A/B struct when serialized. */
#define AVB_AB_MAGIC "\0AB0"
#define AVB_AB_MAGIC_LEN 4

/* Versioning for the on-disk A/B metadata - keep in sync with avbtool. */
#define AVB_AB_MAJOR_VERSION 1
#define AVB_AB_MINOR_VERSION 0

/* Size of AvbABData struct. */
#define AVB_AB_DATA_SIZE 32

/* Maximum values for slot data */
#define AVB_AB_MAX_PRIORITY 15
#define AVB_AB_MAX_TRIES_REMAINING 7

#define MISC_OFFSET 0x800
//#define MISC_OFFSET 2128
#define MISC_PARTITION_NMAE "/dev/block/by-name/misc"

#define MISC_UPDATE_INFO_OFFSET 0x100000
#define MISC_UPDATE_INFO_LENGTH 512

#define MISC_bootloaderMessage_Offset 0x4000
#define MISC_bootloaderMessage_Length 2048

#define MISC_wipeCmdline_Offset 0x1800
#define MISC_wipeCmdline_Length 20

typedef struct AvbABSlotData {
    /* Slot priority. Valid values range from 0 to AVB_AB_MAX_PRIORITY,
     * both inclusive with 1 being the lowest and AVB_AB_MAX_PRIORITY
     * being the highest. The special value 0 is used to indicate the
     * slot is unbootable.
     */
    unsigned char priority;//0,14,15

    /* Number of times left attempting to boot this slot ranging from 0
     * to AVB_AB_MAX_TRIES_REMAINING.
     */
    unsigned char tries_remaining;//7--,成功启动，设为0

    /* Non-zero if this slot has booted successfully, 0 otherwise. */
    unsigned char successful_boot;//0,1

    /* Reserved for future use. */
    unsigned char reserved[1];
}AvbABSlotData;

/* Struct used for recording A/B metadata.
 *
 * When serialized, data is stored in network byte-order.
 */
typedef struct AvbABData {
    /* Magic number used for identification - see AVB_AB_MAGIC. */
    unsigned char magic[AVB_AB_MAGIC_LEN];

    /* Version of on-disk struct - see AVB_AB_{MAJOR,MINOR}_VERSION. */
    unsigned char version_major; //AVB_AB_MAJOR_VERSION
    unsigned char version_minor; //AVB_AB_MINOR_VERSION

    /* Padding to ensure |slots| field start eight bytes in. */
    unsigned char reserved1[2];

    /* Per-slot metadata. */
    AvbABSlotData slots[2];

    /* Reserved for future use. */
    unsigned char last_boot;//默认a，上一次成功启动slot的标志位，0-->a，1-->b
    unsigned char reserved2[11];

    /* CRC32 of all 28 bytes preceding this field. */
    unsigned int crc32;
}AvbABData;


/* Bootloader Message
 *
 * This structure describes the content of a block in flash
 * that is used for recovery and the bootloader to talk to
 * each other.
 *
 * The command field is updated by linux when it wants to
 * reboot into recovery or to update radio or bootloader firmware.
 * It is also updated by the bootloader when firmware update
 * is complete (to boot into recovery for any final cleanup)
 *
 * The status field is written by the bootloader after the
 * completion of an "update-radio" or "update-hboot" command.
 *
 * The recovery field is only written by linux and used
 * for the system to send a message to recovery or the
 * other way around.
 *
 * The systemFlag field is used for the system to send a message to recovery.
 */
struct bootloaderMessage_t {
    char command[32];
    char status[32];
    char recovery[768];
    char needUpdate[4];
    char systemFlag[252];
};

typedef struct updateInfo_t{
    char magic[4];
    char info[504];
    unsigned int crc32;
}updateInfo_t;


typedef struct miscData_t{
    AvbABData main;
    bootloaderMessage_t bootloaderMessage;
    updateInfo_t updateInfo;
    char wipeCmdline[MISC_wipeCmdline_Length];
}miscData_t;

int readMisc(struct miscData_t *data, const char *fileName);
void displayMisc(struct miscData_t *data);
int writeMisc(struct miscData_t *data, const char *fileName,bool updateCrc=true);
