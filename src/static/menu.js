// Collect Buttons
joybtn = document.getElementById('joystick-btn');
btnbtn = document.getElementById('button-btn');
pidbtn = document.getElementById('pid-btn');
databtn = document.getElementById('data-btn');
graphbtn = document.getElementById('graph-btn');
conbtn = document.getElementById('console-btn');


// Collect Tabs
joypane = document.getElementById('joystick-pane');
btnpane = document.getElementById('button-pane');
pidpane = document.getElementById('pid-pane');

datapane = document.getElementById('data-pane');
graphpane = document.getElementById('graph-pane');
conpane = document.getElementById('console-pane');

// add onclicks
joybtn.addEventListener("click", function() {
  btnpane.classList.add("hide");
  pidpane.classList.add("hide");
  joypane.classList.remove("hide");
});
btnbtn.addEventListener("click", function() {
  btnpane.classList.remove("hide");
  pidpane.classList.add("hide");
  joypane.classList.add("hide");
});
pidbtn.addEventListener("click", function() {
  btnpane.classList.add("hide");
  pidpane.classList.remove("hide");
  joypane.classList.add("hide");
});

// add onclicks
databtn.addEventListener("click", function() {
  conpane.classList.add("hide");
  graphpane.classList.add("hide");
  datapane.classList.remove("hide");
});
graphbtn.addEventListener("click", function() {
  conpane.classList.add("hide");
  graphpane.classList.remove("hide");
  datapane.classList.add("hide");
});
conbtn.addEventListener("click", function() {
  conpane.classList.remove("hide");
  graphpane.classList.add("hide");
  datapane.classList.add("hide");
});
