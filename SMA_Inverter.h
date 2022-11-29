/* MIT License

Copyright (c) 2022 Lupo135

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef SMA_INVERTER_SEEN
#define SMA_INVERTER_SEEN


#define tokWh(value64)  (double)(value64)/1000
#define tokW(value32)   (float)(value32)/1000
#define toHour(value64) (double)(value64)/3600
#define toAmp(value32)  (float)value32/1000
#define toVolt(value32) (float)value32/100
#define toHz(value32)   (float)value32/100
#define toTemp(value32) (float)value32/100


#define UG_USER      0x07
#define UG_INSTALLER 0x0A

enum E_RC {
    E_OK =            0,    // No error
    E_INIT =         -1,    // Unable to initialise
    E_INVPASSW =     -2,    // Invalid password
    E_RETRY =        -3,    // Retry the last action
    E_EOF =          -4,    // End of data
    E_NODATA =       -5,    // no data
    E_OVERFLOW =     -6,    // data buffer overflow
    E_BADARG =       -7,    // Invalid data type
    E_CHKSUM =       -8,    // Invalid checksum
    E_INVRESP =      -9,    // Invalid response
};


struct InverterData {
    uint8_t BTAddress[6];
    uint8_t SUSyID;
    uint32_t Serial;
    uint8_t NetID;
    int32_t Pmax;
    int32_t Pac;
    int32_t Uac;
    int32_t Iac;
    int32_t Udc;
    int32_t Idc;
    int32_t Freq;
    uint64_t EToday;
    uint64_t ETotal;
    uint64_t OperationTime;
    uint64_t FeedInTime;
    E_RC     status;
};


enum getInverterDataType {
    EnergyProduction    = 1 << 0,
    SpotDCPower         = 1 << 1,
    SpotDCVoltage       = 1 << 2,
    SpotACPower         = 1 << 3,
    SpotACVoltage       = 1 << 4,
    SpotGridFrequency   = 1 << 5,
    //MaxACPower        = 1 << 6,
    //MaxACPower2       = 1 << 7,
    SpotACTotalPower    = 1 << 8,
    TypeLabel           = 1 << 9,
    OperationTime       = 1 << 10,
    SoftwareVersion     = 1 << 11,
    DeviceStatus        = 1 << 12,
    GridRelayStatus     = 1 << 13,
    BatteryChargeStatus = 1 << 14,
    BatteryInfo         = 1 << 15,
    InverterTemp        = 1 << 16,
    MeteringGridMsTotW  = 1 << 17,
    sbftest             = 1 << 31
};

enum LriDef {
    OperationHealth                 = 0x2148,   // *08* Condition (aka INV_STATUS)
    CoolsysTmpNom                   = 0x2377,   // *40* Operating condition temperatures
    DcMsWatt                        = 0x251E,   // *40* DC power input (aka SPOT_PDC1 / SPOT_PDC2)
    MeteringTotWhOut                = 0x2601,   // *00* Total yield (aka SPOT_ETOTAL)
    MeteringDyWhOut                 = 0x2622,   // *00* Day yield (aka SPOT_ETODAY)
    GridMsTotW                      = 0x263F,   // *40* Power (aka SPOT_PACTOT)
    BatChaStt                       = 0x295A,   // *00* Current battery charge status
    OperationHealthSttOk            = 0x411E,   // *00* Nominal power in Ok Mode (deprecated INV_PACMAX1)
    OperationHealthSttWrn           = 0x411F,   // *00* Nominal power in Warning Mode (deprecated INV_PACMAX2)
    OperationHealthSttAlm           = 0x4120,   // *00* Nominal power in Fault Mode (deprecated INV_PACMAX3)
    OperationGriSwStt               = 0x4164,   // *08* Grid relay/contactor (aka INV_GRIDRELAY)
    OperationRmgTms                 = 0x4166,   // *00* Waiting time until feed-in
    DcMsVol                         = 0x451F,   // *40* DC voltage input (aka SPOT_UDC1 / SPOT_UDC2)
    DcMsAmp                         = 0x4521,   // *40* DC current input (aka SPOT_IDC1 / SPOT_IDC2)
    MeteringPvMsTotWhOut            = 0x4623,   // *00* PV generation counter reading
    MeteringGridMsTotWhOut          = 0x4624,   // *00* Grid feed-in counter reading
    MeteringGridMsTotWhIn           = 0x4625,   // *00* Grid reference counter reading
    MeteringCsmpTotWhIn             = 0x4626,   // *00* Meter reading consumption meter
    MeteringGridMsDyWhOut           = 0x4627,   // *00* ?
    MeteringGridMsDyWhIn            = 0x4628,   // *00* ?
    MeteringTotOpTms                = 0x462E,   // *00* Operating time (aka SPOT_OPERTM)
    MeteringTotFeedTms              = 0x462F,   // *00* Feed-in time (aka SPOT_FEEDTM)
    MeteringGriFailTms              = 0x4631,   // *00* Power outage
    MeteringWhIn                    = 0x463A,   // *00* Absorbed energy
    MeteringWhOut                   = 0x463B,   // *00* Released energy
    MeteringPvMsTotWOut             = 0x4635,   // *40* PV power generated
    MeteringGridMsTotWOut           = 0x4636,   // *40* Power grid feed-in
    MeteringGridMsTotWIn            = 0x4637,   // *40* Power grid reference
    MeteringCsmpTotWIn              = 0x4639,   // *40* Consumer power
    GridMsWphsA                     = 0x4640,   // *40* Power L1 (aka SPOT_PAC1)
    GridMsWphsB                     = 0x4641,   // *40* Power L2 (aka SPOT_PAC2)
    GridMsWphsC                     = 0x4642,   // *40* Power L3 (aka SPOT_PAC3)
    GridMsPhVphsA                   = 0x4648,   // *00* Grid voltage phase L1 (aka SPOT_UAC1)
    GridMsPhVphsB                   = 0x4649,   // *00* Grid voltage phase L2 (aka SPOT_UAC2)
    GridMsPhVphsC                   = 0x464A,   // *00* Grid voltage phase L3 (aka SPOT_UAC3)
    GridMsAphsA_1                   = 0x4650,   // *00* Grid current phase L1 (aka SPOT_IAC1)
    GridMsAphsB_1                   = 0x4651,   // *00* Grid current phase L2 (aka SPOT_IAC2)
    GridMsAphsC_1                   = 0x4652,   // *00* Grid current phase L3 (aka SPOT_IAC3)
    GridMsAphsA                     = 0x4653,   // *00* Grid current phase L1 (aka SPOT_IAC1_2)
    GridMsAphsB                     = 0x4654,   // *00* Grid current phase L2 (aka SPOT_IAC2_2)
    GridMsAphsC                     = 0x4655,   // *00* Grid current phase L3 (aka SPOT_IAC3_2)
    GridMsHz                        = 0x4657,   // *00* Grid frequency (aka SPOT_FREQ)
    MeteringSelfCsmpSelfCsmpWh      = 0x46AA,   // *00* Energy consumed internally
    MeteringSelfCsmpActlSelfCsmp    = 0x46AB,   // *00* Current self-consumption
    MeteringSelfCsmpSelfCsmpInc     = 0x46AC,   // *00* Current rise in self-consumption
    MeteringSelfCsmpAbsSelfCsmpInc  = 0x46AD,   // *00* Rise in self-consumption
    MeteringSelfCsmpDySelfCsmpInc   = 0x46AE,   // *00* Rise in self-consumption today
    BatDiagCapacThrpCnt             = 0x491E,   // *40* Number of battery charge throughputs
    BatDiagTotAhIn                  = 0x4926,   // *00* Amp hours counter for battery charge
    BatDiagTotAhOut                 = 0x4927,   // *00* Amp hours counter for battery discharge
    BatTmpVal                       = 0x495B,   // *40* Battery temperature
    BatVol                          = 0x495C,   // *40* Battery voltage
    BatAmp                          = 0x495D,   // *40* Battery current
    NameplateLocation               = 0x821E,   // *10* Device name (aka INV_NAME)
    NameplateMainModel              = 0x821F,   // *08* Device class (aka INV_CLASS)
    NameplateModel                  = 0x8220,   // *08* Device type (aka INV_TYPE)
    NameplateAvalGrpUsr             = 0x8221,   // *  * Unknown
    NameplatePkgRev                 = 0x8234,   // *08* Software package (aka INV_SWVER)
    InverterWLim                    = 0x832A,   // *00* Maximum active power (deprecated INV_PACMAX1_2) (SB3300/SB1200)
    GridMsPhVphsA2B6100             = 0x464B,
    GridMsPhVphsB2C6100             = 0x464C,
    GridMsPhVphsC2A6100             = 0x464D
};

// MultiPacket:
//  Packet-first: Cmd0=08 Byte[18]=0x7E
//  Packet-last : Cmd1=01=cmdcodetowait 
#pragma pack(1) //bracket was missing. not sure if it supposed to have push or pull also..
typedef struct __attribute__ ((packed)) PacketHeader {
    uint8_t   SOP;                // Start Of Packet (0x7E)
    uint16_t  pkLength;
    uint8_t   pkChecksum;
    uint8_t   SourceAddr[6];      // SMA Inverter Address
    uint8_t   DestinationAddr[6]; // Local BT Address
    uint16_t  command;
} L1Hdr;


#endif //SMA_INVERTER_SEEN
