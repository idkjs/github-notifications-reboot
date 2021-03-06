[%bs.raw {|require('./app.css')|}];

open Notifications;

type action =
  | LoadNotifications(array(notification))
  | DeleteNotification(notification)
  | Login(string)
  | Logout;

type state = {
  token: option(string),
  notifications: option(array(notification)),
};

let githubToken: string = [%bs.raw {| localStorage.getItem("githubToken") |}];

let component = ReasonReact.reducerComponent("App");

let make = _children => {
  ...component,
  initialState: () => {token: None, notifications: None},
  didMount: self => {
    let loadNotifications = token =>
      fetchNotifications(token)
      |> Js.Promise.then_(response =>
           self.reduce(
             () =>
               LoadNotifications(Js.Array.filter(n => n.unread, response)),
             (),
           )
           |> Js.Promise.resolve
         )
      |> ignore;
    switch (Github.getLocalToken()) {
    | Some(token) =>
      self.reduce(() => Login(token), ());
      loadNotifications(token);
    | None =>
      switch (Github.getCodeFromUrl()) {
      | Some(code) =>
        /* request token from api */
        Github.requestToken(code)
        |> Js.Promise.then_(token => {
             self.reduce(() => Login(token), ());
             Github.setLocalToken(token);
             Js.Promise.resolve(loadNotifications(token));
           })
        |> ignore
      | None => ()
      };
      ();
    };
    ReasonReact.NoUpdate;
  },
  reducer: (action, state) =>
    switch (action) {
    | LoadNotifications(notifications) =>
      ReasonReact.Update({...state, notifications: Some(notifications)})
    | DeleteNotification(notification) =>
      switch (state.notifications) {
      | Some(notifications) =>
        ReasonReact.Update({
          ...state,
          notifications:
            Some(
              Js.Array.filter(n => n.id != notification.id, notifications),
            ),
        })
      | None => ReasonReact.NoUpdate
      }
    | Login(token) => ReasonReact.Update({...state, token: Some(token)})
    | Logout => ReasonReact.Update({token: None, notifications: None})
    },
  render: self => {
    let loggedIn =
      switch (self.state.token) {
      | Some(_) => true
      | None => false
      };
    let logout = e =>
      self.reduce(
        e => {
          ReactEventRe.Mouse.preventDefault(e);
          Github.deleteLocalToken();
          Logout;
        },
        e,
      );
    let deleteNotification = n =>
      markAsRead(githubToken, n)
      |> Js.Promise.then_(success => {
           success ?
             self.reduce(() => DeleteNotification(n), ()) :
             Js.log("Could not delete notification " ++ n.id);
           Js.Promise.resolve();
         })
      |> ignore;
    let notificationList =
      switch (self.state.notifications) {
      | Some(n) =>
        <NotificationList notifications=n markAsRead=deleteNotification />
      | None =>
        loggedIn ?
          <div className="mw6 center tc mv3">
            <h2 className="f5 f4-m f3-l fw2 black-50 mt0 lh-copy">
              (ReasonReact.stringToElement("Loading..."))
            </h2>
          </div> :
          <div className="mw6 center tc mv3">
            <h2 className="f5 f4-m f3-l fw2 black-50 mt0 lh-copy">
              (ReasonReact.stringToElement("Log in to see notifications."))
            </h2>
          </div>
      };
    <div className="App">
      <Header loggedIn onLogout=logout />
      notificationList
    </div>;
  },
};