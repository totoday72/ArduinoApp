import 'package:flutter/widgets.dart';
import 'package:charts_flutter/flutter.dart' as charts;

class TemperaturaRegistrada {
  final DateTime fecha;
  final double valor;
  TemperaturaRegistrada(this.fecha, this.valor);
}

class TemperaturaChart extends StatelessWidget {
  final List<TemperaturaRegistrada> data;

  TemperaturaChart(this.data);

  @override
  Widget build(BuildContext context) {
    return new charts.TimeSeriesChart(
      [
        new charts.Series<TemperaturaRegistrada, DateTime>(
            id: "dd",
            data: this.data,
            domainFn: (TemperaturaRegistrada d, _) => d.fecha,
            measureFn: (TemperaturaRegistrada d, _) => d.valor)
      ],
      animate: false,
    );
  }
}
