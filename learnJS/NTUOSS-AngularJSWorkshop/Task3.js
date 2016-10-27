var app = angular.module('app', []);

app.factory("messages", function () {
	var messages = {};
	messages.list = [];
	messages.add = function (message) {
		messages.list.push({id: messages.list.length, text: message});
	}
	return messages
})

app.controller('listCtrl', ['messages', function (messages) {
	var self = this;
	self.messages = messages.list;
}]);


app.controller('addCtrl', ['messages', function (messages) {
	var self = this;
	self.newMessage = "hello ntu";
	self.addMessage = function (message) {
		messages.add(message);
		self.newMessage = "";

	}
}])