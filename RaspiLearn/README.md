--------README for wiringPi-------
g++ -Wall main.cpp -o main -lwiringPi
"-I/usr/local/include -L/usr/local/lib -lwiringPi"

or
g++ -pthread main.cpp -o main -lwiringPi

NOTE: To compile programs with wiringPi, you need to add:
    -lwiringPi
  to your compile line(s) To use the Gertboard, MaxDetect, etc.
  code (the devLib), you need to also add:
    -lwiringPiDev
  to your compile line(s).