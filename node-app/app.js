const express = require('express')
const mqtt = require('mqtt')

const app = express()
const port = 3000

const protocol = 'mqtt'
const mqtt_host = 'ip_addr' // Modify with the Ip address of the MQTT broker
const mqtt_port = '1883'

const bodyParser = require('body-parser');
app.use(bodyParser.json());

const mqtt_connectUrl = `${protocol}://${mqtt_host}:${mqtt_port}`

const devices = {}

const locations = JSON.parse('[]')
locations.push('-')


const options = {
    retain: true 
  };

  console.log(__dirname)
  app.use(express.static(__dirname + "/static"));


app.get('/', (res) => {
  
    // Affiche le fichier index.html
    res.sendFile(`${__dirname}/static/index.html`);
})


// Connect to MQTT broker when server is ready, subscribe to topics and listen to messages
app.listen(port, () => {
    console.log(`Le serveur Express écoute sur le port ${port}`);
  
    // Connect to MQTT broker
    const client = mqtt.connect(mqtt_connectUrl);
  
    // Event when connected to MQTT broker
    client.on('connect', () => {
      console.log('Connecté au broker MQTT');
  
      const topics = [
        'commander/devices/+',
        'arduino',
      ];

      // Subscribe to topics
      client.subscribe(topics, options, (err, granted) => {
        if (err) {
          console.error('Erreur lors de la souscription :', err);
        } else {
          console.log('Souscription réussie aux topics :');
          granted.forEach(({ topic }) => {
            console.log(topic);
          });
        }
      });
    });

    client.on('message', (topic, message) => {

        if (topic.startsWith('commander/devices/')) {
        
        const deviceId = topic.split('/')[2];  
      
        const device = JSON.parse(message.toString());

        devices[deviceId] = device

        const location = device.deviceLocation;
        if (!locations.includes(location)){
          locations.push(location);
        }
        }

        if (topic.startsWith('arduino')) {

        }
    });
});

app.get('/data', (res) => {
    res.send([devices, locations])
})

// Influx Route to fetch data from influxDB
app.post('/influx', (req, res) => {

  
  const deviceLocation = req.body.deviceLocation;
  const deviceUID = req.body.deviceUID;
  const sensor = req.body.sensor;

  let org = `iot_project`
  let bucket = `iot_bucket`

const {InfluxDB} = require('@influxdata/influxdb-client')

// Not great but it's a POC
const token = 'FqAcY3TmDmEl6xfidOWN9eEoh9NTKzRc0bJoLf1srfZaadVQIfqJ5vFymaLK1tpWlF-USUd9M32e3GPyOkYW9A=='
const url = 'http://<Ip_Addr>8086' // Modify with the Ip address of the InfluxDB server

const client = new InfluxDB({url, token})

let queryClient = client.getQueryApi(org)

// Flux query to fetch data from InfluxDB
let fluxQuery = `from(bucket: "${bucket}")
|> range(start: -2d)
|> filter(fn: (r) => r["_measurement"] == "${sensor}")
|> filter(fn: (r) => r["_field"] == "value")
|> filter(fn: (r) => r["location"] == "${deviceLocation}")
|> filter(fn: (r) => r["device"] == "${deviceUID}")`


var tableObject = {}
var objects = []

// Query data from InfluxDB
queryClient.queryRows(fluxQuery, {
  next: (row, tableMeta) => {
    tableObject = tableMeta.toObject(row)
    objects.push(tableObject)
    
  },  
  error: (error) => {
    console.error('\nError', error)
  },
  complete: () => {
    res.send(objects)
  },
})
})
