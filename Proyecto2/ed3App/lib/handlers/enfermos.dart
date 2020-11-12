import 'dart:convert';

import 'package:ed3App/handlers/handler.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

import 'package:http/http.dart' as http;

import '../temperatura_chart.dart';

class EnfermosHanlder extends AbstracHandler {
  final DateTime mes;

  EnfermosHanlder(this.mes);

  @override
  bool canHandle(Object request) {
    return request == "enfermos";
  }

  @override
  Object mainHandle(Object request) {
    return EnfermosWidget(this.mes);
  }
}

class EnfermosWidget extends StatefulWidget {
  final DateTime mes;

  EnfermosWidget(this.mes);
  @override
  _EnfermosWidgetState createState() => _EnfermosWidgetState();
}

class _EnfermosWidgetState extends State<EnfermosWidget> {
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
        "https://api.taskycodes.com/verusuariosenfermos?fecha=${mes.year}-${mes.month}-${mes.day}");

    return json.decode(res.body);
  }

  Widget generateItemList(dynamic data) {
    return Row(
      children: [
        Expanded(
            child: Align(
                alignment: Alignment.center,
                child: FlatButton(
                    color: Colors.amber,
                    onPressed: null,
                    child: Text("${data["usuario"]}")))),
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
          Row(
            children: [
              ...["USUARIO"].map((e) => this.generateHeadItemList(e))
            ],
          ),
          ...this.data.map((e) => this.generateItemList(e)),
        ],
      ),
    );
  }

  generateHeadItemList(e) {
    return Expanded(
        child: Align(
      alignment: Alignment.center,
      child: Text(e),
    ));
  }
}
