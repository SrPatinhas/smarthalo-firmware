
# Differences between original (Nordic issue) and SmartHalo


## SDK 14.2.0

No changes.


## SDK 12.1.0

* Some random project files have been modified/removed. Does not appear to affect functionality.
* Micro-ecc is prebuilt in the SmartHalo version
* UART FIFO values can be accessed in the SmartHalo version. This doesn't appear to be used anywhere:

```
+++ /Users/charlie/git/smarthalo-firmware/smart_halo-nrf5_sdk-e414c91a3966/nRF5_SDK_12.1.0_0d23e2a/components/libraries/uart/app_uart_fifo.c	2023-09-02 04:21:04
@@ -168,6 +168,15 @@
     return NRF_SUCCESS;
 }

+uint32_t app_uart_get_nb_tx_fifo_chars()
+{
+	return FIFO_LENGTH(m_tx_fifo);
+}
+
+uint32_t app_uart_get_nb_rx_fifo_chars()
+{
+	return FIFO_LENGTH(m_rx_fifo);
+}

 uint32_t app_uart_get(uint8_t * p_byte)
 {
 ```

We can probably run from a vanilla copy of the SDK, we just need to build micro-ecc.


## SDK 11

Several changes:

* Pin number definitions in `/examples/bsp/pca10040.h`
* Toolchain revision changed in `components/toolchain/gcc/Makefile.windows` and `components/toolchain/gcc/Makefile.posix` (q1 vs q3)
* Lots of bootloader / UART app-related changes.

Only used in `smart_halo-firmware.../Code/Firmware/legacy` - not ever shipped to customers?