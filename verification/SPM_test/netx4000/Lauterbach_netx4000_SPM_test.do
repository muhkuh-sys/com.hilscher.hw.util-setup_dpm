reset // reset Lauterbach debugger before change to different CPU

system.reset

system.JTAGCLOCK 10Mhz

// Single Cortex R7
// Integrated inside new Lauterbach version
;system.cpu NETX4000-CR7

// Single Cortex R7
// manual setup for Lauterbach

SYStem.cpu CORTEXR7F
// Total number of Cores
// 1. core => CR7
;SYStem.CONFIG CoreNumber 1.   // NOT required because of a single CR7 CPU

// Assign single CR7 core into the GUI Processing.
;CORE.Assign 1. // NOT required because of a single CR7 CPU


SYStem.Option.TRST OFF 
SYStem.Option.EnReset ON
SYStem.Option ResBreak OFF
SYStem.Option WaitReset 200ms
TrOnchip.Set RESET OFF // RESET Vektor aus -> Firmware wird normal gestartet


SYStem.CONFIG.COREDEBUG.Base DAP:0x80130000

; SYStem.CONFIG.BMC.Base DAP:0x80131000 // not supported inside Lauterbach version before 
// TRACE32 PowerView for ARM
//    Release Feb 2014 SP1 (64-bit)
//    Software Version: S.2015.06.000063732X
//    Build: 63732.         06/2015

SYStem.CONFIG.CTI.Base DAP:0x80138000 
SYStem.CONFIG.CTI.Config CortexV1

SYStem.CONFIG.ETM.Base DAP:0x8013c000 

SYStem.CONFIG.ITM1.Base DAP:0x80005000 

SYStem.CONFIG.FUNNEL2.Base DAP:0x80121000 
SYStem.CONFIG.FUNNEL2.ATBSource ETMDATA 0 ETM 1

SYStem.CONFIG.ETF1.Base DAP:0x80122000 
SYStem.CONFIG.ETF1.ATBSource FUNNEL2 

SYStem.CONFIG.FUNNEL1.Base DAP:0x80004000 
SYStem.CONFIG.FUNNEL1.ATBSource  ETF1 2 ITM1 3 

SYStem.CONFIG.ETB1.Base DAP:0x80001000 
SYStem.CONFIG.ETB1.ATBSource FUNNEL1 


//system.MEMACCESS DAP


// Disable access to anything, as accesses to undefined areas cause bus hang at SCIT board
map.reset
map.denyaccess 0x00000000--0xFFFFFFFF

map.NODENYACCESS 0x00000000--0x0001FFFF ;Allow access to CR7 ROM mirror (default ITCM 128kByte)
map.NODENYACCESS 0x00020000--0x0003FFFF ;Allow access to default DTCM 128kByte
map.NODENYACCESS 0x04000000--0x040FFFFF ;Allow access to LLRAM  1MByte
map.NODENYACCESS 0x04100000--0x0411FFFF ;Allow access to CR7 ROM  128kByte

// AHB RAMs inside RAP area
map.NODENYACCESS 0x05000000--0x0501FFFF ;Allow access to AHB RAM0 128kByte
map.NODENYACCESS 0x05020000--0x0503FFFF ;Allow access to AHB RAM1 128kByte
map.NODENYACCESS 0x05040000--0x0505FFFF ;Allow access to AHB RAM2 128kByte
map.NODENYACCESS 0x05060000--0x0507FFFF ;Allow access to AHB RAM3 128kByte
// INT RAMs inside netX area
map.NODENYACCESS 0x05080000--0x0509FFFF ;Allow access to NETX RAM0 128kByte
map.NODENYACCESS 0x050A0000--0x050BFFFF ;Allow access to NETX RAM1 128kByte
map.NODENYACCESS 0x050C0000--0x050DFFFF ;Allow access to NETX RAM2 128kByte
map.NODENYACCESS 0x050E0000--0x050EFFFF ;Allow access to NETX HS0  64kByte
map.NODENYACCESS 0x050F0000--0x050FFFFF ;Allow access to NEXT HS1  64kByte
map.NODENYACCESS 0x05100000--0x0517FFFF ;Allow access to INTRAM_ETH 32kByte

