# RFID attendance log using ESP8266
This project aims to implement a comprehensive system that monitors bus usage by college students and faculty, replaces manual attendance registers in labs and libraries, and restricts unauthorized access to sensitive locations such as server rooms. The system utilizes unique RFID cards assigned to each student and faculty member, which contain essential details such as their name, register number, and a unique identifier (UID). Students and faculty scan their cards at designated RFID scanners to log their attendance or gain authorized access to restricted areas.
## functioning
When the students or faculties scan their RFID ID cards at the scanner the details are stored in the memory card.
To access the stored information, you have to connect to the WIFI network of the RFID module and you can use any web browser and type in the IP address of the devise which will lead you to a webpage where you have to enter the user id and password to gain access, then you can enter the month which will download the data as a CSV file to your device. 
To write new RFID cards you have to place the RFID card on the scanner and use [this](/rfid_write_personal_data/) code to write the required information to the card

### components used
* ESP 8266 12 E microcontroller
* DS1307 I2C real-time clock chip (RTC)
* RC522 RFID module
* Micro SD Card Reader Module
* Buzzer
* Resistors
### Other meterials
