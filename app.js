var mqtt = require('mqtt')
var elasticsearch = require('elasticsearch');


var client  = mqtt.connect('mqtt://mosquitto')
client.on('connect', function () {
  client.subscribe('room-status', function (err) {
    if (!err) {
//      client.publish('room-status', 'Hello mqtt')
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
          //console.log(dat)
      
          eclient.index({  
              index: indexname,
              type: '_doc',
              body: dat
          },function(err,resp,status) {
              if(err) 
              console.err("Error while indexing document to Elasticsearch");
              else
              console.log("Entry was saved")
          });
      } catch(e){
          console.log("Error while converting MQTT Message to Elasticsearch"+e);
      }
      
      //  client.end()
      })
}

function runme() {

 eclient = new elasticsearch.Client({
    host: 'elasticsearch:9200',
 //   log: 'trace',
    apiVersion: '7.2', // use the same version of your Elasticsearch instance
  });
eclient.ping({
    requestTimeout: 30000
  }, function (error) {
    if (error) {
//      console.err('elasticsearch cluster is down!');
        console.log("ElasticSearch not ready yet. Will retry in 10secs....")
        setTimeout(runme,10000)
    } else {
        eclient.indices.exists({index: indexname}, (err, res, status) => {
            if(err){
                console.err("Error while checking ElasticSearch index")
            }
            else
            if (res) {
                console.log('index already exists');
                subscribe()
            } else {
                eclient.indices.create( {index: indexname}, (err, res, status) => {
                    putMapping(eclient)
                console.log("***",err, res, status);
            })
          }
        })
      }
  });
}

runme()

async function putMapping (eclient) {
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

