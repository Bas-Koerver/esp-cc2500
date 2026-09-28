#ifndef CC2500_REG_H
#define CC2500_REG_H

// CC2500 configuration registers
#define CC2500_REG_IOCFG2               0x00
#define CC2500_REG_IOCFG1               0x01
#define CC2500_REG_IOCFG0               0x02
#define CC2500_REG_FIFOTHR              0x03
#define CC2500_REG_SYNC1                0x04
#define CC2500_REG_SYNC0                0x05
#define CC2500_REG_PKTLEN               0x06
#define CC2500_REG_PKTCTRL1             0x07
#define CC2500_REG_PKTCTRL0             0x08
#define CC2500_REG_ADDR                 0x09
#define CC2500_REG_CHANNR               0x0A
#define CC2500_REG_FSCTRL1              0x0B
#define CC2500_REG_FSCTRL0              0x0C
#define CC2500_REG_FREQ2                0x0D
#define CC2500_REG_FREQ1                0x0E
#define CC2500_REG_FREQ0                0x0F
#define CC2500_REG_MDMCFG4              0x10
#define CC2500_REG_MDMCFG3              0x11
#define CC2500_REG_MDMCFG2              0x12
#define CC2500_REG_MDMCFG1              0x13
#define CC2500_REG_MDMCFG0              0x14
#define CC2500_REG_DEVIATN              0x15
#define CC2500_REG_MCSM2                0x16
#define CC2500_REG_MCSM1                0x17
#define CC2500_REG_MCSM0                0x18
#define CC2500_REG_FOCCFG               0x19
#define CC2500_REG_BSCFG                0x1A
#define CC2500_REG_AGCCTRL2             0x1B
#define CC2500_REG_AGCCTRL1             0x1C
#define CC2500_REG_AGCCTRL0             0x1D
#define CC2500_REG_WOREVT1              0x1E
#define CC2500_REG_WOREVT0              0x1F
#define CC2500_REG_WORCTRL              0x20
#define CC2500_REG_FREND1               0x21
#define CC2500_REG_FREND0               0x22
#define CC2500_REG_FSCAL3               0x23
#define CC2500_REG_FSCAL2               0x24
#define CC2500_REG_FSCAL1               0x25
#define CC2500_REG_FSCAL0               0x26
#define CC2500_REG_RCCTRL1              0x27
#define CC2500_REG_RCCTRL0              0x28
#define CC2500_REG_FSTEST               0x29
#define CC2500_REG_PTEST                0x2A
#define CC2500_REG_AGCTEST              0x2B
#define CC2500_REG_TEST2                0x2C
#define CC2500_REG_TEST1                0x2D
#define CC2500_REG_TEST0                0x2E
// 0x2F reserved.

// CC2500 status registers
#define CC2500_STATUS_PARTNUM           0x30
#define CC2500_STATUS_VERSION           0x31
#define CC2500_STATUS_FREQEST           0x32
#define CC2500_STATUS_LQI               0x33
#define CC2500_STATUS_RSSI              0x34
#define CC2500_STATUS_MARCSTATE         0x35
#define CC2500_STATUS_WORTIME1          0x36
#define CC2500_STATUS_WORTIME0          0x37
#define CC2500_STATUS_PKTSTATUS         0x38
#define CC2500_STATUS_VCO_VC_DAC        0x39
#define CC2500_STATUS_TXBYTES           0x3A
#define CC2500_STATUS_RXBYTES           0x3B
#define CC2500_STATUS_RCCTRL1_STATUS    0x3C
#define CC2500_STATUS_RCCTRL0_STATUS    0x3D

#define CC2500_PATABLE_ADDR             0x3E
#define CC2500_FIFO_ADDR                0x3F

#endif // CC2500_REG_H