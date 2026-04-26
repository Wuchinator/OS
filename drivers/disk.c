#include "disk.h"
#include "string.h"


static disk_info_t disk_drives[2];


static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static void ata_io_delay(void) {
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
}

static void ata_copy_identify_string(const uint16_t* identify_data,
                                     uint32_t word_offset,
                                     uint32_t word_count,
                                     char* dest) {
    uint32_t len = word_count * 2;

    for (uint32_t i = 0; i < word_count; i++) {
        uint16_t word = identify_data[word_offset + i];
        dest[i * 2] = (char)(word >> 8);
        dest[i * 2 + 1] = (char)(word & 0xFF);
    }

    dest[len] = '\0';
    while (len > 0 && dest[len - 1] == ' ') {
        dest[--len] = '\0';
    }
}

static int disk_wait_ready(void) {
    
    for (int i = 0; i < 100000; i++) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (!(status & ATA_SR_BSY)) {
            return 0;
        }
        
        ata_io_delay();
    }
    return -1; 
}


static int disk_wait_drq(void) {
    for (int i = 0; i < 100000; i++) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (!(status & ATA_SR_BSY)) {
            if (status & (ATA_SR_ERR | ATA_SR_DF)) {
                return -1;
            }

            if (status & ATA_SR_DRQ) {
                return 0;
            }

            if (status == 0) {
                return -1;
            }
        }

        ata_io_delay();
    }
    return -1;
}


static int disk_identify(int drive, disk_info_t* info) {
    memset(info, 0, sizeof(disk_info_t));

    if (disk_wait_ready() != 0) {
        return -1;
    }
    
    
    outb(ATA_PRIMARY_DRIVE, drive == 0 ? ATA_DRIVE_MASTER : ATA_DRIVE_SLAVE);
    ata_io_delay();
    outb(ATA_PRIMARY_SECCOUNT, 0);
    outb(ATA_PRIMARY_LBA_LOW, 0);
    outb(ATA_PRIMARY_LBA_MID, 0);
    outb(ATA_PRIMARY_LBA_HIGH, 0);
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_IDENTIFY);
    ata_io_delay();
    
    
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status == 0 || (status & (ATA_SR_ERR | ATA_SR_DF))) {
        return -1;
    }
    
    
    if (disk_wait_drq() != 0) {
        return -1;
    }
    
    
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(ATA_PRIMARY_DATA);
    }
    
    
    info->exists = 1;
    info->is_atapi = (identify_data[0] & 0x8000) != 0;
    info->size_sectors = identify_data[60] | ((uint32_t)identify_data[61] << 16);
    
    ata_copy_identify_string(identify_data, 27, 20, info->model);
    ata_copy_identify_string(identify_data, 10, 10, info->serial);
    ata_copy_identify_string(identify_data, 23, 4, info->firmware);
    
    return 0;
}


void disk_init(void) {
    
    disk_identify(0, &disk_drives[0]);
    
    
    disk_identify(1, &disk_drives[1]);
}


int disk_get_info(int drive, disk_info_t* info) {
    if (drive < 0 || drive > 1 || !info) {
        return -1;
    }
    
    *info = disk_drives[drive];
    return disk_drives[drive].exists ? 0 : -1;
}


int disk_read(int drive, uint32_t lba, uint8_t* buffer, uint32_t count) {
    if (drive < 0 || drive > 1 || !buffer) {
        return -1;
    }

    if (!disk_drives[drive].exists) {
        return -1;
    }
    
    if (count == 0 || count > 256) {
        return -1;
    }

    if (lba >= disk_drives[drive].size_sectors ||
        count > (disk_drives[drive].size_sectors - lba)) {
        return -1;
    }
    
    if (disk_wait_ready() != 0) {
        return -1;
    }
    
    
    outb(ATA_PRIMARY_DRIVE, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, count & 0xFF);
    outb(ATA_PRIMARY_LBA_LOW, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HIGH, (lba >> 16) & 0xFF);
    
    
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_SECTORS);
    
    
    uint16_t* word_buffer = (uint16_t*)buffer;
    for (uint32_t sector = 0; sector < count; sector++) {
        if (disk_wait_drq() != 0) {
            return -1;
        }
        
        
        for (int i = 0; i < 256; i++) {
            word_buffer[i] = inw(ATA_PRIMARY_DATA);
        }
        
        word_buffer += 256;
    }
    
    return 0;
}


int disk_write(int drive, uint32_t lba, const uint8_t* buffer, uint32_t count) {
    if (drive < 0 || drive > 1 || !buffer) {
        return -1;
    }

    if (!disk_drives[drive].exists) {
        return -1;
    }
    
    if (count == 0 || count > 256) {
        return -1;
    }

    if (lba >= disk_drives[drive].size_sectors ||
        count > (disk_drives[drive].size_sectors - lba)) {
        return -1;
    }
    
    if (disk_wait_ready() != 0) {
        return -1;
    }
    
    
    outb(ATA_PRIMARY_DRIVE, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, count & 0xFF);
    outb(ATA_PRIMARY_LBA_LOW, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HIGH, (lba >> 16) & 0xFF);
    
    
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_SECTORS);
    
    
    const uint16_t* word_buffer = (const uint16_t*)buffer;
    for (uint32_t sector = 0; sector < count; sector++) {
        if (disk_wait_drq() != 0) {
            return -1;
        }
        
        
        for (int i = 0; i < 256; i++) {
            outw(ATA_PRIMARY_DATA, word_buffer[i]);
        }
        
        word_buffer += 256;
    }
    
    
    if (disk_wait_ready() != 0) {
        return -1;
    }
    
    return 0;
}
