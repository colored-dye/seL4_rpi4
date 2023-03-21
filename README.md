# Port seL4 to LS1046A platform.

Forlinx OK1046A-C2 board with FET1046A-C core platform.

## How to ... ?

### How to get ported code?

First, run `./scripts/get-manifest.sh manifest.txt`. This will download original official repositories to `backup/` directory.

Next, run `./scripts/patch-projects.sh`. This will copy the backup projects to the disired location, and apply the patches from `diffs/` directory.

If the code fails to patch correctly, you can also modify the code manually.

### How to build user applications?

Run `./scripts/build-app.sh <app>`. Keep the `<app>` argument empty to see a list of all the available apps.

If you want to build the code for another platform, you can modify the `plat` variable in the script.

The script will automatically set up all the necessary arguments and possibly some unused arguments.

## DTS & DTB files

```
├── apps
│   ├── comm                -- Communication system.
│   ├── comm-smaccm         -- Follow SMACCM structure.
│   ├── empty               -- Empty user space program, ends with a stack dump.
│   ├── hello               -- Print "Hello, World", ends with a stack dump.
│   ├── hello-camkes-1      -- CAmkES components, Echo server and Client, says something, with safe exit.
│   └── hello-camkes-2      -- Client communicates with Echo server through Event and Dataport, ends with a write-permission violation.
├── backup
│   ├── kernel
│   ├── projects
│   ├── tools
│   └── USB
├── build
├── diffs                   -- Patches for the our changes to the original repos.
├── dts
│   ├── dtb
│   │   └── freescale       -- Compiled device tree. Copied from a compiled kernel(4.19.238).
│   ├── dtb2dts
│   │   └── freescale       -- DTS dumped from DTB, using `dtbdump.py` or `dtc`.
│   └── dts-linux
│        └── freescale      -- Device tree source.
│                              Copied from https://github.com/torvalds/linux/tree/v5.19/arch/arm64/boot/dts/freescale.
├── kernel
│   ├── configs
│   ├── include
│   ├── libsel4
│   ├── LICENSES
│   ├── manual
│   ├── src
│   └── tools
├── projects
│   ├── capdl
│   ├── global-components
│   ├── musllibc
│   ├── projects_libs
│   ├── seL4_libs
│   ├── seL4_projects_libs
│   ├── sel4runtime
│   ├── sel4-tutorials
│   └── util_libs
├── scripts
└── tools
    ├── camkes
    └── seL4
dts/
├── dtb
│   └── freescale           -- Compiled device tree. Copied from a compiled kernel(4.19.238).
├── dtb2dts
│   └── freescale           -- DTS dumped from DTB, using `dtbdump.py` or `dtc`.
└── dts-linux
    └── freescale           -- Device tree source.
                               Copied from https://github.com/torvalds/linux/tree/v5.19/arch/arm64/boot/dts/freescale.
```


## seL4 kernel (`kernel/`)

URL: https://github.com/seL4/seL4

Version: 12.1.0-dev

Branch: master

Commit: dd41d4de63949e689e54844089f851425e2ab748

### `tools/dts/ok1046a-c2.dts`

Copied from `dts/dtb2dts/freescale/fsl-ok1046a-1133-5a59-c2.dts`.

### `src/plat/ls1046a/`

1. `config.cmake`

    ```cmake
    declare_platform(ls1046a KernelPlatformOK1046AC2 PLAT_LS1046A KernelArchARM)

    declare_default_headers(
        TIMER_FREQUENCY xxx
        MAX_IRQ xxx
        NUM_PPI xxx
        TIMER xxx
        INTERRUPT_CONTROLLER xxx
        KERNEL_WCET xxx
        CLK_MAGIC xxx
        CLK_SHIFT xxx
    )
    ```

2. `overlay-ls1046a.dts`

    - Add offset and size for `/memory@80000000` node;
    - Add `/reserved-memory` nodes;
    - `/chosen` node:
        ```dts
        chosen {
            seL4,elfloader-devices =
                "serial0";

            seL4,kernel-devices =
                "serial0",
                &{/interrupt-controller@1400000},
                &{/timer};
        };
        ```

### libsel4

`libsel4/sel4_plat_include/ls1046a/sel4/plat/api/constants.h`

### Kernel drivers

1. GIC: GIC-400, compatible with GICv2.

2. Timer: `arm,armv8-timer`, compatible with `generic_timer`.

3. UART: `fsl,ns16550;ns16550a`. Manually implemented.


## seL4 tools

URL: https://github.com/seL4/seL4_tools

Branch: master

Commit: 074a54aedcef97bfcc4ea0724a4c5d75fa311b3e

### `cmake-tool`

1. `helpers/application_settings.cmake`:

    In `ApplyData61ElfLoaderSettings`, set ElfLoader image type to binary.

### `elfloader-tool`

1. drivers

    Add an ns16550a UART driver, modified from the kernel version, without `getchar()`.

2. Platform-specific init functions

    In `src/plat/ls1046a/platform_init.c`, add `platform_init()` and `non_boot_init` functions, similar to `rpi4`.
