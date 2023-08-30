# Parallel and Sequential Screensaver

### Members:
- [Jun Woo Lee](https://github.com/jwlh00)
- [Roberto Rios](https://github.com/robertriosm)

### Project Description:
This project focuses on the design and development of a unique screensaver application. Initially, the screensaver is implemented using a sequential algorithm. Later, to improve performance and increase efficiency, a parallel version of the same screensaver will be used.

### Dependencies:
To run the code, you'll need to install a few dependencies. Here's what you'll need:
'''
sudo apt install libsdl2-2.0-0
sudo apt install libsdl2-dev
'''

### Compile:
To compile and run the code you will need to do the following:
'''
g++ main.cpp -o <executable_name> -lSDL2 -fopenmp
./<executable_name> <number_of_circles_to_generate>
'''
