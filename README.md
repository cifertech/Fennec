<div align="center">
  
<img src="https://github.com/user-attachments/assets/a5734370-30ae-4775-8335-b919a8742d76" alt="Fennec Banner" width="100%"/>

  <br/>
  <br/>
  
  <p align="center">
    <a href="https://github.com/cifertech/Fennec"><img src="https://img.shields.io/static/v1?label=cifertech&message=Fennec&color=c9a468&logo=github"/></a>
    <a href="https://github.com/cifertech/Fennec"><img src="https://img.shields.io/github/stars/cifertech/Fennec?style=social"/></a>
    <a href="https://github.com/cifertech/Fennec"><img src="https://img.shields.io/github/forks/cifertech/Fennec?style=social"/></a>
    <img src="https://img.shields.io/badge/ESP32--S3-SI4732%20Receiver-c9a468"/>
    <img src="https://img.shields.io/badge/license-MIT-c9a468"/>
  </p>

  <p align="center">
    <a href="https://twitter.com/techcifer"><img src="https://img.shields.io/badge/Twitter-c9a468?logo=x&logoColor=black"/></a>
    <a href="https://www.instagram.com/cifertech/"><img src="https://img.shields.io/badge/Instagram-c9a468?logo=instagram&logoColor=black"/></a>
    <a href="https://www.youtube.com/@techcifer"><img src="https://img.shields.io/badge/YouTube-c9a468?logo=youtube&logoColor=black"/></a>
    <a href="https://cifertech.net/"><img src="https://img.shields.io/badge/Website-c9a468?logo=googlechrome&logoColor=black"/></a>
  </p>

</div>

<div>&nbsp;</div>

## 📖 Explore the Fennec Wiki

Hardware, flash steps, every face, the web API, serial commands, and the "it doesn't work" list live in the [Wiki](https://cifertech.github.io/Fennec/). Same pages are in [`docs/index.html`](https://github.com/cifertech/Fennec/blob/main/docs/index.html) if you are reading this from the repo.

Copy [`docs/TFT_eSPI_User_Setup.h`](https://github.com/cifertech/Fennec/blob/main/docs/TFT_eSPI_User_Setup.h) over `Arduino/libraries/TFT_eSPI/User_Setup.h` before you upload.

Firmware, [PCB](https://github.com/cifertech/Fennec/tree/main/PCB), [schematic](https://github.com/cifertech/Fennec/tree/main/Schematic), and a [pre-compiled bin](https://github.com/cifertech/Fennec/tree/main/Pre-compiled%20Bin) are in the [GitHub repo](https://github.com/cifertech/Fennec).

<div>&nbsp;</div>

<!-- About the Project -->
## :star2: About the Project

Fennec is a handheld **shortwave listening station** on the **ESP32-S3** and the **SI4732-A10**. Receive only. No transmitter, no CAT, no station-memory list.

Tune AM, FM, and HF from a 2.8" touchscreen. Decode SSTV and Morse. Sweep an antenna into a peak. Log when a carrier comes up. Drive the same radio from a phone over Wi-Fi (SSID `Fennec`, `http://192.168.4.1/`).

Same family of hardware as [ESP32-DIV](https://github.com/cifertech/ESP32-DIV). Different job. This one *listens*.

> [!NOTE]
> Analog coverage is **FM 64-108 MHz** and **AM/SW 153 kHz-27.90 MHz**. Airband, NOAA, VHF/UHF, and 10 m ham (28 MHz+) are out of range. That's the chip, not a firmware bug.

<div>&nbsp;</div>

## :dart: Features

- **Radio** - FM / AM / SW through one SI4732, seek, RDS, last frequency in flash
- **SSTV** - Martin, Scottie, Robot 36 from the radio tap or the I2S mic
- **Morse** - Goertzel copy, pitch, USB/LSB, BFO
- **Homing** - RSSI plot, not a compass. You walk the antenna
- **Watch** - ARM a channel, log when the carrier holds (32 events)
- **Web** - phone and LCD stay in sync

Stay on **Arduino-ESP32 2.0.10**. Core 3.x breaks the NeoPixel RMT code. Full flash guide is in the [Wiki](https://cifertech.github.io/Fennec/#flash).

<table>
  <tr>
    <td style="text-align: center;">
      <img src="https://github.com/user-attachments/assets/70a29449-118c-4b57-82da-213496307c94" alt="ESP32-DIV Beta" style="width: 600px; border: 1px solid #ccc; border-radius: 5px;">
      <p style="font-style: italic; font-size: 14px; margin-top: 5px;">Fennec</p>
    </td>    
    <td style="text-align: center;">
      <img src="https://github.com/user-attachments/assets/0e17a610-2853-496e-8fbc-9f6c246f3f75" alt="ESP32-DIV v1" style="width: 600px; border: 1px solid #ccc; border-radius: 5px;">
      <p style="font-style: italic; font-size: 14px; margin-top: 5px;">Also Fennec</p>
    </td>
  </tr>
</table>

<div>&nbsp;</div>


## 💬 Support & Contributions

- 💬 Found a bug or have a feature request? Open an [Issue](https://github.com/cifertech/Fennec/issues)
- ⭐ Like the project? Star the repo!
- 🛠 Want to contribute? Fork it and submit a pull request.

## :warning: License

MIT © CiferTech 2026. See [LICENSE](https://github.com/cifertech/Fennec/blob/main/LICENSE).


## :handshake: Contact

▶ Support me on Patreon [patreon.com/cifertech](https://www.patreon.com/cifertech)

CiferTech - [@twitter](https://twitter.com/techcifer) - CiferTech@gmali.com

Project Link: [https://github.com/cifertech/Fennec](https://github.com/cifertech/Fennec)



## :gem: Acknowledgements

- [PU2CLR SI4735](https://github.com/pu2clr/SI4735) (Ricardo Caratti)
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) (Bodmer)
- [XPT2046_Touchscreen](https://github.com/PaulStoffregen/XPT2046_Touchscreen) (Paul Stoffregen)
- [ESP32-DIV](https://github.com/cifertech/ESP32-DIV) - same ESP32-S3 / ILI9341 / XPT2046 bus layout
