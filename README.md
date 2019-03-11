# MozzyJet

MozzyJet is an unmanned aerial vehicle to perform targeted, 
effective, and low-cost mosquito control. The vehicle is designed to apply an 
environmentally friendly biopesticide, known as the [Bti](https://www.epa.gov/mosquitocontrol/bti-mosquito-control), by injecting 
precisely the required doses at the critical locations and time periods 
to maximise effectiveness and minimise collateral damage to the environment.

This is a collection of the code for the firmware and the serial interface
for operating the pressurised sprayer module of MozzyJet. The code is largely
a personal collection of the author of code available from the developer community.
Adaptations have been made for the smooth operation of this specific application, 
and links to external resources have been provided where applicable.

Home page: [mozzyjet.com](http://mozzyjet.com/)

## Configuration

Pin configuration for [MSP430G2553](http://www.ti.com/product/MSP430G2553)

```bash
          |  VCC [      ] GND  |
Servo pos |  1.0 [      ] XIN  |
UART_RXD  |  1.1 [      ] XOUT |
UART_TXD  |  1.2 [      ] TEST |
          |  1.3 [      ] RST  |
MS58_SCE  |  1.4 [      ] 1.7  | SSI_SIMO
SSI_SCLK  |  1.5 [      ] 1.6  | SSI_SOMI
PWM recv  |  2.0 [      ] 2.5  |
PWM send  |  2.1 [      ] 2.4  |
          |  2.2 [      ] 2.3  | PWM recv
```

## Authors

All firmware and software have been develop at [The Cyprus Institute](https://www.cyi.ac.cy)
by [Kamil Erguler](mailto:k.erguler@cyi.ac.cy).

The code is heavily influenced by the great work of the extensive community 
of developers of the MSP430 and Arduino microchips. Links to resources are 
given where applicable.

## License

Code released under the [GNU GPL v3 license](LICENSE).
