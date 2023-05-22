var tools = [0, 0, 0]; //pen, eraser, fill
var canvasMouseX = 0; //position of the mouse on the canvas element
var canvasMouseY = 0;
var isDrawing = false;
var color = "#FF0000"; //color wich is used to paint
var pickedColor = "#FF0000"; //color of the colorpicker
var canvas_size = 32;
var currentPattern = new Array(canvas_size); //stores the displayed pattern
var prevX = -1; //used to prevent spam
var prevY = -1;
var names; //stores the names of the patterns
var live = false; //draw live on canvas or display saved patterns

function changeTool(id_number) {
  for (let i = 0; i < tools.length; i++) {
    tools[i] = 0;
  }
  tools[id_number] = 1;

  //highlight selected tool
  tools[0] == 0
    ? (document.getElementById("pencil").style.backgroundColor = "#FFFFFF00")
    : (document.getElementById("pencil").style.backgroundColor = "#524561");
  tools[1] == 0
    ? (document.getElementById("eraser").style.backgroundColor = "#FFFFFF00")
    : (document.getElementById("eraser").style.backgroundColor = "#524561");
  tools[2] == 0
    ? (document.getElementById("fill").style.backgroundColor = "#FFFFFF00")
    : (document.getElementById("fill").style.backgroundColor = "#524561");
}

function changeColor(tempcolor) {
  color = tempcolor;
}

function changeMode() {
  live = !live;
  //highlight selection if true
  live
    ? (document.getElementById("live").style.backgroundColor = "#524561")
    : (document.getElementById("live").style.backgroundColor = "#FFFFFF00");

  //new POST request to the arduino to change mode
  var xhr = new XMLHttpRequest();
  xhr.open("POST", "mode", true);
  xhr.setRequestHeader(
    "Content-Type",
    "application/x-www-form-urlencoded; charset=UTF-8"
  );
  xhr.send("mode=" + live);
}

function draw(x, y, drawColor, post) {
  var canvas = document.getElementById("canvas");
  var ctx = canvas.getContext("2d");
  var rect = canvas.getBoundingClientRect();
  var size = rect.width / canvas_size;
  var index = y * canvas_size + x;
  if (post && live) {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "live", true);
    xhr.setRequestHeader(
      "Content-Type",
      "application/x-www-form-urlencoded; charset=UTF-8"
    );
    xhr.send("i=" + index + "&c=" + drawColor.slice(1));
  }
  //update currentPattern array with the new color and draw new pixel on canvas
  currentPattern[y][x] = drawColor;

  x = x * size;
  y = y * size;
  ctx.fillStyle = drawColor;
  if (drawColor == "#000000" || drawColor == "0x000000") {
    //black pixel = transparent pixel (looks better)
    ctx.clearRect(x, y, size, size);
  } else {
    ctx.fillRect(x, y, size, size);
  }
  drawGrid(0, 0); //redraw the grid
}

function drawGrid(x, y) {
  var canvas = document.getElementById("canvas");
  var ctx = canvas.getContext("2d");
  var rect = canvas.getBoundingClientRect();
  var size = rect.width / canvas_size;
  for (var i = 0; i < canvas_size; i++) {
    for (var j = 0; j < canvas_size; j++) {
      x = i * size;
      y = j * size;
      ctx.strokeRect(x, y, size, size);
    }
  }
}

function fill(x, y, referenceColor) {
  /*
          This is a very basic fill algorithm making use of recursion. I added a little delay to make it
          animate a little. Unfortunately the POST requests that "draw()" makes are quite slow
          */
  setTimeout(function () {
    if (x >= 0 && x <= canvas_size - 1 && y >= 0 && y <= canvas_size - 1) {
      if (currentPattern[y][x] == referenceColor) {
        draw(x, y, color, 1);
        fill(x, y - 1, referenceColor);
        fill(x, y + 1, referenceColor);
        fill(x - 1, y, referenceColor);
        fill(x + 1, y, referenceColor);
      }
    }
  }, 40);
}

function clearCanvas() {
  var canvas = document.getElementById("canvas");
  var ctx = canvas.getContext("2d");
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  for (var i = 0; i < canvas_size; i++) {
    for (var j = 0; j < canvas_size; j++) {
      currentPattern[i][j] = "#000000";
    }
  }
  drawGrid(0, 0);

  if (live) {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "live", true);
    xhr.setRequestHeader(
      "Content-Type",
      "application/x-www-form-urlencoded; charset=UTF-8"
    );
    xhr.send("cl=1");
  }
}

function getNames() {
  var selection = document.getElementById("selection");

  //make a new GET request to the arduino which reads the names from the SD card
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open("GET", "get", false);
  xmlHttp.send();
  console.log(xmlHttp.responseText);

  //response is JSON array which we parse into a variable and then
  //make appear in the selection of the HTML
  names = JSON.parse(xmlHttp.responseText);
  for (var i = 0; i < names.length; i++) {
    if (!names[i].includes("^")) {
      var option = document.createElement("option");
      option.value = i;
      option.innerHTML = names[i];
      selection.appendChild(option);
    }
  }
}

