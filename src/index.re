[%bs.raw {|require('tachyons/css/tachyons.css')|}];

[%bs.raw {|require('./index.css')|}];

[@bs.module "./registerServiceWorker"]
external register_service_worker : unit => unit = "default";

ReactDOMRe.renderToElementWithId(<App />, "root");

register_service_worker();