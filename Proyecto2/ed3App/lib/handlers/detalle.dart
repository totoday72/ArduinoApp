import 'dart:convert';

import 'package:flutter/widgets.dart';

import '../temperatura_chart.dart';
import 'handler.dart';
import "package:http/http.dart" as http;

class DetalleHandler extends AbstracHandler {
  final String username;

  DetalleHandler(this.username);

  @override
  bool canHandle(Object request) {
    return request == "detalle";
  }

  @override
  Object mainHandle(Object request) {
    return DetalleWidget(this.username);
  }
}

class DetalleWidget extends StatefulWidget {
  final String username;

  DetalleWidget(this.username);

  @override
  _DetalleWidgetState createState() => _DetalleWidgetState();
}

class _DetalleWidgetState extends State<DetalleWidget> {
  List<dynamic> temperaturaList = [];

  @override
  void initState() {
    super.initState();
    cargarDetalle(widget.username);
  }

  Future<void> cargarDetalle(String username) async {
    final res = await http.get(
        "https://api.taskycodes.com/verrtemperaturasusuarioproyecto2?usuario=$username");

    setState(() {
      temperaturaList = json.decode(res.body);
    });
  }

  get data => temperaturaList
      .map<TemperaturaRegistrada>((item) => TemperaturaRegistrada(
          DateTime.parse(item["fecha"]), item["temperaturaCorporal"]))
      .toList();

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: EdgeInsets.all(10),
      height: 300,
      child: Column(
        children: [
          Row(
            children: [
              Expanded(
                  child: Text(
                "${widget.username}",
                style: TextStyle(fontWeight: FontWeight.w900, fontSize: 24),
                textAlign: TextAlign.center,
              )),
            ],
          ),
          Expanded(
              child: TemperaturaChart(temperaturaList
                  .map<TemperaturaRegistrada>((item) => TemperaturaRegistrada(
                      DateTime.parse("${item["fecha"]} ${item["hora"]}"),
                      double.parse(item["temperaturaCorporal"].toString())))
                  .toList()))
        ],
      ),
    );
  }
}
