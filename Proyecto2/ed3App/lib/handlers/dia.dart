import 'dart:convert';

import 'package:ed3App/handlers/detalle.dart';
import 'package:ed3App/temperatura_chart.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';
import 'handler.dart';
import 'package:http/http.dart' as http;

class DiaHandler extends AbstracHandler {
  final DateTime dia;

  DiaHandler(this.dia);
  @override
  bool canHandle(Object request) {
    return request == "dia";
  }

  @override
  Object mainHandle(Object request) {
    return DiaWidget(this.dia);
  }
}

class DiaWidget extends StatefulWidget {
  final DateTime dia;

  DiaWidget(this.dia);

  @override
  _DiaWidgetState createState() => _DiaWidgetState();
}

class _DiaWidgetState extends State<DiaWidget> {
  List<dynamic> temperaturaList = [];

  DateTime selected;
  dynamic userSelected;

  @override
  void initState() {
    super.initState();
    setState(() {
      selected = widget.dia;
    });
    getTemperaturaList(widget.dia);
  }

  Future<void> getTemperaturaList(DateTime dia) async {
    final res = await http
        .get("https://api.taskycodes.com/vertemperaturaspordia?fecha=$dia");

    setState(() {
      temperaturaList = json.decode(res.body);
    });
  }

  Widget generateItemList(dynamic data) {
    return Row(
      children: [
        Expanded(child: Text("${data["hora"]}")),
        Expanded(
            child: Align(
                alignment: Alignment.center,
                child: FlatButton(
                    color: Colors.amber,
                    onPressed: () {
                      setState(() {
                        userSelected = data;
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

  get username => this.userSelected["usuario"];

  DateTime getDaySelectedAtTime(String time) {
    return DateTime.parse(
        "${this.selected.year}-${this.selected.month}-${this.selected.day} $time");
  }

  TemperaturaRegistrada getTempRegFromDynamic(dynamic item) {
    return TemperaturaRegistrada(getDaySelectedAtTime(item["hora"]),
        double.parse("${item["temperaturaCorporal"]}"));
  }

  get last5 => this
      .temperaturaList
      .map<TemperaturaRegistrada>((e) => getTempRegFromDynamic(e))
      .toList();

  @override
  Widget build(BuildContext context) {
    return Container(
      child: userSelected != null
          ? Column(
              children: [
                Align(
                  alignment: Alignment.topRight,
                  child: FlatButton(
                      onPressed: () {
                        setState(() {
                          userSelected = null;
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
                FlatButton(
                    onPressed: () async {
                      final dd = await showDatePicker(
                          context: context,
                          initialDate: selected,
                          firstDate: selected.subtract(Duration(days: 365)),
                          lastDate: DateTime.now());

                      setState(() {
                        selected = dd;
                      });

                      this.getTemperaturaList(dd);
                    },
                    child: Text(
                        "${selected.year}-${selected.month}-${selected.day}")),
                Container(
                  margin: EdgeInsets.fromLTRB(0, 0, 0, 25),
                  height: 350,
                  child: TemperaturaChart(this.last5),
                ),
                Row(children: [
                  Expanded(child: Text("HORA")),
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
