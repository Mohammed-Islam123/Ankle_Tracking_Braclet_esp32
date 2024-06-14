const mqtt = require("mqtt");

const broker = "mqtt://test.mosquitto.org:1883";
const publishTopic = "zmalaPublish";
const messageInterval = 5000; // 5 seconds

const client = mqtt.connect(broker);
client.on("message", (topic, message) => {
  console.log("I recieved this: " + message);
});
let longitude = -0.6331753;
console.clear();
client.on("connect", function () {
  console.log("mqtt client connected");

  setInterval(() => {
    const message = `{"latitude":35.209770399999999,"longitude":${-0.6331753},"deviceID":1542,"battery":90}`;
    // longitude = longitude - 0.000001;
    client.publish(publishTopic, message);
    console.log("Published message:", message);
  }, messageInterval);
});

client.on("error", function () {
  console.log("There is an error in MQTT");
});
