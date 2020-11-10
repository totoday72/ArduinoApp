import 'dart:convert';

import 'package:flutter/widgets.dart';
import 'handler.dart';
import 'package:http/http.dart' as http;

class PanelHandler extends AbstracHandler {
  @override
  bool canHandle(Object request) {
    return request == "panel";
  }

  @override
  Object mainHandle(Object request) {
    return PanelWidget();
  }
}

class PanelWidget extends StatefulWidget {
  @override
  _PanelWidgetState createState() => _PanelWidgetState();
}

class _PanelWidgetState extends State<PanelWidget> {
  List<dynamic> temperaturaList = [];

  @override
  void initState() {
    super.initState();
    getTemperaturaList();
  }

  Future<void> getTemperaturaList() async {
    final res = await http.get("https://api.taskycodes.com/verrtodoproyecto2");

    setState(() {
      temperaturaList = json.decode(res.body);
    });
  }

  Widget generateItemList(dynamic data) {
    return Row(
      children: [
        Expanded(child: Text("${data["fecha"]}")),
        Expanded(
            child: Align(
                alignment: Alignment.center,
                child: Text("${data["usuario"]}"))),
        Expanded(
            child: Align(
                alignment: Alignment.center,
                child: Text("${data["temperaturaCorporal"]}"))),
        Expanded(
            child: Align(
                alignment: Alignment.center,
                child: Text("${data["temperaturaAmbiente"]}")))
      ],
    );
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      child: Column(
        children: [
          Row(children: [
            Expanded(child: Text("FECHA")),
            Expanded(
                child:
                    Align(alignment: Alignment.center, child: Text("USUARIO"))),
            Expanded(
                child: Align(
                    alignment: Alignment.center, child: Text("CORPORAL (C)"))),
            Expanded(
                child: Align(
                    alignment: Alignment.center, child: Text("AMBIENTE (C)"))),
          ]),
          ...this.temperaturaList.map((item) => generateItemList(item))
        ],
      ),
    );
  }
}
