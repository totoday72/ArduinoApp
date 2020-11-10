import 'package:flutter/widgets.dart';
import 'package:charts_flutter/flutter.dart' as charts;

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