// function getCanvasSizes()
// {
//     var tamanos = document.getElementById("canv_size");

//     tamanos.style.fontSize = "18px";
//     var option1 = document.createElement('option');
//     var option2 = document.createElement('option');
//     option1.value = 1;
//     option1.innerHTML = "16x16";
//     tamanos.appendChild(option1);
//     option2.value = 2;
//     option2.innerHTML = "32x32";
//     tamanos.appendChild(option2);
// }

function save() {
  var name = window.prompt("Enter a name:", "mario");
  if (name.length > 15) {
    window.alert("Name can't be longer than 15 characters :(");
  } else {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "save", true);
    xhr.setRequestHeader(
      "Content-Type",
      "application/x-www-form-urlencoded; charset=UTF-8"
    );
    var message = "pt=";

    // Las imagenes 64x64 se las tendrá que acordar el ESP
    // No puedo mandarselas porque rompe todo!
    // Solo le mando el nombre ;)
    if (canvas_size != 64) {
      //convert into format that the arduino uses
      for (var i = 0; i < canvas_size; i++) {
        for (var j = 0; j < canvas_size; j++) {
          message += "0x" + currentPattern[i][j].slice(1) + "\n";
        }
      }
    } //si el canvas es de 64x64 solo le mando nombre de archivo
    else {
      message += "\n";
    }
    message += "&name=" + name;
    console.log(message); //Add by guga to see what it sends
    xhr.send(message);
  }
}

function deletePattern() {
  var name = window.prompt(
    "Enter the name of the pattern that you want to delete:",
    "mario"
  );
  //check if name is valid
  if (!names.includes(name)) {
    window.alert("There is no pattern with this name :(");
  } else {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "delete", true);
    xhr.setRequestHeader(
      "Content-Type",
      "application/x-www-form-urlencoded; charset=UTF-8"
    );
    var message = "name=" + name;
    xhr.send(message);
  }
}

function loadPattern() {
  // Redibujar el canvas cuando tengamos el archivo y sepamos el tamaño de canvas con que lo hicieron!
  // Ver si puede ser medio "automático"
  //get the value of the select tag
  var select = document.getElementById("selection");
  var id = select.value;

  //new Request to the arduino
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open("POST", "load", false); // false for synchronous request
  xmlHttp.setRequestHeader(
    "Content-Type",
    "application/x-www-form-urlencoded; charset=UTF-8"
  );
  var message = "id=" + id;
  xmlHttp.send(message);

  //parse the response and show it on the screen
  var loadedPattern = JSON.parse(xmlHttp.responseText);
  console.log("Loaded Pattern Lenght:");
  console.log(loadedPattern.length);
  // Habrá que leer y escribir hasta canvas_size PERO DEL ARCHIVO EN CUESTION!
  // CUIDADO
  if (loadedPattern.length == 256) {
    console.log("Canvas 16x16-infered from file");
    canvas_size = 16;
  } else if (loadedPattern.length == 1024) {
    console.log("Canvas 32x32-infered from file");
    canvas_size = 32;
  } else {
    console.log("Canvas 64x64-infered from file");
    canvas_size = 64;
  }
  redrawcanvasload(); //ver si puedo meterlo aca a huevo!

  for (var i = 0; i < canvas_size; i++) {
    for (var j = 0; j < canvas_size; j++) {
      draw(j, i, "#" + loadedPattern[i * canvas_size + j].slice(2), false); //slice(2) hace la magia de dibujar en al canvas!

      // console.log(i,j);
    }
  }
}

//gets executed when the web page is loaded
function initialise() {
  //get the parameters of the canvas and draw the grid
  var canvas = document.getElementById("canvas");
  var size = canvas.clientWidth;
  canvas.height = size;
  canvas.width = size;
  drawGrid(0, 0);
  changeTool("0");

  //create a 2 dimensional array to store the current pattern
  for (var i = 0; i < canvas_size; i++) {
    currentPattern[i] = new Array(canvas_size);
  }
  for (var i = 0; i < canvas_size; i++) {
    for (var j = 0; j < canvas_size; j++) {
      currentPattern[i][j] = "#000000";
    }
  }
  //get the names and change the mode to live drawing
  getNames(); //comento mientras pruebo huevadas!
  changeMode();
  //getCanvasSizes();
}

