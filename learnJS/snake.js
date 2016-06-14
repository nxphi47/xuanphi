/**
 * Created by phi on 14/06/16.
 */

var squareWidth = 500;
var positionWidth = 10;
var snakeWidth = squareWidth / positionWidth;

function snakeInit() {
	var division = document.getElementById("content");
	while (division.hasChildNodes()) {
		division.removeChild(division.lastChild);
	}
	// board for whole game(include the gameBoard and control board
	var mainBoard = $("<table id='snakeMainBoard' style='margin: auto'></table>");
	var mainBoardRow = $("<tr id='snakeMainBoardRow' style='margin: auto'></tr>");
	// board is square of 500px, s
	var board = $("<div id='snakeBoard' tabindex='0'></div>");
	$(board).css({
		"width": squareWidth + "px",
		"height": squareWidth + "px",
		"background-color": "black",
		"margin": "auto",
		"position": "relative"
	});

	var dataBoard = $("<td></td>").append(board);
	var dataControl = $("<td></td>").append(controlModule.init());
	// add the board to division
	$(mainBoardRow).append(dataBoard);
	$(mainBoardRow).append(dataControl);
	$(mainBoard).append(mainBoardRow);
	$(division).append(mainBoard);

	snake.init();
	//snake.runLoop();
	snake.move("down");
	food.init();

	snake.runLoop();
}

var controlModule = {
	activated: false,
	mainSnake: snake,
	init: function () {
		this.controlDIV = $("<div id='control'></div>");
		// controll will contain:
		/*///////////////////////////////////////////////////////////
		 greeding and instroduction heading
		 text field of score:
		 speed range: 50-----[]---100
		 start button
		 stop button
		 pause button
		 snake color: white, blue, yellow
		 food color: red, green,
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
			"<input id='inputSpeed' type='range' min='50' max='500' step='50'>500</td> " +
			"</tr>" +
			"<tr><td colspan='3' style='width: 100%; margin: auto'>" +
			"<button class='controlLabel' id='startButton'>START</button>" +
			"<button class='controlLabel' id='stopButton'>STOP</button>" +
			"<button class='controlLabel' id='pauseButton'>PAUSE</button>" +
			"</td></tr>" +
			"<tr>" +
			"<td class='controlLabel'>snake color</td>" +
			"<td class='controlVal' id='snakeColor'>Input of color</td> " +
			"</tr>" +
			"<tr>" +
			"<td class='controlLabel'>food color</td>" +
			"<td class='controlVal' id='foodColor'>input of color</td> " +
			"</tr>");
		// adding the table to div
		$(this.controlDIV).append(controlTable);

		this.activated = true;
		return this.controlDIV;
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
	getSettings: function (color) {
		this.color = color;
		for (var part in this.list){
			part.setColor(this.color);
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
	},

	checkValidPos: function (x, y, whenMove) {
		var lengthCheck;
		if (whenMove){
			lengthCheck = this.list.length - 1;
		}
		else {
			lengthCheck = this.list.length;
		}
		for (var i = 0; i < lengthCheck; i++){
			var part = this.list[i];
			if (part.x == x && part.y == y){
				console.log("Invalid new position");
				return false;
			}
		}
		return true;
	},

	// moveTo function use argument of actual new position to moveDot
	moveTo: function (x, y) {
		if ((x == this.pos.x && y == this.pos.y) || (x == this.pos.x2nd && y == this.pos.y2nd)){
			// it go to the same or previous position, do nothting
			//console.log("MoveTo: " + x + " & " + y + "do nothing");
			return true;
		}
		else if (!this.checkValidPos(x, y, true)){
			//if not valid then return false, cannot moveDot that
			//console.log("MoveTo: " + x + " & " + y + "not valid");
			return false;
		}
		else if (x == food.pos.x && y == food.pos.y){
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
		switch (direction){
			case "up":
				var yNew = (this.pos.y - 1 < 0)?(positionWidth - 1):(this.pos.y - 1);
				successful = this.moveTo(this.pos.x, yNew);
				break;
			case "right":
				var xNew = (this.pos.x + 1 == positionWidth)?0:(this.pos.x + 1);
				successful = this.moveTo(xNew, this.pos.y);
				break;
			case "down":
				var yNew = (this.pos.y + 1 == positionWidth)?0:(this.pos.y + 1);
				successful = this.moveTo(this.pos.x, yNew);
				break;
			case "left":
				var xNew = (this.pos.x - 1 < 0)?(positionWidth - 1):(this.pos.x - 1);
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
		catch (e){
			console.error("Error when eating");
		}
	},

	// Key event handler, to be attach to the this.div
	keyHandle: function () {
		if (snake.div == "undefined"){
			console.log("KeyHandle: error, this.div undefined");
			return;
		}
		snake.div.addEventListener('keyup', function (e) {
			var key = getKey(e);
			if (key != "none" && checkOposite(snake.directQueue[-1], key)){
				snake.directQueue.push(key);
				console.log("Keys: " + snake.directQueue);
			}
		})
	},
	runLoop: function () {
		this.loop = window.setInterval(snake.run, snake.speed);
	},

	// to be repeated
	run: function () {
		var direction;
		while (snake.directQueue.length > 0){
			var x = snake.directQueue.shift();
			console.log("x = " + x);
			if (checkOposite(snake.direction, x)){
				snake.direction = x;
				break;
			}
		}

		var fail = snake.move(snake.direction);
		if (fail == false){
			console.log("stop");
			// while it is not stopped
			window.clearInterval(snake.loop);
		}
		//food.autoGenerate();
	},
	
};

var food = {
	color: "red",
	pos: {},
	init : function () {
		this.div = document.getElementById("snakeBoard");
		this.autoGenerate();
	},
	setColor: function (color) {
		this.color = color;

	},
	autoGenerate: function () {
		do {
			this.pos.x = Math.floor(Math.random() * (positionWidth));
			this.pos.y = Math.floor(Math.random() * (positionWidth));
		} while (!snake.checkValidPos(this.pos.x, this.pos.y, false));
		if (typeof this.dot == "undefined"){
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
		this.setCSS();
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
	switch (key){
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
	if (key1 == "left" && key2 == "right"){
		return false;
	}
	else if (key1 == "right" && key2 == "left"){
		return false;
	}
	else if (key1 == "up" && key2 == "down"){
		return false;
	}
	else return !(key1 == "down" && key2 == "up");
}