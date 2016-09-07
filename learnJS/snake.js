/**
 * Created by phi on 14/06/16.
 */

var squareWidth = 480;
var positionWidth = 16;
var snakeWidth = squareWidth / positionWidth;


var controlModule = {
	activated: false,
	mainSnake: snake,
	init: function () {
		this.controlDIV = $("<div id='control'></div>");
		$(this.controlDIV).html("");
		// controll will contain:
		/*///////////////////////////////////////////////////////////
		 greeding and instroduction heading
		 text field of score:
		 speed range: 50-----[]---100
		 start button
		 stop button
		 pause button
		 snake color: red, blue, yellow
		 food color: white, green, orange
		 background color:
		 $$$$$future feature: highscore, get from server, user ID
		 *///////////////////////////////////////////////////////////
		var controlTable = $("<table id='controlTable'></table>");
		$(controlTable).html("<caption>SNAKE VER 1</caption>" +
			"<tr>" +
			"<td class='controlLabel'>SCORES</td>" +
			"<td class='controlVal' id='score'>1</td> " +
			"</tr>" +
			"<tr>" +
			"<td class='controlLabel'>SPEED</td>" +
			"<td class='controlVal' id='speed'>50" +
			"<input id='inputSpeed' type='range' min='50' max='500' step='50' value='100' " +
			"onchange='controlModule.setSpeed()'>500</td> " +
			"</tr>" +
			"<tr>" +
			"<td class='controlLabel'>snake color</td>" +
			"<td class='controlVal' id='snakeColor'>" +
			"<form id='snakeColorRadio''>" +
			"<input name='snakeColorOpt' type='radio' value='red' checked> Red " +
			"<input name='snakeColorOpt' type='radio' value='blue'> Blue " +
			"<input name='snakeColorOpt' type='radio' value='yellow'> Yellow " +
			"</form></td>" +
			"</tr>" +
			"<tr>" +
			"<td class='controlLabel'>Food Color</td>" +
			"<td class='controlVal' id='foodColor'><form id='foodColorRadio'>" +
			"<input name='foodColorOpt' type='radio' value='white' checked> White " +
			"<input name='foodColorOpt' type='radio' value='green'> Green " +
			"<input name='foodColorOpt' type='radio' value='orange'> Orange " +
			"</form></td>" +
			"<tr><td colspan='3' style='width: 100%; margin: auto'>" +
			"<button class='controlLabel' id='startButton'>START</button>" +
			"<button class='controlLabel' id='stopButton'>STOP</button>" +
			"<button class='controlLabel' id='pauseButton'>PAUSE</button>" +
			"</td></tr>" +
			"</tr>");
		// adding the table to div
		$(this.controlDIV).append(controlTable);

		this.activated = true;
		return this.controlDIV;
	},

	configControl: function () {
		$('document').ready(function () {
			$('#startButton').click(snake.runLoop);
			$('#stopButton').click(snake.stopLoop);
		});
		$("[name='snakeColorOpt']").on("click change", function () {
			controlModule.setControlColor(true);
		});
		$("[name='foodColorOpt']").on("click change", function () {
			controlModule.setControlColor(false);
		});

		this.setSpeed();

		this.setControlColor(true);
		this.setControlColor(false);
	},

	setSpeed: function () {
		snake.speed = parseInt($("#inputSpeed").val());
		console.log("Speed: " + snake.speed);
	},

	setControlColor: function (isSnake) {
		if (isSnake) {
			var colorOption = document.getElementsByName('snakeColorOpt');
			for (var i = 0; i < colorOption.length; i++) {
				if (colorOption[i].checked == true) {
					snake.updateColor(colorOption[i].value);
					//console.log("Color snake set: " + colorOption[i].value);
				}
			}
		}
		else {
			// it the food color
			var colorOption = document.getElementsByName('foodColorOpt');
			for (var i = 0; i < colorOption.length; i++) {
				if (colorOption[i].checked == true) {
					food.updateColor(colorOption[i].value);
					//console.log("Color snake set: " + colorOption[i].value);
				}
			}
		}
	},

};

