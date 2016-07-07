// Whether to use AJAX or not
var DEF_AJAX = true;
var USE_BTN = false;

function Sensor (w, h, id) {
	this.cnvW = w;
	this.cnvH = h;
	this.id = id;
	this.data = [];
	this.bkgd = null;
	this.initHTML = function () {
		return "<table border=0 cellspacing=2 cellpadding=2>" +
			"<tr><td class='label' id='" + id + "-label'>Sensor " + id +
			"</td></tr><tr><td class='graph' id='" + id + "-td'>" +
			"<canvas id='" + id + "-cnv' width='" + this.cnvW + "' " +
			"height='" + this.cnvH + "'/></td></tr></table>";
	};
	this.initCanvas = function () {
		this.ctx = document.getElementById(this.id + "-cnv").getContext("2d");
		this.ctx.fillStyle = "rgb(10, 10, 10)";
		this.ctx.fillRect (0, 0, this.cnvW, this.cnvH);
		this.ctx.strokeStyle = "rgb(255, 255, 255)";
		this.ctx.strokeRect (0, 0, this.cnvW, this.cnvH);	// outer rectangle
		this.ctx.fillStyle = "rgb(192, 192, 192)";
		this.bkgd = this.ctx.getImageData (0, 0, this.cnvW, this.cnvH);
		this.draw();
	};
	this.draw = function () {
		if (this.bkgd == null)
			this.initCanvas();
		else this.ctx.putImageData (this.bkgd, 0, 0);
		if (this.data.length <= 2)
			return;
		// init mapping
		var yofs = this.cnvH;
		var yscale = -1.0*this.cnvH/256.0;
		var xscale = 1.0*this.cnvW/this.data.length;
		var x, i;
		// start drawing the curve
		this.ctx.beginPath();
		this.ctx.moveTo (0, this.data[0]*yscale + yofs);
		for (x=0, i=0; i<this.data.length; i++, x+=xscale)
			this.ctx.lineTo (x, yofs + this.data[i] * yscale);
		this.ctx.strokeStyle = "rgb(60, 255, 60)";
		this.ctx.lineWidth = 2;
		this.ctx.stroke();
	};
	this.update = function (data) { this.data = data; this.draw(); };
}

function update() {
	// Call AJAX for data update
	$.ajax(
	{
		type: "POST",
		url: "/cgi/handle-ajax.py",
		data: "PARAM=UPDATE",
		dataType: "text",
		success: function(response) {
			eval(response);
			return false;
		}
	});	
}

var sensors = [];
sensors[0] = new Sensor(600, 400, 0);
sensors[1] = new Sensor(600, 400, 1);
sensors[2] = new Sensor(600, 400, 2);
sensors[3] = new Sensor(600, 400, 3);

function init() {
	// --- initialize display div ---
	$('#display-div').height($(window).height() - 64);
	if (USE_BTN)
		$('#button-div').height($(window).height() - 64);
	else $('#display-div').css('margin-left', 0);

	$('#display-div').html(
		'<p align="center"><table border=0 cellspacing=2 cellpadding=2>' +
		'<tr><td>' + sensors[0].initHTML() + '</td><td>' + sensors[1].initHTML() + '</td></tr>' +
		'<tr><td>' + sensors[2].initHTML() + '</td><td>' + sensors[3].initHTML() + '</td></tr>' +
		'</table></br></br><a href="javascript:update()"> UPDATE </a></br></br>'
	);
	for (var i=0; i<sensors.length; i++)
		sensors[i].initCanvas();
}


var mockupData = [];
mockupData[0] = [49, 49, 48, 47, 67, 157, 159, 64, 48, 49, 64, 127, 122, 98, 71, 80, 54, 62, 78, 47, 45, 49, 47, 49, 54, 49, 49 ];
