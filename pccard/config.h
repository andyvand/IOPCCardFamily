/*
  Automatically generated by 'make config' -- don't edit!

  Actually, for Mac OS X this was hand edited :-)

*/
#ifndef _PCMCIA_CONFIG_H
#define _PCMCIA_CONFIG_H

#define AUTOCONF_INCLUDED
#define __IN_PCMCIA_PACKAGE__

#define __MACOSX__ 1
#undef	LINUX				//macosx
#undef	PREFIX
#define CC "gcc"
#define LD "ld"
#define KFLAGS ""
#define UFLAGS ""
#define PCDEBUG ""
#undef	USE_PM
#undef 	UNSAFE_TOOLS
#define	CONFIG_CARDBUS 1
#define CONFIG_PNP_BIOS 1
#undef	MODDIR
#undef	HAVE_MEMRESERVE //1

/* Options from current kernel */
#undef	CONFIG_MODULES
#define	CONFIG_SMP 1
#define CONFIG_PCI 1
#define CONFIG_PCI_QUIRKS 1
#undef	CONFIG_P
#define CONFIG_SCSI 1
#undef  CONFIG_IEEE1394
#define CONFIG_INET 1
#define CONFIG_NET_PCMCIA_RADIO 1
#define CONFIG_TR 1
#undef  CONFIG_NET_FASTROUTE
#undef  CONFIG_NET_DIVERT
#define CONFIG_MODVERSIONS 1
#define CONFIG_X86_L1_CACHE_BYTES 32
#define CONFIG_X86_L1_CACHE_SHIFT 5
#define CONFIG_PROC_FS 1
#define CONFIG_1GB 1
#undef  CONFIG_2GB
#undef  CONFIG_3GB
#define ARCH "ppc"
#define HOST_ARCH "ppc"
#define AFLAGS ""
#undef	CONFIG_ISA			//macosx

#undef	HAS_PROC_BUS			//macosx
#undef	SYSV_INIT
#undef  HAS_FORMS

#endif /* _PCMCIA_CONFIG_H */
