# EfiLibRedfish

Copyright 2018 Distributed Management Task Force, Inc. All rights reserved.

## About
EFILibRedfish binding is EFI library based on libredfish. EFILibRedfish is built on UEFI EDK2 open source and linked with other EFI modules.

## Pre-requisists

 * EDK2 open source and EDK2 build environment are required - [https://github.com/tianocore/edk2/](https://github.com/tianocore/edk2/)
 * C library is necessary to pull into build process for DXE Driver/UEFI Driver/ UEFI Application modules. Adding the C library instance in [LibraryClass] section in EfiLibRedfish.inf

#### EfiLibRedfish INF Example

```C
[LibraryClasses]
  BaseLib
  DebugLib
  MemoryAllocationLib
  BaseMemoryLib
  JanssonLib
  TinyLibC		#Add your own C library instance.
```

 * JanssonLib used for EFI libredfish library.


## Source code and Header files
In order to keep the code architecture of libredfish and reduce the maintenance efforts, EFI libredfish doesn't 100% follow EDK2 C source/header files reference principles. Instead, the relative path is used to refer to C source files and header files as shown in below,
### libredfish source files reference and override
##### EfiLibRedfish INF
```C
[Sources]
  ../../src/payload.c
  ../../src/redpath.c
  service.c	# Override original libredfish service.c
```

### libredfish header files reference
EFI binding of libredfish maintains its own libredfish header files under /binding/uefi and refer to the original libredfish Header file implicitly in order to avoid EDK2 build violations. 
##### redfishPayload.h
```C
#include "..\..\include\redfishPayload.h"
```

For overriding libredfish header file for EFI binding, additional header file searching directory is added to the build option. Current source directory is added to build option to override redfishService.h because there is no culr support in EFI.
##### EfiLibRedfish INF
```C
[BuildOptions]
MSFT:*_*_X64_CC_FLAGS  /I$(WORKSPACE)\YourPackageName\Library\libredfish\bindings\uefi
```
Change "YourPackageName" to your EDK2 package

## curl is not supported in pre-OS EFI environment
EFI REST EX protocol is used to communicate with Redfish service instead. redfishService.h and service.c are overided under binding/uefi for this reason.

