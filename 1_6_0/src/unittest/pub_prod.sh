
UUID_FILE=/odi/log/uuid.txt
ID_FILE=/odi/log/id.txt
TOKEN_FILE=/odi/log.token.txt


id=`cat $ID_FILE`
access_token=`cat $TOKEN_FILE`
uuid=`tail -n 1 UUID_FILE |cut -c 102-142`

echo "uuid : $uuid"
echo "token: $access_token"
echo "id   : $id"

for((i=0;i<10000;i++))
do
mosquitto_pub -d -p 8080 --tls-version tlsv1 --cafile /odi/conf/m2mqtt_ca_prod.crt -t /$uuid/dashboard -i device-$id -u $access_token -q 0 -m '{"serialNumber": '$id', "name":"script", "firmware": "v1", "officer_name":"Lance II", "officer_uuid":"00001", "upload": 0.3, "charge": 0.5, "disk":0.9, "ip": "192.168.2.2", "usb_login": true}' -h "prod-mqtt-lb.l3capture.com"

sleep 10 
echo "publishing loop count: $i"
done