// HS RAMs 
map.NODENYACCESS 0x05200000--0x0523FFFF ;Allow access to NETX HS0  256kByte
map.NODENYACCESS 0x05240000--0x0527FFFF ;Allow access to NETX HS1  256kByte


// Mirror AHB RAMs inside RAP area
map.NODENYACCESS 0x06000000--0x0601FFFF ;Allow access to AHB RAM0 128kByte
map.NODENYACCESS 0x06020000--0x0603FFFF ;Allow access to AHB RAM1 128kByte
map.NODENYACCESS 0x06040000--0x0605FFFF ;Allow access to AHB RAM2 128kByte
map.NODENYACCESS 0x06060000--0x0607FFFF ;Allow access to AHB RAM3 128kByte
// Mirror INT RAMs inside netX area
map.NODENYACCESS 0x06080000--0x0609FFFF ;Allow access to NETX RAM0 128kByte
map.NODENYACCESS 0x060A0000--0x060BFFFF ;Allow access to NETX RAM1 128kByte
map.NODENYACCESS 0x060C0000--0x060DFFFF ;Allow access to NETX RAM2 128kByte
map.NODENYACCESS 0x060E0000--0x060EFFFF ;Allow access to NETX HS0  64kByte
map.NODENYACCESS 0x060F0000--0x060FFFFF ;Allow access to NEXT HS1  64kByte
map.NODENYACCESS 0x06100000--0x0617FFFF ;Allow access to INTRAM_ETH 32kByte

// mirror HS RAMs 
map.NODENYACCESS 0x06200000--0x0623FFFF ;Allow access to NETX HS0  256kByte
map.NODENYACCESS 0x06240000--0x0627FFFF ;Allow access to NETX HS1  256kByte


// Mirror AHB RAMs inside RAP area
map.NODENYACCESS 0x07000000--0x0701FFFF ;Allow access to AHB RAM0 128kByte
map.NODENYACCESS 0x07020000--0x0703FFFF ;Allow access to AHB RAM1 128kByte
map.NODENYACCESS 0x07040000--0x0705FFFF ;Allow access to AHB RAM2 128kByte
map.NODENYACCESS 0x07060000--0x0707FFFF ;Allow access to AHB RAM3 128kByte
// Mirror INT RAMs inside netX area
map.NODENYACCESS 0x07080000--0x0709FFFF ;Allow access to NETX RAM0 128kByte
map.NODENYACCESS 0x070A0000--0x070BFFFF ;Allow access to NETX RAM1 128kByte
map.NODENYACCESS 0x070C0000--0x070DFFFF ;Allow access to NETX RAM2 128kByte
map.NODENYACCESS 0x070E0000--0x070EFFFF ;Allow access to NETX HS0  64kByte
map.NODENYACCESS 0x070F0000--0x070FFFFF ;Allow access to NEXT HS1  64kByte
map.NODENYACCESS 0x07100000--0x0717FFFF ;Allow access to INTRAM_ETH 32kByte

// mirror HS RAMs 
map.NODENYACCESS 0x07200000--0x0723FFFF ;Allow access to NETX HS0  256kByte
map.NODENYACCESS 0x07240000--0x0727FFFF ;Allow access to NETX HS1  256kByte


// NAND flash RAM area
map.NODENYACCESS 0x0A000000--0x0AFFFFFF ;Allow access to PL353 NAND flash RAM area 16MByte

// SQI XiP RAM area
map.NODENYACCESS 0x10000000--0x13FFFFFF ;Allow access to SQI0 64MByte
map.NODENYACCESS 0x14000000--0x17FFFFFF ;Allow access to SQI1 64MByte

// PL353 NOR/SRAM 
map.NODENYACCESS 0x18000000--0x1BFFFFFF ;Allow access to PL353 RAM CS0 64MByte
map.NODENYACCESS 0x1C000000--0x1FFFFFFF ;Allow access to PL353 RAM CS1 64MByte

// netX external SDRAM memory interface
map.NODENYACCESS 0x20000000--0x2FFFFFFF ;Allow access to HIFMEM 256MByte
map.NODENYACCESS 0x30000000--0x3FFFFFFF ;Allow access to EXTMEM 256MByte

