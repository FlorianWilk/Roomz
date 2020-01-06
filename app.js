var mqtt = require('mqtt')
var elasticsearch = require('elasticsearch');

var client  = mqtt.connect('mqtt://localhost')
var eclient = new elasticsearch.Client({
    host: 'localhost:9200',
 //   log: 'trace',
    apiVersion: '7.2', // use the same version of your Elasticsearch instance
  });

client.on('connect', function () {
  client.subscribe('room-status', function (err) {
    if (!err) {
      client.publish('room-status', 'Hello mqtt')
    }
  })
})
 
var indexname="roomstatus"

function subscribe() {
    client.on('message', function (topic, message) {
        // message is Buffer
        try {
          var dat = JSON.parse(message.toString())
          dat.time=Date.now()
          console.log(dat)
      
          eclient.index({  
              index: indexname,
              type: '_doc',
              body: dat
          },function(err,resp,status) {
              console.log(resp);
          });
      } catch(e){
          console.log(e,message.toString());
      }
      
      //  client.end()
      })
}


eclient.indices.exists({index: indexname}, (err, res, status) => {
    if (res) {
        console.log('index already exists');
        subscribe()
    } else {
        eclient.indices.create( {index: indexname}, (err, res, status) => {
            putMapping()
        console.log(err, res, status);
    })
  }
})

async function putMapping () {
    console.log("Creating Mapping index");
    eclient.indices.putMapping({
        index: indexname,
        type: '_doc',
        body: {
        properties: { 
            time: { type: 'date' },
            temp: { type: 'float' },
            hum: { type: 'float' },
            pressure: { type: 'float' },
            index: { type: 'integer' },
            room: { type: 'keyword' } }
        }
    }, (err,resp, status) => {
        if (err) {
          console.error(err, status);
        }
        else {
            console.log('Successfully Created Index', status, resp);
            subscribe();
        }
    });
}

