# Nightclub-simulator
In the scope of the Operating System course we were proposed to implementation of a nightclub simulator. This implementation has to be done with the concepts taught in theoretical and practical classes, namely concurrency, synchronization and communication mechanisms existing in the C language.

# How to build
To build the project: make all
To delete unnecessary file do: make clean
The configuration data can be found in the file: simulator-conf.txt.
In this file you can not change the word order or the value of "FREQUENCY".

Then open two terminals, in the first do: ./monitor and select option 1 and after that do in the second terminal: ./simulator
On the monitor you can see the events such as queue entries, quits, zone changes, entering zones, and leaving the disco.
Clients of gender 1 are female clients.
After the creation of all customers OR the completion of the total time, the simulation ends and the report in "log.txt" is generated
