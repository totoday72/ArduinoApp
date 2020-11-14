import 'dart:convert';

import 'package:ed3App/handlers/detalle.dart';
import 'package:flutter/material.dart';
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

  dynamic selected;

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
                child: FlatButton(
                    color: Colors.amber,
                    onPressed: () {
                      setState(() {
                        selected = data;
                      });
                    },
                    child: Text("${data["usuario"]}")))),
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

  get username => selected["usuario"];

  @override
  Widget build(BuildContext context) {
    return Container(
      child: selected != null
          ? Column(
              children: [
                Align(
                  alignment: Alignment.topRight,
                  child: FlatButton(
                      onPressed: () {
                        setState(() {
                          selected = null;
                        });
                      },
                      child: Text("CERRAR")),
                ),
                DetalleHandler(this.username).handle("detalle") ??
                    Text("Ocurrio un error, intente mas tarde.")
              ],
            )
          : Column(
              children: [
                Row(children: [
                  Expanded(child: Text("FECHA")),
                  Expanded(
                      child: Align(
                          alignment: Alignment.center, child: Text("USUARIO"))),
                  Expanded(
                      child: Align(
                          alignment: Alignment.center,
                          child: Text("CORPORAL (C)"))),
                  Expanded(
                      child: Align(
                          alignment: Alignment.center,
                          child: Text("AMBIENTE (C)"))),
                ]),
                ...this.temperaturaList.map((item) => generateItemList(item))
              ],
            ),
    );
  }
}
