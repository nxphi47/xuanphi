--------README for wiringPi-------
g++ -Wall main.cpp -o main -lwiringPi
"-I/usr/local/include -L/usr/local/lib -lwiringPi"
g++ -pthread main.cpp -o main -lwiringPi

CMAKE: cmake .. --> make