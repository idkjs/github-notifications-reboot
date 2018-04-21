[@bs.module "react-json-view"]
external reactClass : ReasonReact.reactClass = "default";

let make = (~src: option(Js.Json.t)=?, children) =>
  ReasonReact.wrapJsForReason(
    ~reactClass,
    ~props={"src": Js.Null_undefined.fromOption(src)},
    children,
  );