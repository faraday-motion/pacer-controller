
<p align="center">
  <img src="http://www.faradaymotion.com/wp-content/uploads/2017/07/pace_logo.png" width="150">
</p>

# Wireless Controller for Pacer Powered Vehicles

This is the firmware for controllers that are compatible with Pacer powered vehicles.

The controller works by performing a handshake with the Pacer powered vehicle and will then send inputs in for acceleration, brake or neutral signals.

# What is Pacer?
Pacer is a development platform that enables fast and cheap development of light electric vehicles.

Pacer bundles open source software with modular hardware components.

Pacer was created to simplify the development of light electric vehicles by providing out of the box trivial functionality like connectivity, controlling and state management of the vehicle. 

[Learn more about Pacer here.](https://github.com/faraday-motion/pacer)

## Software Getting Started

1. Set up a PlatformIO development environment for ESP8266. ([Check this guide](https://github.com/esp8266/Arduino#using-platformio))

2. Build and Upload `pio run --target upload` (run this command in the root of the project)
   Note: You might want to look through the config file in `data/` directory and change relevant configurations.

## Hardware Getting Started 

### nRF24 Connections

![nRF24](http://www.faradaymotion.com/wp-content/uploads/2017/07/Screen-Shot-2017-07-27-at-12.09.51-PM.png)

![nRF24 Pins](http://www.faradaymotion.com/wp-content/uploads/2017/07/Screen-Shot-2017-07-27-at-12.23.36-PM.png)

## Handshake procedure
Note: The commands and the handshake procedure is described in the Commands.h file.

In this setup a Pacer powered vehicles is refered to as Master and the controller is refered to as Slave.

1. Master requests name from slave on the defaule radio address and channel. 
2. Slave send the name to the Master. 
3. Master sends a newly generate address to the slave. 
4. Slave saves the received address and sends it back. 
5. Master validates that the new address was transmitted correctly and asks the Slave to proceed to changing the address. 
6. Both Master and Slave change addresses. 
7. On the new address Master is sending the clearest channel to Slave. 
8. Slave saves the new channel and sends it back. 
9. Master validates that the new channel was transmitted correctly and asks the Slave to proceed to changing the channel.
10. Both Master and Slave change channels. 
11. Master asks the Salve to wait until it tries to register the controller.
12. If registration was successful Master confirms successful registration and asks the Slave to go into IDLE_MODE
13. Slave goes to IDLE mode and waits to be activated. 
-- Handshake finished -- 
14. Master is moving the communication from the Radhiohanshake class to the NunchuckController classs. 
15. Master asks the Slave to set ACTIVE_MODE
16. Master asks requests inputs from Slave. 
17. Slave sends inputs to Slave.

# Community 

There is a [forum thread](http://forum.faradaymotion.com/d/19-what-is-pacer/2) dedicated to Pacer where you can get quick answers to your questions.

# Support Us

You can support the Pacer project by purchasing hardware required for running Pacer and other products from the [Faraday Motion Web Shop](http://shop.faradaymotion.com). We highly appreciate your support. 

# Contributing

There is a long road of development until we get Pacer to be the go to solution. Any contributions are highly valued. 

For minor fixes of code and documentation, go ahead and submit a pull request.

Feature branches with lots of small commits (especially titled "oops", "fix typo", "forgot to add file", etc.) should be squashed before opening a pull request. At the same time, please refrain from putting multiple unrelated changes into a single pull request.

# Authors 
Constantin Colac and Sune Pedersen