// the snake implement the snake and its method accordining to event of keys
// eating and growing of food, use getfood(#food)
var snake = {
	len: 2,
	color: "white",
	list: [],
	direction: "right",
	directQueue: [],
	speed: 100,
	runningStatus: false,
	init: function () {
		this.div = document.getElementById("snakeBoard");

		// create initial snake with 2 Dot
		this.list.unshift(new Dot(0, 0, this.color, this.div));
		this.list.unshift(new Dot(1, 0, this.color, this.div));


		// update the state of the snake
		this.updateState();
		this.keyHandle();
		console.log("Snake init: x=" + this.pos.x + " y=" + this.pos.y);
	},

	reset: function () {
		this.len = 2;
		this.color = 'red';
		this.list = [];
		this.direction = "right";
		this.directQueue = [];
		this.speed = 100;
		this.runningStatus = false;

		if (typeof this.div != "undefined"){
			while (this.div.hasChildNodes()){
				this.div.removeChild(this.div.lastChild);
			}
		}
	},

	updateState: function () {
		this.pos = {
			x: this.list[0].x,
			y: this.list[0].y,
			x2nd: this.list[1].x,
			y2nd: this.list[1].y
		};
		this.len = this.list.length;

		//update the control bar
		$('#score').text(this.len);
	},

	updateColor: function (color) {
		this.color = color;
		for (var i = 0; i < this.list.length; i++) {
			this.list[i].setColor(this.color);
		}
	},

	checkValidPos: function (x, y, whenMove) {
		var lengthCheck;
		if (whenMove) {
			lengthCheck = this.list.length - 1;
		}
		else {
			lengthCheck = this.list.length;
		}
		for (var i = 0; i < lengthCheck; i++) {
			var part = this.list[i];
			if (part.x == x && part.y == y) {
				console.log("Invalid new position");
				return false;
			}
		}
		return true;
	},

	// moveTo function use argument of actual new position to moveDot
	moveTo: function (x, y) {
		if ((x == this.pos.x && y == this.pos.y) || (x == this.pos.x2nd && y == this.pos.y2nd)) {
			// it go to the same or previous position, do nothting
			//console.log("MoveTo: " + x + " & " + y + "do nothing");
			return true;
		}
		else if (!this.checkValidPos(x, y, true)) {
			//if not valid then return false, cannot moveDot that
			//console.log("MoveTo: " + x + " & " + y + "not valid");
			return false;
		}
		else if (x == food.pos.x && y == food.pos.y) {
			this.eat();
			return true;
		}
		else {
			// moveDot to new position
			var lastNode = this.list.pop();
			lastNode.moveDot(x, y);
			this.list.unshift(lastNode);
			this.updateState();
			return true;
		}
	},
	// real move method
	move: function (direction) {
		// up, down, left, right
		var successful;
		switch (direction) {
			case "up":
				var yNew = (this.pos.y - 1 < 0) ? (positionWidth - 1) : (this.pos.y - 1);
				successful = this.moveTo(this.pos.x, yNew);
				break;
			case "right":
				var xNew = (this.pos.x + 1 == positionWidth) ? 0 : (this.pos.x + 1);
				successful = this.moveTo(xNew, this.pos.y);
				break;
			case "down":
				var yNew = (this.pos.y + 1 == positionWidth) ? 0 : (this.pos.y + 1);
				successful = this.moveTo(this.pos.x, yNew);
				break;
			case "left":
				var xNew = (this.pos.x - 1 < 0) ? (positionWidth - 1) : (this.pos.x - 1);
				successful = this.moveTo(xNew, this.pos.y);
				break;
			default:
				console.log("move: problem with direction");
				successful = false;
		}
		return successful;
	},

	// eating method, to be attach to moveTo method
	eat: function () {
		try {
			this.list.unshift(new Dot(food.pos.x, food.pos.y, this.color, this.div));
			this.updateState();
			food.autoGenerate();
		}
		catch (e) {
			console.error("Error when eating");
		}
	},

	// Key event handler, to be attach to the this.div
	keyHandle: function () {
		var board = document.getElementById("content");
		if (board == "undefined") {
			console.error("KeyHandle: error, board undefined");
			return;
		}
		board.addEventListener('keyup', function (e) {
			var key = getKey(e);
			if (key != "none" && checkOposite(snake.directQueue[-1], key)) {
				snake.directQueue.push(key);
				console.log("Keys: " + snake.directQueue);
			}
		})
	},
	runLoop: function () {
		snake.loop = window.setInterval(snake.run, snake.speed);
		snake.runningStatus = true;
	},

	// to be repeated
	run: function () {
		var direction;
		while (snake.directQueue.length > 0) {
			var x = snake.directQueue.shift();
			console.log("x = " + x);
			if (checkOposite(snake.direction, x)) {
				snake.direction = x;
				break;
			}
		}

		var fail = snake.move(snake.direction);
		if (fail == false) {
			console.log("stop");
			snake.stopLoop();

			// update the control bar
			$('#score').html($('#score').html() + " Failed!");
		}
		//food.autoGenerate();
	},

	stopLoop: function () {
		if (snake.runningStatus) {
			window.clearInterval(snake.loop);
			snake.runningStatus = false;
		}
		snake.runningStatus = false;
	}

};

