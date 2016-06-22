/**
 * Created by nxphi on 06/06/16.
 */

var changeBackground = {
	imageHover: function (image) {
		image.style.border = "inset";
		//.style.margin= "0px";
	},
	imageOutHover: function (image) {
		image.style.border = "outset";
		//image.style.margin = "5px";
	},
	imageChange: function (image) {
		console.log(image.src);
		var style = document.body.style;
		style.background = "url('" + image.src + "') no-repeat center center fixed";
		style.mozBackgroundSize = "cover";
		style.webkitBackgroundSize = "cover";
		style.oBackgroundSize = "cover";
		style.backgroundSize = "cover";

	}
};

var changeContent = {
	mode: 'none',
	slideSpeed: 200,
	personalInit: function () {
		var division = document.getElementById("content");
		this.activeOption("personal");

		// slide up the content if it has child node
		if (this.animateSlideUp("personal")){
			return;
		}

		// remove element
		$(division).html("");
		// the making of div //////////////////////////////////////////////////////
		function makeBr() {
			return document.createElement("br");
		}

		// form making
		var form = document.createElement("form");
		form.style.backgroundColor = "white";
		form.action="/cgi/cgi_handle_personal.py";
		form.method = "POST";
		form.target = "_blank";
		form.enctype = "multipart/form-data";

		//personal fieldset
		var personalFieldset = document.createElement("fieldset");
		var legend = document.createElement("legend");
		legend.innerHTML = "Basic information";

		var firstText = document.createTextNode("First name: ");
		var first = document.createElement("input");
		first.type = "text";
		first.required = "required";
		first.placeholder = "Xuan Phi";
		first.name = "fname";

		var lastText = document.createTextNode("Last name: ");
		var last = document.createElement("input");
		last.type = "text";
		last.required = "required";
		last.placeholder = "Nguyen";
		last.name = "lname";

		var agetext = document.createTextNode("your age: ");
		var age = document.createElement("input");
		age.type = "number";
		age.required = "required";
		age.defaultValue = "20";
		age.min = "1";
		age.max = "100";
		age.name = "age";

		var countryText = document.createTextNode("Nationality: ");
		var country = document.createElement("input");
		country.type = "text";
		country.required = "required";
		country.pattern = "[A-Z]{3}";
		country.placeholder = "VNM";
		country.name = "country";

		var gender = $("<form></form>").html("Gender: <input type='radio' name='gender' value='male' checked>" +
			" Male <input type='radio' name='gender' value='female'> Female ");

		personalFieldset.appendChild(firstText);
		personalFieldset.appendChild(first);
		personalFieldset.appendChild(makeBr());
		personalFieldset.appendChild(lastText);
		personalFieldset.appendChild(last);
		personalFieldset.appendChild(makeBr());
		personalFieldset.appendChild(agetext);
		personalFieldset.appendChild(age);
		personalFieldset.appendChild(makeBr());
		personalFieldset.appendChild(countryText);
		personalFieldset.appendChild(country);
		personalFieldset.appendChild(makeBr());
		$(personalFieldset).append(gender);
		personalFieldset.appendChild(legend);
		// finish personal fieldset

		// education information fieldset
		var eduFieldset = $("<fieldset id='edu'></fieldset>").html("<legend>Education</legend>" +
			"High School: <input name='highschool' type='text' placeholder='Phan Chau trinh'><br>" +
			"University: <input name='uni' type='text' placeholder='NTU'><br>" +
			"Major: " +
			"<select>" +
			"<option value='EEE'>EEE</option>" +
			"<option value='SCE'>SCE</option>" +
			"<option value='MASS'>MASS</option>" +
			" </select> <br>" +
			"<form oninput='outGPA.value=parseFloat(gpa.value)'>" +
			"CGPA: <input name='GPA' type='range' min='0' max='5' step='0.1' value='3.5' id='gpa'> = " +
			"<output name='outGPA' for='gpa' ></output>" +
			"</form>" +
			"Upload your CV: <input type='file' name='filename'>");
		
		// submit button
		var submit = document.createElement("input");
		submit.type = "submit";
		submit.value = "Submit";

		form.appendChild(personalFieldset);
		$(form).append(eduFieldset);
		form.appendChild(submit);
		// finish for making
		division.appendChild(form);
		// done the making of div//////////////////////////////////////////////////
		// animation
		$(division).slideDown(this.slideSpeed);
	},

	// the snake game
	snakeInit: function () {
		var division = document.getElementById("content");
		this.activeOption("snake");
		// slide up animation if the content have childnodes
		if (this.animateSlideUp("snake")){
			return;
		}

		// remove all child nodes
		$(division).html("");
		division.tabIndex = '0';

		// creation of div content for game -----------------------------------------
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
		food.init();
		controlModule.configControl();

		// done creation of content --------------------------------------

		// slidedown animation
		$(division).slideDown(this.slideSpeed);
	},

	backgroundInit: function () {
		var division = document.getElementById("content");
		this.activeOption("background");
		// slide up animation if the content have childnodes
		if (this.animateSlideUp("background")){
			return;
		}
		// remove all child nodes
		$(division).html("");
		division.tabIndex = '0';

		// creating content -------------------------------------------------
		$(division).html("<fieldset id='changeBack'>" +
			"<legend>Change the Background</legend>" +
			"<img src='background.jpg' class='backgroundImg' onclick='changeBackground.imageChange(this)'>" +
			"<img src='background2.png' class='backgroundImg' onclick='changeBackground.imageChange(this)'>" +
			"<img src='background3.jpg' class='backgroundImg' onclick='changeBackground.imageChange(this)'>" +
			"</fieldset>");

		// Done creating content --------------------------------------------

		// slidedown animation
		$(division).slideDown(this.slideSpeed);
	},
	
	chatInit: function () {
		var division = document.getElementById("content");
		this.activeOption("chat");
		// slide up animation
		if (this.animateSlideUp("chat")){
			return;
		}
		$(division).html("");
		
		// creating content for chat ---------------------------
		chat.init();
		// Done creating content for chat-----------------------
		
		$(division).slideDown(this.slideSpeed);
	},

	statusInit: {},

	animateSlideUp: function (mode) {
		// return true if the init should return afterthis or false if just continu
		var division = document.getElementById("content");
		if (this.mode == mode) {
			// do nothing on init
			return false;
		}
		else if (this.mode != 'none') {
			$(division).slideUp(changeContent.slideSpeed);
			// need to wait for the sildeup finish
			window.setTimeout(function () {
				changeContent.mode = 'none';
				//changeContent.personalInit();
				eval("changeContent." + mode + "Init();");
			}, changeContent.slideSpeed);
			return true;
		}
		else {
			$(division).css("display", "none");
		}
		this.mode = mode;
		console.log("Mode: " + this.mode);
		return false;
	},

	activeOption: function (option) {
		var opt = document.getElementsByName("option");
		for (var i = 0; i < opt.length; i++){
			opt[i].className = "option";
		}
		document.getElementById(option).className = "active";
	}
};

