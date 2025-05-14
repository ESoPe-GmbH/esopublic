# Adding a new mcu

1. Create a new subdirectory in source/mcu with the name of the manufacturer like renesas or nordic.
2. Create a subdirectory for the mcu inside the new directory.
3. Copy the content of template_mcu inside the new directory.
4. Add a new mcu type and a path to the mcu_controller.h into source/mcu/mcu_config.h
5. Set this new mcu type info the mcu_controller.h and replace all `MCU_NEW_NAME` in the directory of the new mcu with the new mcu type.
6. Modify mcu_internal and extend the structures as needed, following the TODOs.
7. Open the source/mcu/sys.c and add modifications (`system_get_tick_count`) for the new mcu there, guarding it with `#if MCU_TYPE == MCU_NEW_NAME`, where `MCU_NEW_NAME` is your new mcu type.

## Implementation order

1. Start by implementing mcu_controller.c, since this is essential.
2. If needed, add the timer functionality, since most stuff needs a stable millisecond timer for timing references.
3. Modify source/mcu/sys.c, so the `system_get_tick_count` function works.
4. Make sure to have a board.c that correctly calls `mcu_init`.
5. Add an LED that should toggle for testing your timer, by using the source/module/led.h module and using `led_init` and `led_set_blinking_ms`.
6. Make sure that this works.
7. Implement mcu_uart.c of your new mcu, so you can use debug prints.
8. Make sure the debug prints work.
9. With debug prints at help you can implement the rest of the mcu peripherals. A logic analyzer is needed to ensure pwms, spi, i2c, etc. work correctly!

Using JTAG helps a lot, since you can review the registers. But keep in mind when stopping with a JTAG debugger, you might get errors in peripherals because interrupts and received data might get lost.

## Other stuff to do with new mcu

- Modify the esope flasher to support new mcu in eso files.
- Modify the eso detail viewer, so you can see the new mcu.
- Modify the esoprog to support the new mcu.
