# RaspberryLatte
## Purpose
This software, written in c++, allows users to control espresso machines with a raspberry pi. The project is in its early stages and is limited to PID temperature control and UI setup.

## Third Party Requirments
The software requires an install of the pigpio library. Download and installation instructions can be found [here](http://abyz.me.uk/rpi/pigpio/download.html). It also requires ncurses for the time being to create the command line interface.


## To Do
The software is still in its early stages and there are many helpfull features that need to be added. They include, in no particular order
- Refactor the code to make it more flexable for specific applications.
- Add logging to allow the user to track the settings, etc and how the resulting cup of espresso turned out.
- Add scheduling.
- Create mobile apps that can link to the pi and provide a nice UI.
- Add the ability to control the pump.
- Create teh hardware and software needed to interface with an output scale.
- Explore pressure/flowrate sensing and control.

Further brainstorming is needed to build a dev plan but these points should serve as a starting point.
