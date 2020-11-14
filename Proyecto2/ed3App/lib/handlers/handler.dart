import 'package:ed3App/temperatura_chart.dart';
import 'package:flutter/cupertino.dart';

class IHandler {
  IHandler setNext(IHandler handler) {}
  handle(Object request) {}
}

abstract class AbstracHandler implements IHandler {
  IHandler next;

  bool canHandle(Object request);
  Object mainHandle(Object request);

  @override
  handle(Object request) {
    if (canHandle(request)) {
      return mainHandle(request);
    }
    if (next != null) {
      return next.handle(request);
    }
    return null;
  }

  @override
  setNext(IHandler handler) {
    this.next = handler;
    return handler;
  }
}

class WaitingHandler extends AbstracHandler {
  @override
  bool canHandle(Object request) {
    return request == "waiting";
  }

  @override
  Object mainHandle(Object request) {
    return WaitingWidget();
  }
}

class WaitingWidget extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Text("waiting...");
  }
}

class CleaningHandler extends AbstracHandler {
  @override
  bool canHandle(Object request) {
    return request == "cleaning";
  }

  @override
  Object mainHandle(Object request) {
    return CleaningWidget("MY-USERNAME");
  }
}

class CleaningWidget extends StatelessWidget {
  static int timer = 22;
  final username;

  CleaningWidget(this.username);

  @override
  Widget build(BuildContext context) {
    return Container(
      child: Padding(
        padding: EdgeInsets.all(10),
        child: Column(
          children: [
            Row(
              children: [
                Expanded(child: Text("$username")),
                Expanded(
                    child: Align(
                  alignment: Alignment.center,
                  child: Text("$timer"),
                ))
              ],
            ),
            // Expanded(child: TemperaturaChart())
          ],
        ),
      ),
    );
  }
}
