class IHandler {
  setNext(IHandler handler) {}
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
    return request;
  }
}
