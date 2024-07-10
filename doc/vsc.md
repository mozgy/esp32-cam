# Visual Studio Code with PlatformIO IDE

[Visual Studio Code](https://code.visualstudio.com/)

  1. Download and install the **Latest Version**
  1. Install PlatformIO IDE extension
      1. Click **View**->**Extensions**
      1. In search bar type Platformio
      1. Click on 'Install' button

  1. Click **File**->**Open Folder**
      - choose folder where you unpacked source from github zip
      - wait for VSC doing some install magic and probably few GUI restarts

  1. Check the defines
      - check the file <a href=../include/credentials_sample.h>Credentials Sample</a>, fill it with actual credentials and rename the file to credentials.h.
      - check the file <a href=../include/variables.h>Config Definitions</a> and change appropriate #defines

  1. Compile the sources
      - click 'PlatformIO' icon on the left
      - click **PROJECT TASKS**->**esp32cam**->**General**->**Build** or PlatformIO: Build icon on lower bar
      - if no errors proceed on next step otherwise fix the mayhem and repeat this step ..

  1. Check PlatformIO setup
      - check file <a href=../platformio.ini>PlatformIO ini</a> for upload/flashing details, first time should be wire connection -
```
; upload_port = 192.168.x.y
; upload_protocol = espota
upload_protocol = esptool
```
after succesfull first flash OTA is also possible, 'Serial Monitor' option can be used to find out DHCP assigned IP -
```
upload_port = 192.168.x.y
upload_protocol = espota
; upload_protocol = esptool
```

  7. Upload the firmware and HTML data
      - click 'PlatformIO' icon on the left
      - click **PROJECT TASKS**->**esp32cam**->**General**->**Upload** or PlatformIO: Upload icon on lower bar
      - click **PROJECT TASKS**->**esp32cam**->**Platform**->**Upload Filesystem Image**

