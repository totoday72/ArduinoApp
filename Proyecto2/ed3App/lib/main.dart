import 'dart:convert';
import 'dart:ffi';

import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';
import './handler.dart';
import 'package:charts_flutter/flutter.dart' as charts;

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'ED3 App',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // Try running your application with "flutter run". You'll see the
        // application has a blue toolbar. Then, without quitting the app, try
        // changing the primarySwatch below to Colors.green and then invoke
        // "hot reload" (press "r" in the console where you ran "flutter run",
        // or simply save your changes to "hot reload" in a Flutter IDE).
        // Notice that the counter didn't reset back to zero; the application
        // is not restarted.
        primarySwatch: Colors.orange,
        // This makes the visual density adapt to the platform that you run
        // the app on. For desktop platforms, the controls will be smaller and
        // closer together (more dense) than on mobile platforms.
        visualDensity: VisualDensity.adaptivePlatformDensity,
      ),
      home: MyHomePage(title: 'ED3'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  MyHomePage({Key key, this.title}) : super(key: key);

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  var pp = new WaitingHandler();

  var isMqttConnected = false;
  static const ARDUINO_TOPIC = "channels/1117472/subscribe/json";
  final mqttChannels = {
    ARDUINO_TOPIC: {
      "conectado": false,
      "nombre": "ARDUINO",
      "campos": [
        {"nombre": "Created at", "key": "created_at", "valor": null},
        {"nombre": "Ubicación", "key": "field1", "valor": null},
        {"nombre": "Estado", "key": "field2", "valor": null},
        {"nombre": "Paquetes entregados", "key": "field3", "valor": 0},
        {"nombre": "Obstaculos", "key": "field4", "valor": 0},
        {"nombre": "Peso", "key": "field5", "valor": 0},
        {"nombre": "Tiempo de entrega", "key": "field6", "valor": null},
        {"nombre": "Tiempo de retorno", "key": "field7", "valor": null},
      ],
      "valor": null
    }
  };

  static const MQTT_SERVER = "mqtt.thingspeak.com";
  static const MQTT_CLIENT_ID = "movil_client";
  static const MQTT_USER = "movil_user";
  static const MQTT_KEY = "RXRMKFZ36GMDVRGB";

  MqttServerClient client =
      MqttServerClient.withPort(MQTT_SERVER, MQTT_CLIENT_ID, 1883);

  void onMqttConnected() {
    print("mqtt connected");

    setState(() {
      isMqttConnected = true;
    });
  }

  void onMqttDisconnected() {
    print("mqtt disconnected");
    setState(() {
      isMqttConnected = false;
    });
  }

  void onMqttSubscribed(to) {
    setState(() {
      this.mqttChannels[to]["conectado"] = true;
    });
  }

  void onMqttUnsubscribed(to) {
    setState(() {
      this.mqttChannels[to]["conectado"] = false;
    });

    if (to == ARDUINO_TOPIC) {
      List campos = this.mqttChannels[to]["campos"];
      campos[0]["valor"] = null;
      campos[1]["valor"] = "";
      campos[2]["valor"] = "";
      campos[3]["valor"] = "0";
      campos[4]["valor"] = "0";
      campos[5]["valor"] = "0";
      campos[6]["valor"] = null;
      campos[7]["valor"] = null;
    }
  }

  void onMqttSubscribeFailed(to) {
    print(to);
  }

  Function(bool) mqttTopicSubscription(String topic) {
    return (val) {
      if (val) {
        this.client.subscribe(topic, MqttQos.atMostOnce);
      } else {
        this.client.unsubscribe(topic);
      }
    };
  }

  void onMqttMessageReceived(List<MqttReceivedMessage<MqttMessage>> c) {
    final MqttPublishMessage message = c[0].payload;
    final payload =
        MqttPublishPayload.bytesToStringAsString(message.payload.message);

    final topic = c[0].topic;
    final localTopic = this.mqttChannels[topic];

    switch (topic) {
      // case ARDUINO_TOPIC:
      //   break;
      default:
        print("${localTopic["nombre"]}: $payload");
    }

    setState(() {
      localTopic["valor"] = payload;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Padding(
        padding: EdgeInsets.all(15),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.start,
          children: [
            Row(
              children: [
                Expanded(
                    child: Column(
                  children: [
                    Text("MQTT"),
                    Switch(
                        value: this.isMqttConnected,
                        onChanged: (to) async {
                          try {
                            if (to) {
                              // client.logging(on: true);

                              client.onConnected = this.onMqttConnected;
                              client.onDisconnected = this.onMqttDisconnected;
                              client.onSubscribed = this.onMqttSubscribed;
                              client.onSubscribeFail =
                                  this.onMqttSubscribeFailed;
                              client.onUnsubscribed = this.onMqttUnsubscribed;
                              final connMesage = MqttConnectMessage()
                                  .authenticateAs(MQTT_USER, MQTT_KEY)
                                  .withClientIdentifier(MQTT_CLIENT_ID)
                                  .startClean();

                              client.connectionMessage = connMesage;

                              print("mqtt connecting...");
                              await client.connect();
                              // print(status);

                              client.updates.listen(onMqttMessageReceived);
                            } else {
                              client.disconnect();
                            }
                          } catch (e) {
                            print(e.toString());
                          }
                        }),
                  ],
                )),
                Expanded(
                    child: Column(children: [
                  Text("ARDUINO "),
                  Switch(
                      value: this.mqttChannels[ARDUINO_TOPIC]["conectado"],
                      onChanged: this.mqttTopicSubscription(ARDUINO_TOPIC)),
                ]))
              ],
            ),
            Padding(
              padding: EdgeInsets.all(20),
              child: Text("MY_USERNAME"),
            ),
            Expanded(child: TemperaturaChart()),
          ],
        ),
      ),
    );
  }
}

class TemperaturaRegistrada {
  final DateTime fecha;
  final double valor;
  TemperaturaRegistrada(this.fecha, this.valor);
}

class TemperaturaChart extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return new charts.TimeSeriesChart(
      [
        new charts.Series<TemperaturaRegistrada, DateTime>(
            id: "dd",
            data: [
              TemperaturaRegistrada(DateTime(2020, 10, 1), 32.3),
              TemperaturaRegistrada(DateTime(2020, 10, 2), 40.3),
              TemperaturaRegistrada(DateTime(2020, 10, 3), 30.3),
              TemperaturaRegistrada(DateTime(2020, 10, 4), 10.3),
              TemperaturaRegistrada(DateTime(2020, 10, 5), 30.3),
            ],
            domainFn: (TemperaturaRegistrada d, _) => d.fecha,
            measureFn: (TemperaturaRegistrada d, _) => d.valor)
      ],
      animate: false,
    );
  }
}
