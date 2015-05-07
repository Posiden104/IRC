#/bin/bash

if [ $# -ne 2 ]
then
  echo "Usage: `basename $0` server-prog port"
  exit 1
fi

SERVER=$1
PORT=$2

echo "Start Test"

#echo Killing Server if any
PID=`ps | grep IRCServer | awk '{ print $1;}'`
kill -9 $PID 2> /dev/null
sleep 2

#Start server in the background
#rm -f password.txt
#$SERVER $PORT > talk-server.out &
#sleep 1

echo Add Users
./TestIRCServer localhost $PORT "ADD-USER user1 123"
./TestIRCServer localhost $PORT "ADD-USER user2 123"
./TestIRCServer localhost $PORT "ADD-USER user3 123"
./TestIRCServer localhost $PORT "ADD-USER mary poppins"


echo Print Users
./TestIRCServer localhost $PORT "GET-ALL-USERS user1 123"

echo Create Room
./TestIRCServer localhost $PORT "CREATE-ROOM user1 123 room1"
./TestIRCServer localhost $PORT "CREATE-ROOM user1 123 room2"

echo List Rooms
./TestIRCServer localhost $PORT "LIST-ROOMS user1 123"

echo Enter Room
./TestIRCServer localhost $PORT "ENTER-ROOM user1 123 room1"
./TestIRCServer localhost $PORT "ENTER-ROOM user2 123 room1"
./TestIRCServer localhost $PORT "ENTER-ROOM user3 123 room1"
./TestIRCServer localhost $PORT "ENTER-ROOM user1 123 room2"
./TestIRCServer localhsot $PORT "ENTER-ROOM mary poppins room2"
./TestIRCServer localhost $PORT "ENTER-ROOM ADMIN 123 room3"
./TestIRCServer localhsot $PORT "ENTER-ROOM mary poppins room3"



# Kill server
echo Killing Server
PID=`ps | grep IRCServer | awk '{ print $1;}'`
kill -9 $PID

