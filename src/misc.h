#pragma once

/* == Partitions == */

// A/B slot data
#define MISC_OFFSET 0x800

// Update info
#define MISC_UPDATE_INFO_OFFSET 0x100000
#define MISC_UPDATE_INFO_LENGTH 512

// Bootloader message
#define MISC_BOOTLOADER_MESSAGE_OFFSET 0x4000
#define MISC_BOOTLOADER_MESSAGE_LENGTH 2048

// Wipe command
#define MISC_WIPE_CMDLINE_OFFSET 0x1800
#define MISC_WIPE_CMDLINE_LENGTH 20


/* == A/B slot struct definitions == */

// Magic for the A/B struct when serialized.
#define AVB_AB_MAGIC "\0AB0"
#define AVB_AB_MAGIC_LENGTH 4

// Versioning for the on-disk A/B metadata - keep in sync with avbtool.
#define AVB_AB_MAJOR_VERSION 1
#define AVB_AB_MINOR_VERSION 0


/* == Other == */

// Path of default misc file to use
#define MISC_DEFAULT "/dev/block/by-name/misc"



struct AvbABSlotData {
    /* Slot priority. Valid values range from 0 to AVB_AB_MAX_PRIORITY,
     * both inclusive with 1 being the lowest and AVB_AB_MAX_PRIORITY
     * being the highest. The special value 0 is used to indicate the
     * slot is unbootable.
     */
    unsigned char priority;

    /* Number of times left attempting to boot this slot ranging from 0
     * to AVB_AB_MAX_TRIES_REMAINING.
     */
    unsigned char tries_remaining;

    /* Non-zero if this slot has booted successfully, 0 otherwise. */
    unsigned char successful_boot;

    /* Reserved for future use. */
    unsigned char reserved[1];
};

/* Struct used for recording A/B metadata.
 *
 * When serialized, data is stored in network byte-order.
 */
struct AvbABData {
    /* Magic number used for identification - see AVB_AB_MAGIC. */
    unsigned char magic[AVB_AB_MAGIC_LENGTH];

    /* Version of on-disk struct - see AVB_AB_{MAJOR,MINOR}_VERSION. */
    unsigned char version_major; //AVB_AB_MAJOR_VERSION
    unsigned char version_minor; //AVB_AB_MINOR_VERSION

    /* Padding to ensure |slots| field start eight bytes in. */
    unsigned char reserved1[2];

    /* Per-slot metadata. */
    AvbABSlotData slots[2];

    /* Reserved for future use. */
    unsigned char last_boot;
    unsigned char reserved2[11];

    /* CRC32 of all 28 bytes preceding this field. */
    unsigned int crc32;
};


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
    char wipeCmdline[MISC_WIPE_CMDLINE_LENGTH];
}miscData_t;


int readMisc(struct miscData_t *data, const char *fileName);
void displayMisc(struct miscData_t *data);
int writeMisc(struct miscData_t *data, const char *fileName,bool updateCrc=true);