var food = {
	color: "white",
	pos: {},
	init: function () {
		this.color = "white";
		this.pos = {};
		this.div = document.getElementById("snakeBoard");
		this.autoGenerate();
	},
	updateColor: function (color) {
		this.color = color;
		if (typeof this.dot == 'undefined'){
			console.error("Food is set color before initialized");
			return;
		}
		this.dot.setColor(this.color);
	},
	autoGenerate: function () {
		do {
			this.pos.x = Math.floor(Math.random() * (positionWidth));
			this.pos.y = Math.floor(Math.random() * (positionWidth));
		} while (!snake.checkValidPos(this.pos.x, this.pos.y, false));
		if (typeof this.dot == "undefined") {
			this.dot = new Dot(this.pos.x, this.pos.y, this.color, this.div);
		}
		this.dot.moveDot(this.pos.x, this.pos.y);
	}

};

// Dot object to manipulate the position of each point of the snake and food
function Dot(x, y, color, container) {
	// x,y is constraint by positionWidth
	this.x = x;
	this.y = y;
	this.color = color;
	this.container = container;
	this.object = $("<div></div>");

	$(this.container).append(this.object);


	// method for Dot
	this.moveDot = function (x, y) {
		this.x = x;
		this.y = y;
		$(this.object).css({
			"left": this.x * snakeWidth + "px",
			"top": this.y * snakeWidth + "px"
		});
	};
	this.setColor = function (color) {
		this.color = color;
		this.setCSS();
	};
	this.setCSS = function () {
		$(this.object).css({
			"width": snakeWidth + "px",
			"height": snakeWidth + "px",
			"background": this.color,
			"position": "absolute",
			"left": this.x * snakeWidth + "px",
			"top": this.y * snakeWidth + "px"
		});
	};

	// excute of function
	this.setCSS();
}

function getKey(event) {
	var key = event.keyCode;
	switch (key) {
		case 37:
		case 65:
			return "left";
		case 38:
		case 87:
			return "up";
		case 39:
		case 68:
			return "right";
		case 40:
		case 83:
			return "down";
		default:
			return "none";
	}
}

function checkOposite(key1, key2) {
	if (key1 == "left" && key2 == "right") {
		return false;
	}
	else if (key1 == "right" && key2 == "left") {
		return false;
	}
	else if (key1 == "up" && key2 == "down") {
		return false;
	}
	else return !(key1 == "down" && key2 == "up");
}