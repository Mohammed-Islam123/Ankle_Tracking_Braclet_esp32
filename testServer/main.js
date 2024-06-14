const express = require("express");
const mqtt = require("mqtt");
const app = express();
const fs = require("fs");
const broker = "mqtt://test.mosquitto.org:1883";
const receiveTopic = "zmalaPublish";
const client = mqtt.connect(broker); // Connect to a public broker for demo purposes
const publishTopic = "zmalaSub";
const listenPort = 3030;

fs.appendFile(
  "./Mqtt_Logs.txt",
  "\n#################################################\nNew Test started in: \n" +
    new Date().toString(),
  "utf-8",
  (err) => {
    if (err) {
      console.error("An error occurred:", err);
    } else {
      console.log("Date and time have been appended to file");
    }
  }
);

let startTime;
let description;
console.clear();
client.on("connect", function () {
  console.log("mqtt client connected");
});

// Endpoint to publish a message to the MQTT broker
app.post("/publish", (req, res) => {
  client.publish(publishTopic, req.params.payload);
  res.send("Published message to topic");
});

// Subscribe to a topic and log messages
client.subscribe(receiveTopic, function (err) {
  if (!err) {
    console.log("Successfullly subscribed !!in topic " + receiveTopic);
    startTime = new Date();
  } else console.log("the error is " + err);
  console.log();
});

client.on("message", function (topic, message) {
  const endTime = new Date();
  const elapsedTime = endTime - startTime;

  description = `\nThe Message is: " + ${message.toString()}\nTime taken to receive the message: ${elapsedTime} ms\n
  message recieved at :
    ${endTime.getHours()}:${endTime.getMinutes()}:${endTime.getSeconds()}`;
  let pubPayload = "NONE";
  let data;
  try {
    data = JSON.parse(message.toString());
    deviceID = data.deviceID;
    description += "\nLatitude : " + data.latitude;
    description += "\nLongitude : " + data.longitude;
  } catch (error) {
    console.error("Error parsing message:", error);
  }
  startTime = new Date();
  // Your existing code here

  client.publish(`prisioner-alert/${deviceID}`, "utside");

  description += "\n-----------------------------------";
  console.log(description);
  // fs.appendFile("./Mqtt_Logs.txt", description, "utf-8", (err) => {
  //   if (err) {
  //     console.error("An error occurred:", err);
  //   } else {
  //     console.log("Date and time have been appended to file");
  //   }
  // });
});
client.on("error", function () {
  console.log("There is an error in MQTT");
});

app.listen(listenPort, () => {
  console.log("Express server started on port " + listenPort);
});