// DDR
map.NODENYACCESS 0x40000000--0xBFFFFFFF ;Allow access to DDR (2GB max.)

// PCIe
map.NODENYACCESS 0xC0000000--0xDFFFFFFF ;Allow access to PCIe Slave (256MByte)

// netX external Flash/NVRAM/SRAM  memory interface
map.NODENYACCESS 0xE0000000--0xE7FFFFFF ;Allow access to HIFMEM 128MByte
map.NODENYACCESS 0xE8000000--0xEFFFFFFF ;Allow access to EXTMEM 128MByte

// CoreSight APB Debugport
map.NODENYACCESS 0xF0000000--0xF3FFFFFF ;Allow access to PCIe Slave (64MByte)

// netX ERICH interface
map.NODENYACCESS 0xF4000000--0xF407FFFF ;Allow Access to netX ERICH (512kByte)

// netX REG interface
map.NODENYACCESS 0xF4080000--0xF7FFFFFF ;Allow Access to netX ERICH (63.5MByte)
//map.NODENYACCESS 0xF4000000--0xF438FFFF ;Allow Access to netX Peripherals

// RAP Peripherals Register Interfaces 
map.NODENYACCESS 0xF8000000--0xF91FFFFF ;Allow Access to RAP Peripherals

// mirror HS RAMs 
map.NODENYACCESS 0xF9200000--0xF923FFFF ;Allow access to NETX HS0  256kByte
map.NODENYACCESS 0xF9240000--0xF927FFFF ;Allow access to NETX HS1  256kByte


map.NODENYACCESS 0xFAF00000--0xFAF01FFF ;Allow Access to R7/A9 Private Peripherals 8kByte
map.NODENYACCESS 0xFAF10000--0xFAF10FFF ;Allow Access to L2CC config register space (A9_ACP) 4kByte

map.NODENYACCESS 0xFB000000--0xFB0007FF ;Allow Access to Firewall   2kByte
map.NODENYACCESS 0xFB100000--0xFB1007FF ;Allow Access to Port Control   2kByte
map.NODENYACCESS 0xFB200000--0xFB2001FF ;Allow Access to Port Control   512Byte

System.Mode Attach

Break

d.dump 0x05200000

// use only for debugging if stopped inside snippet setup_dpm
// Break.Set 0x4000008 /Onchip /PROGRAM /COUNT ?3?
// Data.LOAD.Elf ..\..\..\targets\netx4000_full\setup_dpm_netx4000_full_intram.elf /NoCODE

Mode

Break.Set 0x4000008 /Onchip /PROGRAM /COUNT 2

Break.Set 0x041120bc /Onchip /PROGRAM

// invalid INTRAM
d.Set 0x05100000++0xff %Long 0x1234ffff

// invalid DPM window message
d.Set 0x05200000++0xff %Long 0x8987affe

// set PC into endless loop
d.set 0x05100000++3 %Long 0xEAFFFFFE

R.S PC R:0x05100000
R.S CPSR 0x800001D8

// reset is only allowed, if CPU is running
go

// reset netX4000
SYStem.RESetOut

WAIT (!STATE.RUN()) 1s

IF STATE.RUN()
(
  DIALOG.OK "Failed to stop before start of setup_dpm"
  ENDDO
)
// first breakpoint is expected just before start of setup_dpm program
data.LOAD.Elf ..\..\..\targets\netx4000_full\setup_dpm_netx4000_full_intram.elf /DIFF
IF FOUND()
(
	DIALOG.OK "Error setup_dpm code NOT found"
	ENDDO
)

// 
go
WAIT (!STATE.RUN()) 1s


IF STATE.RUN()
(
  DIALOG.OK "Failed to stop at end of HWC"
  ENDDO
)
data.LOAD.binary ref_netx4000.bin 0x05200000++0x012 /DIFF
IF !FOUND()
(
	DIALOG.OK "Found Message for netx4000 SPM-Setup! TEST OK"
	ENDDO
)
ELSE
(

	DIALOG.OK "TEST FAILED! no correct message found for netx4000 SPM-Setup"
	ENDDO

)


ENDDO