//add the event handlers when the webpage is fully loaded
document.addEventListener("DOMContentLoaded", function (event) {
  console.log("DOM fully loaded and parsed");

  canvas.addEventListener("mousemove", function (e) {
    if (isDrawing == true) {
      var canvas = document.getElementById("canvas");
      var rect = canvas.getBoundingClientRect();
      var x = e.clientX - rect.left; //calculate wich of the 256 pixel the mouse is over
      var y = e.clientY - rect.top;
      x = Math.floor(x * (canvas_size / rect.width)); //calculate wich of the 256 pixel the mouse is over
      y = Math.floor(y * (canvas_size / rect.height));
      if (x != prevX || y != prevY) {
        //functions only execute when new square is selected
        if (tools[0] == 1) {
          draw(x, y, color, 1);
        } else if (tools[1] == 1) {
          draw(x, y, "#000000", 1);
        }
      }
      prevX = x;
      prevY = y;
    }
  });

  canvas.addEventListener("mousedown", function (e) {
    var canvas = document.getElementById("canvas");
    console.log("MotherFuckMouse");

    var rect = canvas.getBoundingClientRect();
    var x = e.clientX - rect.left; //calculate wich of the 256 pixel the mouse is over
    var y = e.clientY - rect.top;
    x = Math.floor(x * (canvas_size / rect.width)); //calculate wich of the 256 pixel the mouse is over
    y = Math.floor(y * (canvas_size / rect.height));

    if (x != prevX || y != prevY) {
      //functions only execute when new square is selected
      if (tools[0] == 1) {
        draw(x, y, color, 1);
        isDrawing = true;
      } else if (tools[1] == 1) {
        draw(x, y, "#000000", 1);
        isDrawing = true;
      } else if (tools[2] == 1) {
        fill(x, y, currentPattern[y][x]);
      }
    }
    prevX = x;
    prevY = y;
  });

  canvas.addEventListener("mouseup", function (e) {
    isDrawing = false;
  });

  canvas.addEventListener("mouseout", function (e) {
    //executes when mouse leaves canvas element
    isDrawing = false;
  });

  // -------------------------------IG------------------------------------------

  canvas.addEventListener("touchmove", function (e) {
    e.preventDefault();
    console.log("MotherFuckTouch");
    var touch = e.touches[0];
    var canvas = document.getElementById("canvas");
    var rect = canvas.getBoundingClientRect();
    var x = touch.clientX - rect.left;
    var y = touch.clientY - rect.top;
    x = Math.floor(x * (canvas_size / rect.width));
    y = Math.floor(y * (canvas_size / rect.height));

    if (x != prevX || y != prevY) {
      //functions only execute when new square is selected
      if (tools[0] == 1) {
        draw(x, y, color, 1);
        isDrawing = true;
      } else if (tools[1] == 1) {
        draw(x, y, "#000000", 1);
        isDrawing = true;
      } else if (tools[2] == 1) {
        fill(x, y, currentPattern[y][x]);
      }
    }
    prevX = x;
    prevY = y;
  });

  // ----------------------------------------------------------------------

  var colorpicker = document.getElementById("colorpicker");
  colorpicker.addEventListener("input", function () {
    pickedColor = colorpicker.value;
    color = pickedColor;
    document.getElementById("pickedColor").style.backgroundColor = color;
  });
});

//Guga trampeando para acomodar tamaños automaticamente
// Aca cuando eligen desde el seector, le viso al ESP el size
function redrawcanvasselector() {
  console.log("Re Draw Canvas ON SELECTOR selection ");

  var selector_size = document.getElementById("canv_size");
  if (selector_size.value == 1) {
    console.log("Selector canvas 16x16");
    canvas_size = 16;
  } else if (selector_size.value == 2) {
    console.log("Selector canvas 32x32");
    canvas_size = 32;
  } else if (selector_size.value == 3) {
    console.log("Selector canvas 64x64");
    canvas_size = 64;
  }
  //get the parameters of the canvas and draw the grid
  var canvas = document.getElementById("canvas");
  var size = canvas.clientWidth;
  canvas.height = size;
  canvas.width = size;
  drawGrid(0, 0);
  changeTool("0");

  //create a 2 dimensional array to store the current pattern
  for (var i = 0; i < canvas_size; i++) {
    currentPattern[i] = new Array(canvas_size);
  }
  for (var i = 0; i < canvas_size; i++) {
    for (var j = 0; j < canvas_size; j++) {
      currentPattern[i][j] = "#000000";
    }
  }
  var xhr = new XMLHttpRequest();
  xhr.open("POST", "changecanvas", true);
  xhr.setRequestHeader(
    "Content-Type",
    "application/x-www-form-urlencoded; charset=UTF-8"
  );
  xhr.send("canvas=" + canvas_size);
}
// Aca cuando leen un file, acomodo al tamaño y lo adopto para seguir operando
// también le aviso al ESP
function redrawcanvasload() {
  console.log("Re Draw Canvas ON LOAD of file ");

  //get the parameters of the canvas and draw the grid
  var canvas = document.getElementById("canvas");
  var size = canvas.clientWidth;
  canvas.height = size;
  canvas.width = size;
  drawGrid(0, 0);
  changeTool("0");

  //create a 2 dimensional array to store the current pattern
  for (var i = 0; i < canvas_size; i++) {
    currentPattern[i] = new Array(canvas_size);
  }
  for (var i = 0; i < canvas_size; i++) {
    for (var j = 0; j < canvas_size; j++) {
      currentPattern[i][j] = "#000000";
    }
  }
  // No lee aviso nada a la pantalla. Solita se ajustará su canvas size por el tamaño de archivo

  // avisarle a la pantalla de cuanto es el canvas!
  // var xhr = new XMLHttpRequest();
  //     xhr.open("POST", "changecanvas", true);
  //     xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
  //     xhr.send("canvas="+canvas_size);
}