function curbChangeColor(text) {
	var style = text.style;
	if (text.innerHTML == " 5 cm") {
		style.backgroundColor = "green";
		style.color = "black";
	}
	else if (text.innerHTML == "10 cm") {
		style.backgroundColor = "yellow";
		style.color = "black";
	}
	else if (text.innerHTML == "15 cm") {
		style.backgroundColor = "red";
		style.color = "white";
	}
	else if (text.innerHTML == "none") {
		style.backgroundColor = "lightblue";
		style.color = "black";
	}
	else if (text.innerHTML == "unknown") {
		style.backgroundColor = "saddlebrown";
		style.color = "white";
	}
	else {
		console.log("Error with curb input when change style");
		console.log(text.innerHTML);
	}
}

function curbChangeValue(text) {
	var val = parseInt(text.innerHTML.substring(0, text.innerHTML.indexOf('c')));
	var newVal = val;
	if (val == 15) {
		newVal = 5;
	}
	else {
		newVal += 5;
	}
	text.innerHTML = newVal + " cm";
	curbChangeColor(text);
}

var realtime = {
	activated: false,
	init: function () {
		this.div = document.getElementById("display");
		this.table = document.createElement("table");
		this.table.align = "center";

		var caption = document.createElement("caption");
		caption.id = "caption";
		caption.innerHTML = "RealTime demo";
		caption.style.fontSize = "200%";
		caption.style.backgroundColor = "#210ebd";
		caption.style.color = "white";

		// statusRow row
		this.statusRow = document.createElement("tr");
		this.statusRow.id = "statusRow";
		var statusLabel = document.createElement("td");
		statusLabel.className = "label";
		statusLabel.innerHTML = "Status";
		this.statusRow.appendChild(statusLabel);
		this.statusVal = document.createElement("td");
		this.statusVal.className = "value";
		this.statusVal.innerHTML = "Burst";
		this.statusRow.appendChild(this.statusVal);

		// Curb length row
		this.curbRow = document.createElement("tr");
		this.curbRow.id = "curbRow";
		var curbLabel = document.createElement("td");
		curbLabel.className = "label";
		curbLabel.innerHTML = "Curb";
		this.curbRow.appendChild(curbLabel);
		this.curbVal = document.createElement("td");
		this.curbVal.className = "value";
		this.curbVal.innerHTML = " 5 cm"; //  change according to....
		// need to add the default style and change of style
		//noinspection JSValidateTypes
		this.curbRow.appendChild(this.curbVal);


		// distance row
		this.distRow = document.createElement("tr");
		this.distRow.id = "distRow";
		var distLabel = document.createElement("td");
		distLabel.className = "label";
		distLabel.innerHTML = "Distance";
		this.distRow.appendChild(distLabel);
		this.distVal = document.createElement("td");
		this.distVal.className = "value";
		this.distVal.innerHTML = "4 m";
		// need to add the changing or value
		this.distRow.appendChild(this.distVal);

		// logTd row
		var logRow = document.createElement("tr");
		var logLabel = document.createElement("td");
		logLabel.className = "label";
		logLabel.colSpan = "2";
		logLabel.innerHTML = "Log";
		logLabel.align = "center";
		logRow.appendChild(logLabel);

		// logTd val
		this.logValRow = document.createElement("tr");
		this.logTd = document.createElement("td");
		this.logTd.className = "logTd";
		this.logTd.colSpan = "2";
		this.log = document.createElement("textarea");
		this.log.className = "log";
		this.log.value = "Log testing\n";

		this.log.rows = "10";
		this.log.cols = "60";
		this.log.disabled = "disabled";
		this.log.style.resize = "none";

		this.logTd.appendChild(this.log);
		this.logValRow.appendChild(this.logTd);


		// add the the table
		this.table.appendChild(caption);
		this.table.appendChild(this.statusRow);
		this.table.appendChild(this.curbRow);
		this.table.appendChild(this.distRow);
		this.table.appendChild(logRow);
		this.table.appendChild(this.logValRow);

		//add to the div
		this.div.appendChild(this.table);
		this.addStyle();
		this.activated = true;
	},

	addStyle: function () {
		var table = document.getElementsByTagName("table")[0];// since we have 2 table
		table.style.border = "1px solid black";
		table.style.borderCollapse = "collapse";
		table.style.fontSize = "150%";
		table.style.width = "500px";

		var td = document.getElementsByTagName("td");
		for (var i = 0; i < td.length; i++) {
			td.item(i).style.border = "1px solid black";
			td.item(i).style.borderCollapse = "collapse";
			td.item(i).style.fontSize = "150%";
		}

		var tdLabel = document.getElementsByClassName("label");
		for (var i = 0; i < tdLabel.length; i++) {
			tdLabel.item(i).style.backgroundColor = "#210ebd";
			tdLabel.item(i).style.color = "aliceblue";
		}
		var tdValue = document.getElementsByClassName("value");
		for (var i = 0; i < tdValue.length; i++) {
			tdValue.item(i).style.backgroundColor = "white";
			tdValue.item(i).style.color = "black";
			tdValue.item(i).style.textAlign = "center";
		}
		/*
		 var log = document.getElementsByClassName("log");
		 log.item(0).style.margin = "0";
		 log.item(0).style.fontSize = "75%";
		 log.item(0).style.backgroundColor = "white";
		 */
	},

	updateStatus: function (burstOrNot) {
		if (burstOrNot) {
			this.statusVal.innerHTML = "Burst";
		}
		else {
			this.statusVal.innerHTML = "Measurement";
		}
	},

	updateCurb: function (val) {
		switch (val) {
			case " 5":
			case "10":
			case "15":
				this.curbVal.innerHTML = val + " cm";
				break;
			case "none":
			case "unknown":
				this.curbVal.innerHTML = val;
				break;
			default:
				this.curbVal.innerHTML = "Error";
		}
		curbChangeColor(this.curbVal);
	},

	updateDist: function (num) {
		this.distVal.innerHTML = num + " m";
	},

	updateLog: function (info) {
		this.log.value += info + "\n";
	},

	showUpdateRealTime: function (burst, curbVal, distVal) {
		// -- need to check if realtime mode activated or not
		if (this.activated == false) {
			console.log("Error realtime mode not activated at show update");
		}

		//need to update the status curb, distance and log
		var statusBurst;// true or false
		var curb;// string
		var dist;

		statusBurst = burst;
		//curb = curbVal;
		if (curbVal == "5") {
			curb = " 5";
		}
		else curb = curbVal;
		console.log(curb);
		dist = distVal;
		// code for update the status
		realtime.updateStatus(statusBurst);
		realtime.updateCurb(curb);
		realtime.updateDist(dist);

		// log output
		var time = new Date();
		var log = time.toTimeString().substring(0, 8) + "--";
		if (curb == "none") {
			log += "found nothing";
		}
		else if (curb == "unknown") {
			log += "found unknown at " + dist + " m";
		}
		else {
			//when found curb
			log += "found curb " + curb + " at " + dist + " m";
		}
		realtime.updateLog(log)
	}
};
