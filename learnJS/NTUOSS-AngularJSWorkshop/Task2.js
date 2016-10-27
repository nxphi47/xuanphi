// Write your code here.
var app = angular.module('app', []);
app.controller('mainCtrl', function () {
	var self = this;
	self.message = "hello NTU";
	self.updateMessage = function (newMessage) {
		self.message = newMessage;
	}
});