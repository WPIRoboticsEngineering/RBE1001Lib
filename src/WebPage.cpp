#include <Arduino.h>
#include "WebPage.h"
#include "SimpleWebServer.h"

String buttonStyle = R"=====(
<!DOCTYPE html>
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: left;}
.button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;
text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
</style>

<html>
<body><h1>ESP32 Web Server</h1>

<script>
setInterval(function() {
  // Call a function repetatively with 1 Second interval
  getData();
  }, 1000); //2000mSeconds update rate

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("DataValue").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "readData", true);
  xhttp.send();
}
</script>
)=====";

/*
 * this stuff is a kludge, but seemed worth it for now to remove complexity
 * from the part of the program developed by students. I'll fix it more
 * generally later.
 */
static WebPage *thisPage;

static void update() {
  thisPage->dataUpdate();
}

void WebPage::dataUpdate() {
  ws.sendResponse(getHTML()); //Send Data value only to client ajax request
}

WebPage::WebPage(SimpleWebServer& sws): ws(sws) {
  contents = buttonStyle;
  head = NULL;
  thisPage = this;
  ws.registerHandler("/readData", update);
}

void WebPage::add(String s) {
  contents += s + "\n";
}

void WebPage::finishPage() {
  add("<span id=\"DataValue\"> </span><br>");
  add("</body></html>");
  add("");
}

String WebPage::getPage() {
  return contents;
}

void WebPage::newButton(String url, void (*handler)(), String label, String description) {
  ws.registerHandler("/" + url, handler);
  add("<p><a href=\"/" + url + "\"><button class=\"button\">" + label + "</button></a>");
  add(description + "</p>");
}

void WebPage::setValue(String name, float value) {
    for (DataValues *dv = head; dv; dv = dv->next) {
        if (dv->name == name) {
            dv->value = value;
            return;
        }
    }
    DataValues *dv = new DataValues;
    dv->name = String(name);
    dv->value = value;
    dv->next = head;
    head = dv;
}

String WebPage::getHTML() {
  String table = "<table>\n";
    for (DataValues *dv = head; dv; dv = dv->next) {
        table += "<tr><td>" + dv->name + "</td><td>" + String(dv->value) + "</td></tr>\n";
    }
    table += "</table>\n";
    return table;
}

void WebPage::sendHTML() {
  String page = getPage();
  ws.sendHTMLResponse(page);
}
