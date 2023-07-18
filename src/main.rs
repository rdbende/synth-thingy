use std::thread;
use std::time::Duration;

use rppal::gpio::*;

const REGISTER_SELECT: u8 = 17;
const ENABLE: u8 = 27;
const DATA_4: u8 = 5;
const DATA_5: u8 = 6;
const DATA_6: u8 = 16;
const DATA_7: u8 = 26;

struct LCD {
    register_pin: OutputPin,
    enable_pin: OutputPin,
    data_4: OutputPin,
    data_5: OutputPin,
    data_6: OutputPin,
    data_7: OutputPin,
}

impl LCD {
    fn new() -> Result<LCD> {
        let gpio = Gpio::new()?;
        let register_pin = gpio.get(REGISTER_SELECT)?.into_output();
        let enable_pin = gpio.get(ENABLE)?.into_output();
        let data_4 = gpio.get(DATA_4)?.into_output();
        let data_5 = gpio.get(DATA_5)?.into_output();
        let data_6 = gpio.get(DATA_6)?.into_output();
        let data_7 = gpio.get(DATA_7)?.into_output();
        Ok(LCD {
            register_pin,
            enable_pin,
            data_4,
            data_5,
            data_6,
            data_7,
        })
    }

    fn initialize_display(&mut self) {
        self.enable_pin.set_low();
        self.register_pin.set_low();

        self.send_4_bits(0, 0b0011);
        thread::sleep(Duration::from_micros(5000));
        self.send_4_bits(0, 0b0011);
        thread::sleep(Duration::from_micros(100));
        self.send_4_bits(0, 0b0011);
        self.send_4_bits(0, 0b0010);
        self.send_8_bits(0, 0b00101000);
        self.send_8_bits(0, 0b00001100);
        self.send_8_bits(0, 0b00000110);
        self.send_8_bits(0, 0b00000001);
        thread::sleep(Duration::from_micros(2000));
    }

    fn pulse_enable(&mut self) {
        self.enable_pin.set_high();
        thread::sleep(Duration::from_micros(10));
        self.enable_pin.set_low();
    }

    fn send_4_bits(&mut self, reg: u8, bits: u8) {
        println!("{bits}");
        self.register_pin.write(Level::from(reg));

        self.data_4.write(Level::from((bits & 0b00000001) >> 0));
        self.data_5.write(Level::from((bits & 0b00000010) >> 1));
        self.data_6.write(Level::from((bits & 0b00000100) >> 2));
        self.data_7.write(Level::from((bits & 0b00001000) >> 3));

        self.pulse_enable();
    }

    fn send_8_bits(&mut self, reg: u8, bits: u8) {
        self.register_pin.write(Level::from(reg));

        self.data_4.write(Level::from((bits & 0b00010000) >> 4));
        self.data_5.write(Level::from((bits & 0b00100000) >> 5));
        self.data_6.write(Level::from((bits & 0b01000000) >> 6));
        self.data_7.write(Level::from((bits & 0b10000000) >> 7));

        self.pulse_enable();

        self.data_4.write(Level::from((bits & 0b00000001) >> 0));
        self.data_5.write(Level::from((bits & 0b00000010) >> 1));
        self.data_6.write(Level::from((bits & 0b00000100) >> 2));
        self.data_7.write(Level::from((bits & 0b00001000) >> 3));

        self.pulse_enable();
    }
}

fn main() -> Result<()> {
    let mut lcd = LCD::new()?;
    lcd.initialize_display();
    Ok(())
}
