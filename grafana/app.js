var mqtt = require('mqtt')
var Influx = require('influx')
const http = require('http')
const os = require('os')

const influx = new Influx.InfluxDB({
    host: 'influxdb',
    database: 'roomz',
    schema: [
      {
        measurement: 'measurements',
        tags: [
            'room'
          ],
        fields: {
          temp: Influx.FieldType.FLOAT,
          hum: Influx.FieldType.FLOAT,
          pressure: Influx.FieldType.FLOAT,
          time: Influx.FieldType.STRING
        },
        tags: [
          'room'
        ]
      }
    ]
  })

function runme() {

  influx.getDatabaseNames()
  .then(names => {
    if (!names.includes('roomz')) {
      return influx.createDatabase('roomz');
    }
  })
  .then(() => {
      subscribe()
  })
  .catch(err => {
    console.error(`Error creating Influx database!`,err);
    console.log("Retrying in 2seks")
    setTimeout(runme,2000)
  })
}

runme()
var client  = mqtt.connect('mqtt://mosquitto')
client.on('connect', function () {
  client.subscribe('room-status', function (err) {
    if (!err) {
        console.log("Connected to MQTT. Listening...")
    }
  })
})

var eclient;
var indexname="roomstatus"

function subscribe() {
    client.on('message', function (topic, message) {
        // message is Buffer
        try {
          var dat = JSON.parse(message.toString())
          dat.time=Date.now()
          console.log(dat)
          influx.writePoints([   
            {     
                 measurement: 'measurements',     
                 tags: { room: dat.room},
                 fields: {  temp: dat.temp, hum: dat.hum, pressure: dat.pressure, time: dat.time}   
            } ])
            .then(() => {   
            console.log("Saved")
                 
            });
      } catch(e){
          console.log("Error while converting MQTT Message to Influxdb"+e);
      }
      
      })
}

