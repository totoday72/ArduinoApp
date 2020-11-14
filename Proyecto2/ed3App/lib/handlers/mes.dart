import 'dart:convert';

import 'package:ed3App/handlers/handler.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

import 'package:http/http.dart' as http;

import '../temperatura_chart.dart';

class MesHanlder extends AbstracHandler {
  final DateTime mes;

  MesHanlder(this.mes);

  @override
  bool canHandle(Object request) {
    return request == "mes";
  }

  @override
  Object mainHandle(Object request) {
    return MesWidget(this.mes);
  }
}

class MesWidget extends StatefulWidget {
  final DateTime mes;

  MesWidget(this.mes);
  @override
  _MesWidgetState createState() => _MesWidgetState();
}

class _MesWidgetState extends State<MesWidget> {
  DateTime selected;

  List<dynamic> data = [];

  get last5 => this
      .data
      .map<TemperaturaRegistrada>((e) => TemperaturaRegistrada(
          DateTime.parse("${e["fecha"]}"),
          double.parse("${e["temperaturaCorporal"]}")))
      .toList();

  @override
  void initState() {
    super.initState();

    changeSelected(widget.mes);
  }

  Future<void> changeSelected(DateTime mes) async {
    setState(() {
      this.selected = mes;
    });
    final data = await getData(mes);

    setState(() {
      this.data = data;
    });
  }

  Future<List<dynamic>> getData(DateTime mes) async {
    final res = await http.get(
        "https://api.taskycodes.com/verdetalleusuariosenfermos?fecha=${mes.year}-${mes.month}-${mes.day}");

    return json.decode(res.body);
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
                    onPressed: null,
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

  @override
  Widget build(BuildContext context) {
    return Container(
      child: Column(
        children: [
          FlatButton(
              onPressed: () async {
                final dd = await showDatePicker(
                    context: context,
                    initialDate: selected,
                    firstDate: DateTime.parse("2020-01-01"),
                    lastDate: DateTime.now());

                if (dd != null) {
                  this.changeSelected(dd);
                }
              },
              child: Text("${selected.year}-${selected.month}")),
          Container(
            margin: EdgeInsets.fromLTRB(0, 0, 0, 25),
            height: 350,
            child: TemperaturaChart(this.last5),
          ),
          Row(
            children: [
              ...["FECHA", "USUARIO", "T. CORPORAL (C)", "T. AMBIENTE (C)"]
                  .map((e) => this.generateHeadItemList(e))
            ],
          ),
          ...this.data.map((e) => this.generateItemList(e)),
        ],
      ),
    );
  }

  generateHeadItemList(e) {
    return Expanded(child: Text(e));
  }
}
