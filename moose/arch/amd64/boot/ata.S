/*
 ATA disk driver. It is used in stage 2 bootloader to load the kernel (aka
 stage3 bootloader that sets up long mode).
 This disk driver uses ATA 28 bit PIO mode
*/
.code32
.global ata_read_sectors

#define PRIMARY_BUS 0x1f0
#define SECONDARY_BUS 0x170

#define DATA_REG 0x0
#define ERR_REG 0x1
#define FEAT_REG 0x1
#define SEC_CNT_REG 0x2
#define SEC_NUM_REG 0x3
#define CYL_LOW_REG 0x4
#define CYL_HIG_REG 0x5
#define DRIVE_HEAD_REG 0x6
#define STAT_CMD_REG 0x7

/*
 ecx = sector count 
 esi = sector addr
 edi = dest addr

 reads 'master' drive 
*/
ata_read_sectors:
    /* 0xe0 indicates that we will use lba */
    mov $0xe0, %al
    mov %esi, %ebx
    shr $24, %ebx
    or %bl, %al
    mov $PRIMARY_BUS + DRIVE_HEAD_REG, %dx
    out %al, %dx
    mov $0, %al
    mov $PRIMARY_BUS + FEAT_REG, %dx
    out %al, %dx
    mov %cl, %al
    mov $PRIMARY_BUS + SEC_CNT_REG, %dx
    out %al, %dx
    mov %esi, %eax
    mov $PRIMARY_BUS + SEC_NUM_REG, %dx
    out %al, %dx
    shr $8, %eax
    mov $PRIMARY_BUS + CYL_LOW_REG, %dx
    out %al, %dx
    shr $8, %eax
    mov $PRIMARY_BUS + CYL_HIG_REG, %dx
    out %al, %dx
    /* send read sectors command */
    mov $20, %al
    mov $PRIMARY_BUS + STAT_CMD_REG, %dx
    out %al, %dx
    /* perform io delay */
    mov $4, %ecx
1:
    in %dx, %al
    test $0x80, %al
    jne 2f
    test $8, %al
    jne 4f
2:
    dec %ecx
    jg 1b
3:
    in %dx, %al
    test $0x80, %al
    jne 3b
    test $0x21, %al
    jne fail
4:
    mov $PRIMARY_BUS + DATA_REG, %dx
    mov $256, %cx
    rep insw
    /* delay for 400ns */
    mov $PRIMARY_BUS + STAT_CMD_REG, %dx
    in %dx, %al
    in %dx, %al
    in %dx, %al
    in %dx, %al

    dec %ecx
    test %cl, %cl
    jne 3b

    jmp done
fail:
    stc
done:
    ret
