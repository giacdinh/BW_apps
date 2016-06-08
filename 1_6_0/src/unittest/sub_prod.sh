if [ ! $1 ]; then
   echo please provide ID
   echo Usage: ./sub_prod.sh 1214100085
   exit 0
fi
echo "START SUB chain"
UUID_FILE=/odi/log/uuid.txt
ID_FILE=/odi/log/id.txt
TOKEN_FILE=/odi/log.token.txt
rm $UUID_FILE 
rm $ID_FILE 
rm $TOKEN_FILE 

DEV_ID=$1
tokens=`curl -H "Accept: application/json" \
-H "Authorization: Basic Qm9keVZpc2lvbmFwcDpteVNlY3JldE9BdXRoU2VjcmV0" \
-H "Device: BodyVision" -k -a --capath "/odi/conf/m2mqtt_ca_prod.crt" \
--data "username=$DEV_ID&password=$DEV_ID&grant_type=password&scope=read%20write" \
-s https://prod-app-lb.l3capture.com/api/oauth/token`

echo $tokens
access_token=`echo "$tokens" |cut -c 18-53`
refresh_token=`echo "$tokens" |cut -c 95-130`

#debug echo
echo "access token : $access_token"
echo "refresh token: $refresh_token"

#debug echo
echo "$DEV_ID" > $ID_FILE 
echo "$access_token " > $TOKEN_FILE 

echo "run Init SUB"
mosquitto_sub -v -q 1 -p 8080 --tls-version tlsv1 --cafile /odi/conf/m2mqtt_ca_prod.crt -u "$access_token" -i NEWDEV-$DEV_ID \
-t "/DEVICE/$DEV_ID" -h "prod-mqtt-lb.l3capture.com" > $UUID_FILE &

echo "Launched chain"
sleep 5 
killall mosquitto_sub

id=`cat id.txt`
access_token=`cat token.txt`
uuid=`tail -n 1 uuid.txt |cut -c 102-142`

echo "******************************************************************"
device="DEVICE-$id"
topic="/$uuid/$id"
echo "agency uuid 	: $uuid"
echo "token		: $access_token"
echo "id   		: $id" 
echo "device		: $device"
echo "topic		: $topic"


echo "RUN config SUB"
mosquitto_sub -v -q 1 -p 8080 --tls-version tlsv1 --cafile /odi/conf/m2mqtt_ca_prod.crt -u "$access_token" -i $device -t $topic -h prod-mqtt-lb.l3capture.com

