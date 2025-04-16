***** This is a Arduino code for esp32s3.*****
***** The code set up one nrf24 module to the SPI. The nrf24 use 110 channel and "ABCDE" address. The transmission over this nrf24 is a simple unsafe one (no ACK, no interrupt).*****
***** The matched nrf24 sender is sending a data frame which includes "5 bytes header + 27 bytes data". For each of the sensory data from each sensors, the nrf24 send data frame separately. 
      Data in one group means sensors get data under same physical timestamp.*****
