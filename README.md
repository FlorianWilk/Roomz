# Roomz
IOT Tutorial - How to build a Room-Dashboard for Temperature and Humidity with
a Microsoft MXChip (Arduino), Kibana, Docker and some lines of NodeJS.

# Quickstart

## Find a PC

You will need some PC inside of your local network which will be used as Datastore and Cockpit. Since this PC should be running 24/7 something like a NUC or RaspberryPI would be a good choice. 
All this has been tested on a Intel-based mini PC similar to a NUC. If you try it on a RaspberryPI some feedback would be great. 

To make all the installation as easy as possible the whole backend-setup is bundled into Docker-Containers.

### New to Docker?

If you never worked with Docker before, no Problem at all:

Just download Docker for your OS:

https://docs.docker.com/install/

And then let's install Docker-Compose

https://docs.docker.com/compose/install/

On Linux it was pretty simple. If you use Windows, feedback would be great here again.
At this point you don't even need to understand what these Tools do. Just sit back and relax :)

### Install our Environment

Now lets clone this Repository here into some Folder on the PC.

If you use Linux:

make sure you have installed git
```
sudo apt-get install git
```

and thanks to Elastic-Search you will need to make some adjustments on your Host-Machine:
```
sudo sysctl -w vm.max_map_count=262144
sudo nano /etc/sysctl.conf
```
and add the following line:
```
vm.max_map_count=262144
```
Then press Ctrl-O to save, Ctrl-X to exit.

Then clone the Repository into some Folder:
```
cd ~
git clone https://github.com/FlorianWilk/Roomz.git
cd Roomz
```

Now we can start the whole Stack with
```
docker-compose up
```

You shouldn't see any Error-Messages. If you do, PM.

Now open a Browser and open the URL for this machine +":5601", so if it's your local machine:
```
http://localhost:5601/
```
or
```
http://<pc_where_you_installed_this_all>:5601/
```
I guess you got it. Now you should see a nice and clean Kibana-Dashboard. If you do: Formidable! If not: PM me.

Now let's prepare...

## The Sensor

First lets download the Arduino IDE for your OS:

https://www.arduino.cc/en/main/software

Install it, open it and the you will have to add some Libaries:

Tools -> Manage Libraries

< add which libs are needed. >

Now hit CTRL-O and open the folder "ArduinoCode" of this Repository which you have cloned before (for example "~/Roomz/ArduinoCode"). 

Now change the line 

```
const char* mqttServer = "hubert";
```
to match the Hostname or IP of the PC where you installed this Repository. In my case the Hostname is "hubert". But you can also use an IP-Address:
```
const char* mqttServer = "192.168.178.12";
```
Press CTRL-S to save.

Connect your MXChip via USB to the PC, wait some seconds.
Then press CTRL-U to upload the code to the MXChip.

You should see no errors here. Again, if you do: PM me.

## Tadaaa

Now you should see the first Data coming in. Yay.







