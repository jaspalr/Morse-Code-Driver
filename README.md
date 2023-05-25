# Morse Code Driver
A Linux driver that translates a phrases inputted from the Linux terminal to Morse code and displays them via an external LED connected to a BeagleBone 

## To Run:
&nbsp; &nbsp; &nbsp; Type "Make" into the host's terminal \
&nbsp; &nbsp; &nbsp; On the BeagleBone type “cd /mnt/remote/myApps/” \
&nbsp; &nbsp; &nbsp; Then “sudo insmod morsecode.ko” \
&nbsp; &nbsp; &nbsp; Then “echo '\<insert phrase>\’ | sudo tee /dev/morse-code“
 
## To Remove:
&nbsp; &nbsp; &nbsp; Type "sudo rmmod testdriver"

## Video Demo 


https://user-images.githubusercontent.com/105681721/235572114-b0c29dfa-1079-4d35-a279-2b8730b4b563.mp4





