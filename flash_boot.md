# STM32N6 Flash Boot

This project is set up for STM32N6 `FSBL + Appli` load-and-run boot from the
board's external XSPI2 flash.

## Build outputs

- `FSBL/build/Debug/STM32N6_FSBL.bin`
- `Appli/build/Debug/STM32N6_Appli.bin`

## Flash layout

- FSBL image base: `0x70000000`
- Appli image base: `0x70100000`

The application remains linked for internal SRAM execution. At power-on, the
boot ROM loads the FSBL into SRAM, then the FSBL copies the application image
from XSPI2 flash into internal SRAM and jumps to it.

## Signing

STM32N6 boot ROM expects signed or trusted-header images. With the ST signing
tool, generate trusted binaries for both images before programming:

```powershell
STM32_SigningTool_CLI.exe -bin STM32N6_Appli.bin -nk -of 0x80000000 -t fsbl -o STM32N6_Appli-trusted.bin -hv 2.3 -align -dump STM32N6_Appli-trusted.bin
STM32_SigningTool_CLI.exe -bin STM32N6_FSBL.bin  -nk -of 0x80000000 -t fsbl -o STM32N6_FSBL-trusted.bin  -hv 2.3 -align -dump STM32N6_FSBL-trusted.bin
```

## Programming

Program the trusted binaries to external flash:

- `STM32N6_FSBL-trusted.bin` at `0x70000000`
- `STM32N6_Appli-trusted.bin` at `0x70100000`

Use the NUCLEO-N657X0-Q external loader / XSPI2 external flash flow in
STM32CubeProgrammer.

## Boot switches

For flash boot on NUCLEO-N657X0-Q:

- `BOOT0`: `1-2`
- `BOOT1`: `1-2`

Then press reset or power-cycle the board.

## Notes

- This repo currently keeps XSPI in a conservative 50 MHz mode and does not
  program the irreversible VDDIO3 high-speed OTP fuse.
- The FSBL copy window is configured to `0x80000` bytes, which matches the
  signed-header-plus-image span of the current 511 KB Appli linker region.
