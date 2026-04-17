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


static int disk_wait_ready(void) {
    
    for (int i = 0; i < 100000; i++) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (!(status & ATA_SR_BSY)) {
            return 0;
        }
        
        inb(ATA_PRIMARY_STATUS);
        inb(ATA_PRIMARY_STATUS);
        inb(ATA_PRIMARY_STATUS);
        inb(ATA_PRIMARY_STATUS);
    }
    return -1; 
}


static int disk_wait_drq(void) {
    for (int i = 0; i < 100000; i++) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_SR_DRQ) {
            return 0;
        }
        inb(ATA_PRIMARY_STATUS);
        inb(ATA_PRIMARY_STATUS);
        inb(ATA_PRIMARY_STATUS);
        inb(ATA_PRIMARY_STATUS);
    }
    return -1;
}


static int disk_identify(int drive, disk_info_t* info) {
    disk_wait_ready();
    
    
    outb(ATA_PRIMARY_DRIVE, drive == 0 ? ATA_DRIVE_MASTER : ATA_DRIVE_SLAVE);
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_IDENTIFY);
    
    
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status == 0) {
        info->exists = 0;
        return -1;
    }
    
    
    if (disk_wait_drq() != 0) {
        info->exists = 0;
        return -1;
    }
    
    
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(ATA_PRIMARY_DATA);
    }
    
    
    info->exists = 1;
    info->is_atapi = (identify_data[0] & 0x8000) != 0;
    info->size_sectors = identify_data[60] | (identify_data[61] << 16);
    
    
    for (int i = 0; i < 20; i++) {
        info->model[i * 2] = identify_data[27 + i] & 0xFF;
        info->model[i * 2 + 1] = identify_data[27 + i] >> 8;
    }
    info->model[40] = '\0';
    
    
    for (int i = 0; i < 10; i++) {
        info->serial[i * 2] = identify_data[10 + i] & 0xFF;
        info->serial[i * 2 + 1] = identify_data[10 + i] >> 8;
    }
    info->serial[20] = '\0';
    
    
    for (int i = 0; i < 4; i++) {
        info->firmware[i * 2] = identify_data[23 + i] & 0xFF;
        info->firmware[i * 2 + 1] = identify_data[23 + i] >> 8;
    }
    info->firmware[8] = '\0';
    
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
    if (!disk_drives[drive].exists) {
        return -1;
    }
    
    if (count == 0 || count > 256) {
        return -1;
    }
    
    disk_wait_ready();
    
    
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
    if (!disk_drives[drive].exists) {
        return -1;
    }
    
    if (count == 0 || count > 256) {
        return -1;
    }
    
    disk_wait_ready();
    
    
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
