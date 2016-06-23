/**
 * Created by phi on 21/06/16.
 */

var chat = {
	// generate the UI on div content
	initUI: function () {
		this.division = document.getElementById("content");
		this.rowLayout = [];
		for (var i = 0; i < 4; i++){
			this.rowLayout.push($("<div class='rowLayout'></div>"))
		}
		this.contentBox = $("<div id='chatContentBox'></div>");
		this.controlBox = $("<div id='chatControlBox'></div>");


		// username text field and connection buttons
		this.usernameTextField = $("<input id='username' name='client' class='chatContent' " +
			"type='text' placeholder='Enter your name' required>");

		this.connectButton = $("<button id='connectButton' class='controlButton' name='buttons'>Connect</button>");
		$(this.rowLayout[0]).append(this.usernameTextField).append(this.connectButton);

		// target text field and OK buttons(indicate connected to the target)
		this.targetTextField = $("<input id='target' name='client' class='chatContent' " +
			"type='text' placeholder='Enter your friend' required>");

		this.OKButton = $("<button id='OKButton' class='controlButton' name='buttons'>OK</button>");
		$(this.rowLayout[1]).append(this.targetTextField).append(this.OKButton);

		// conversation text area and control board
		this.convTextArea = $("<textarea id='conversation' name='chatConver' class='chatContent'" +
			" placeholder='conversation...' rows='20'></textarea>");
		$(this.convTextArea).attr({
			disabled: "true",
			value: ""
		});
		$(this.convTextArea).val("");

		this.controlBoard = $("<div id='controlBoard' class='controlBoard'></div>");
		this.resetButton = $("<button id='resetButton' class='controlButton' name='buttons'>Reset</button>");
		this.controlBoard.append(this.resetButton);

		$(this.rowLayout[2]).append(this.convTextArea).append(this.controlBoard);

		// message text field and send button
		this.messTextField = $("<input id='message' name='client' class='chatContent' " +
			"type='text' placeholder='Enter your message'>");

		// send Button
		this.sendButton = $("<button id='sendButton' class='controlButton' name='sendButton'>Send</button>");

		$(this.rowLayout[3]).append(this.messTextField).append(this.sendButton);

		////////// add the boxes to division
		for (var i = 0; i < this.rowLayout.length; i++){
			$(this.division).append(this.rowLayout[i]);
		}
	},

	init: function () {
		this.initUI();
		$(this.connectButton).click(function () {
			chat.handleConnect();
			return false;
		});
		$(this.usernameTextField).keyup(function (event) {
			if (event.keyCode == 13){
				$(chat.connectButton).click();
			}
			return false;
		});

		$(this.sendButton).click(function () {
			chat.handleSend();
			return false;
		});
		$(this.messTextField).keyup(function (event) {
			if (event.keyCode == 13){
				$(chat.sendButton).click();
			}
			return false;
		});
		this.updateUIState("dis"); // changing from dis, ing, ed
		this.username = "";
		this.targetname = "";
		this.message = ""; // to be keep updated on messtextfield or "" instead

	},

	// set of function with the buttons, execute one of the following click and enter handling
	handleConnect: function () {
		switch (this.connectiontState){
			case "dis":
				// start to connect
				if ($(this.usernameTextField).val() == ""){
					alert("Please enter your username !");
					return;
				}
				this.updateUIState('ing');
				this.loopUpdate = window.setTimeout(chat.updateCGI, 100);
				this.updateUIState('ed');
				break;
			case 'ed':
				// start to disconnect
				this.updateUIState('dis');
				window.clearInterval(this.loopUpdate);
				break;
			default:
				console.log("Failed to handleConnect")
		}
	},
	updateUIState: function (state) {
		switch (state){
			case "dis":
				this.connectiontState = "dis";
				$(this.connectButton).html("Connect");
				$(this.usernameTextField).prop('disabled', false);
				// do CSS stuff here
				$(this.connectButton).attr({
					name: "connectButton",
				});
				break;
			case "ing":
				this.connectiontState = "ing";
				$(this.connectButton).html("Connecting..");
				$(this.usernameTextField).attr({
					disabled: ""
				});
				// do CSS stuff here
				break;
			case "ed":
				this.connectiontState = "ed";
				$(this.connectButton).html("Disconnect");
				$(this.usernameTextField).prop('disabled', true);
				// do CSS stuff here
				$(this.connectButton).attr({
					name: "disconnectButton",
				});

				break;
			default:
				console.log("Chat:UpdateConnectionState: failed to update %s\n", state);
				break;
		}
	},
	updateCGI: function () {
		if (!chat.updatePara()){
			return;
		}
		var _data = {
			username: chat.username,
			target: chat.targetname,
			message: chat.message
		};
		$.ajax({
			type: "GET",
			url: "/cgi/cgi_handle_chat.py",
			data: _data,
			dataType: "text",
			success: function (response) {
				// do somthing on the response
				if (response != "" && response != "\n") {
					if (response.indexOf("Chat//") < 0) {
						chat.updateConversation(response);
					}
					else {
						console.log("CGI error: " + response);
					}
				}
				else {
					console.log("receive nothing");
				}
				if (chat.connectiontState != "dis"){
					window.setTimeout(chat.updateCGI, 100);
				}
				return false;
			},
			error: function (request, error) {
				console.log("Ajax_error: " + error);
				return false;
			}
			
		})
	},

	handleSend: function () {
		if (!chat.updatePara()){
			return;
		}
		if ($(chat.messTextField).val() == ""){
			// do nothing if there is nothing in the text field
			return;
		}
		var _data = {
			username: chat.username,
			target: chat.targetname,
			message: $(chat.messTextField).val()
		};
		$.ajax({
			type: "GET",
			url: "/cgi/cgi_handle_chat.py",
			data: _data,
			dataType: "text",
			success: function (response) {
				// do somthing on the response
				chat.updateConversation(chat.username + ": " + _data['message']);
				if (response != "" && response != "\n") {
					if (response.indexOf("Chat//") < 0) {
						console.log("Send successfully");
						chat.updateConversation(response);
					}
					else {
						console.log("CGI error: " + response);
					}
				}
				else {
					console.log("receive nothing");
				}
				return false;
			},
			error: function (request, error) {
				alert("Ajax_error: " + request);
				return false;
			}
		});
		$(chat.messTextField).val("");
	},
	updatePara: function () {
		chat.username = $(chat.usernameTextField).val();
		chat.targetname = $(chat.targetTextField).val();

		if (chat.username == ""){
			console.log("Username empty when updatePara");
			return false;
		}
		else
			return true;
	},
	updateConversation: function (mes) {
		var start = 0;
		var end = mes.length -1;
		while (mes[start] == "\n" || mes[end] == "\n"){
			if (mes[start] == "\n")
				start += 1;
			if (mes[end] == "\n")
				end -= 1;
		}
		mes = mes.substring(start, end + 1);
		$(this.convTextArea).val($(this.convTextArea).val() + mes + "\n");
		$(this.convTextArea).scrollTop($(this.convTextArea)[0].scrollHeight);
	}
};