>**A final project of Linux Device Driver course**
(Snake game - Using Device Tree and SPI protocol)

##Usage:
- Make sure you **modify device tree source code first**
- _make_: build a linux kernel module
- _make clean_: remove a linux kernel module
- Copy module into Beaglebone Black
- _sudo insmod module_: start game. Using button to direct the snake.

##Note:
- My project build in Linux kernel Main-line, am33x-v5.4 (Longterm 5.4.x) 
- Using Beaglebone Black REV C3

##Device tree: 
&am33xx_pinmux {
	snake_game: snake_game {
		pinctrl-single,pins = <
			AM33XX_PADCONF(AM335X_PIN_MCASP0_AHCLKR, PIN_OUTPUT_PULLUP, MUX_MODE3)			/* spi1_cs			P9_28 */
			AM33XX_PADCONF(AM335X_PIN_MCASP0_FSX, PIN_INPUT, MUX_MODE3)						/* spi1_d0 (miso) 	P9_29 */
			AM33XX_PADCONF(AM335X_PIN_MCASP0_AXR0, PIN_INPUT, MUX_MODE3)					/* spi1_d1 (mosi) 	P9_30 */
			AM33XX_PADCONF(AM335X_PIN_MCASP0_ACLKX, PIN_INPUT, MUX_MODE3)					/* spi1_sclk 		P9_31 */

			AM33XX_PADCONF(AM335X_PIN_MCASP0_AHCLKX, PIN_OUTPUT_PULLUP, MUX_MODE7)			/* gpio3[21] res 	P9_25 */
			AM33XX_PADCONF(AM335X_PIN_MCASP0_FSR, PIN_OUTPUT_PULLUP, MUX_MODE7)				/* gpio3[19] dc 	P9_27 */
			AM33XX_PADCONF(AM335X_PIN_UART1_TXD, PIN_OUTPUT_PULLUP, MUX_MODE7)				/* gpio0[15] led 	P9_24 */

			AM33XX_PADCONF(AM335X_PIN_GPMC_AD10, PIN_INPUT_PULLUP, MUX_MODE7)			/* gpio0_26		P8_14 leftButton */
			AM33XX_PADCONF(AM335X_PIN_GPMC_AD15, PIN_INPUT_PULLUP, MUX_MODE7)			/* gpio1_15		P8_15 rightButton */
			AM33XX_PADCONF(AM335X_PIN_GPMC_AD14, PIN_INPUT_PULLUP, MUX_MODE7)			/* gpio1_14		P8_16 upButton */
			AM33XX_PADCONF(AM335X_PIN_GPMC_AD11, PIN_INPUT_PULLUP, MUX_MODE7)			/* gpio0_27		P8_17 downButton */
			AM33XX_PADCONF(AM335X_PIN_GPMC_CLK, PIN_INPUT_PULLUP, MUX_MODE7)			/* gpio2_1		P8_18 resetButton */

		>;
	};
};
&spi1 {
	status = "okay";

	nokia5110: nokia-5110@0 {
		compatible = "nokia, snake game";
		reg = <0>;
		pinctrl-names = "default";
		pinctrl-0 = <&snake_game>;

		spi-max-frequency = <4000000>;
		led-gpios = <&gpio0 15 GPIO_ACTIVE_HIGH>;
		reset-gpios = <&gpio3 21 GPIO_ACTIVE_HIGH>;
		dc-gpios =<&gpio3 19 GPIO_ACTIVE_HIGH>;

		leftButton-gpios = <&gpio0 26 GPIO_ACTIVE_LOW>;
		rightButton-gpios = <&gpio1 15 GPIO_ACTIVE_LOW>;
		upButton-gpios = <&gpio1 14 GPIO_ACTIVE_LOW>;
		downButton-gpios = <&gpio0 27 GPIO_ACTIVE_LOW>;
		resetButton-gpios = <&gpio2 1 GPIO_ACTIVE_LOW>;

		status = "okay";
	};
};