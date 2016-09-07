// Viewer2.js
// More powerful waveform viewer
// chanwah.ng@sg.panasonic.com

// ---------------------------------------------------------------
// To Declare Object Inheritance
function inheritsFrom (child, parent) {
	child.prototype = Object.create(parent.prototype);
	child.prototype.constructor = child;
}
// ---------------------------------------------------------------

// ---------------------------------------------------------------
// Viewer Base Class
//		Same class for main viewer and sub-viewer
// ---------------------------------------------------------------

function ViewerBaseObj (div, w, h, opt) {
	"use strict";
	this.div = div;			// name of the tag containing the graph (usually a <div>, but can be <td> also
	this.w = w;				// width of the canvas
	this.h = h;				// height of the canvas
	this.opt = opt;			// default options for flot
	this.firstDraw = true;
}

// generate HTML placeholder for flot to plot the graph 
ViewerBaseObj.prototype.initHTML = function () {
	"use strict";
	$('#' + this.div).html (
		"<div id='" + this.div + "-div' style='width:" + this.w + "px;height:" + this.h + "px'></div>"
	);
	this.cnv = $('#'+this.div+'-div');		// remmeber this 'canvas' object
	this.firstDraw = true;
}

// encapsulate the data in flot required format
ViewerBaseObj.prototype.data2flot = function (data) {
	return [{ color: '#00ff00', data: data, lines: { show: true } }];
}

// draw the data on the graph.  nopt is any overriding options if specified
ViewerBaseObj.prototype.draw = function (data, nopt) {
	"use strict";
	var opt = nopt ? $.extend(true, {}, this.opt, nopt) : this.opt;
	this.plot = $.plot(this.cnv, this.data2flot(data), opt);
	if (this.firstDraw)
		this.setPlotSelection();
	this.firstDraw = false;
}

// virtual method, to be overridden
ViewerBaseObj.prototype.setPlotSelection = function () { }

// ---------------------------------------------------------------
// Main Viewer Class
//		For the main graph
// ---------------------------------------------------------------

function MainViewerObj (div, w, h, parent) {
	"use strict";
	this.parent = parent;		// parent should be the ZViewer
	this.timeDelayScale = 0;
	this.fill = false;
	// call parent constructor
	ViewerBaseObj.call (this, div || 'main-viewer', w || 800, h || 600,
		{
			legend: { 
				labelBoxBorderColor: "#fff",
			},
			series: {
				lines: {
					show: true,
				},
			},
			grid: {
				backgroundColor: "#000000",
				tickColor: "#008040",
			},
			selection: {
				mode: 'xy'
			},
		}
	);
}
inheritsFrom(MainViewerObj, ViewerBaseObj);

// setup the callback for the event when user selects an area to zoom in
MainViewerObj.prototype.setPlotSelection = function () {
	"use strict";
	var myself = this;
	this.cnv.bind("plotselected", function (event, ranges) {
		// clamp the zooming to prevent eternal zoom
		if (ranges.xaxis.to - ranges.xaxis.from < 0.00001) {
			ranges.xaxis.to = ranges.xaxis.from + 0.00001;
		}
		if (ranges.yaxis.to - ranges.yaxis.from < 0.00001) {
			ranges.yaxis.to = ranges.yaxis.from + 0.00001;
		}
		// call ZViewer's setZoom() method
		myself.parent.setZoom (ranges);
	});
}

// convert data to flot format
MainViewerObj.prototype.data2flot = function (data) {
	"use strict";
	// check if user want to show the sampling jitter
	if (this.timeDelayScale > 0) {
		var time = [ [data[0][0], 0] ];
		var ave = 0.0, d;
		// get time delay
		for (var i=1; i<data.length; i++) {
			ave += d = (data[i][0] - time[i-1][0]) * this.timeDelayScale * 1000;
			time[i] = [ data[i][0], d ];
		}
		time[0][1] = ave / data.length;
		// return the data wih jitter
		return [ { color: '#ff0000', data: time, lines: { show: true, fill: this.fill } }, 
				 { color: '#00ff00', data: data, lines: { show: true, fill: this.fill } }];
	} else {
		// return only data
		return [{ color: '#00ff00', data: data, lines: { show: true, fill: this.fill } }];
	}
}

// ---------------------------------------------------------------
// Sub Viewer Class
//		Thumbnail graph
// ---------------------------------------------------------------

function SubViewerObj (div, w, h, peer) {
	this.peer = peer;		// peer should be the main-viewer
	ViewerBaseObj.call (this, div || "sub-viewer", w || 180, h || 125,
		{
			legend: { 
				show: false,
			},
			series: {
				lines: {
					show: true,
					linewidth: 1,
				},
				shadowSize: 0,
			},
			grid: {
				backgroundColor: "#000000",
				tickColor: "#008040",
			},
			selection: {
				mode: 'xy'
			}
		}
	);
}

inheritsFrom(SubViewerObj, ViewerBaseObj);

SubViewerObj.prototype.setPlotSelection = function () {
	"use strict";
	var peer = this.peer;
	this.cnv.bind("plotselected", function (event, ranges) {
		peer.plot.setSelection(ranges);		// call Main Viewer's method
	});
}


// ---------------------------------------------------------------
// Zoomable Viewer Class
//		Essentially just a container for both MainViewer and SubViewer
// ---------------------------------------------------------------

function ZViewerObj (maincnv, subcnv, cnvw, cnvh, subw, subh) {
	"use strict";
	this.main = new MainViewerObj(maincnv, cnvw, cnvh, this);
	this.sub = new SubViewerObj(subcnv, subw, subh, this.main);
	this.main.initHTML();
	this.sub.initHTML();
	this.zoomRange = { 				// remmeber user's zoom setting so that it will persist when signal change
		xaxis: { from: 0, to: -1 },
		yaxis: { from: 0, to: -1 }	
	};
}

// Set the data to plot
ZViewerObj.prototype.setData = function (time, env) {
	"use strict";
	var len = env.length;
	if (len > time.length)
		len = time.length;
	// combine them into array of [x=time,y=env]
	this.data = [];
	for (var i=0; i<len; i++) {
		this.data[i] = [ time[i], env[i] ];
	}
	// plot
	this.sub.draw(this.data);
	this.updatePlot();
}

// If user has a zomed setting, only return the zoomed portion of the data
ZViewerObj.prototype.getZoomedData = function () {
	"use strict";
	var ret = [], ri, v;
	for (var i=ri=0; i<this.data.length; i++) {
		if (this.data[i][0] < this.zoomRange.xaxis.from)
			continue;
		if (this.data[i][0] > this.zoomRange.xaxis.to)
			break;
		ret[ri++] = this.data[i];
	}
	return (ret);
}

// set the zoom settings.  ranges=null to clear zoom setting
ZViewerObj.prototype.setZoom = function (ranges) {
	"use strict";
	if (ranges && ranges.xaxis.to < ranges.xaxis.from) {
		ranges.xaxis.from = this.data[0][0];
		ranges.xaxis.to = this.data[this.data.length-1][0];
	}
	this.zoomRange = ranges || { 	
		xaxis: { from: 0, to: -1 },
		yaxis: { from: 0, to: -1 }	
	};
	this.updatePlot();
}

// update the plot whenever some setting is changed
ZViewerObj.prototype.updatePlot = function () {
	"use strict";
	if (this.zoomRange.yaxis.from < this.zoomRange.yaxis.to) {
		this.main.draw(this.getZoomedData(), zoomRange2clip(this.zoomRange));
		this.sub.plot.setSelection(this.zoomRange, true);
	} else {
		this.main.draw(this.data);
	}
}

// Resize the main viewer
ZViewerObj.prototype.resize = function (cnvw, cnvh) {
	"use strict";
	this.main.w = cnvw;
	this.main.h = cnvh;
	this.main.initHTML();
	this.updatePlot();
}

// convenient routine to change ranges{from,to} into ranges{min,max}
function zoomRange2clip (zr) {
	return { xaxis: { min: zr.xaxis.from, max: zr.xaxis.to }, yaxis: { min: zr.yaxis.from, max: zr.yaxis.to } };
}

// ---------------------------------------------------
// Navigation contol
//		Base Control (essentially version 1) of overall
//		signal plotting for the case where there is only
//		one set of signals (from a single file)
//		We no longer use this directly, but extends it
//		with an inherited child class FileNavCtrlObj
// ---------------------------------------------------

function NavCtrlObj (name, mviewer, sviewer) {
	"use strict";
	this.myname = name || 'navctl';
	this.flags = [];
	this.viewer = new ZViewerObj(mviewer, sviewer);
	// handle UI related stuff
	var myself = this;
	this.flags['nav-prev'] = this.flags['nav-next'] = 'ctrl';
	this.flags['clip'] = this.flags['fill'] = false;
	$(document).keypress(function(evt) {			// capture the keypress event
		var k = String.fromCharCode(evt.which);
		if ('>.Nn'.indexOf(k) > -1)		// same effect as user press next
				myself.go(1);
		if ('<,Pp'.indexOf(k) > -1)		// same effect as user press prev
				myself.go(-1);
		if ('cC'.indexOf(k) > -1) {		// toggle clipping to 900-2100
			if (!myself.flags['clip']) {
				myself.viewer.setZoom({ xaxis: { from: 0, to: -1 }, yaxis: { from: 900, to: 2100 } });
				myself.flags['clip'] = true;
			} else {
				myself.viewer.setZoom();
				myself.flags['clip'] = false;
			}
		}
		if ('fF'.indexOf(k) > -1) {		// nice fill effect for area under graph
			myself.viewer.main.fill = myself.viewer.main.fill ? false : true;
			myself.viewer.updatePlot();
		}
	});
}

// set the object
//		obj should be a JS Array, where each element is the JS Object 
//		containing timing and envelop data for one sample
NavCtrlObj.prototype.setObj = function (obj) {
	"use strict";
	this.obj = obj || [];
	this.currObj = 0;
	this.go(0);
}	

// Navigate to the sample with the given offset ofs
NavCtrlObj.prototype.go = function (ofs) {
	"use strict";
	this.currObj += ofs;
	if (this.currObj < 0)					// make sure currObj is pointing to a valid sample
		this.currObj = 0;
	if (this.currObj >= this.obj.length)
		this.currObj = this.obj.length-1;
	this.updateUI();
	if (this.obj.length > 0)
		// show the sample
		this.viewer.setData(this.obj[this.currObj]['timing'], this.obj[this.currObj]['envelop']);
}

// Convinient routine to set the display attributes of HTML element
NavCtrlObj.prototype.setClass = function (ele, cls) {
	"use strict";
	if (this.flags[ele] != cls) {
		$('#'+ele).removeClass(this.flags[ele]);
		$('#'+ele).addClass(cls);
		this.flags[ele] = cls;
	}
}

// Update the display of sample information
NavCtrlObj.prototype.updateUI = function () {
	"use strict";
	// Meta
	if (this.obj.length > 0) {
		var meta = this.obj[this.currObj]['meta'];
		$('#stats-sample').html("Sample: <br> &nbsp; <b> "+(this.currObj+1)+" </b>");
		$('#stats-total').html("Total: <br> &nbsp; <b> "+(this.obj.length)+" </b>");
		$('#stats-time').html("Start Time: <br> &nbsp; <b> "+(meta['start'])+" </b>");
		$('#stats-setup').html("M. Setup: <br> &nbsp; <b> "+(meta['setup'])+" </b>");
	} else {
		$('#stats-sample').html("No Samples Loaded");
		$('#stats-total').html("");
		$('#stats-time').html("");
		$('#stats-setup').html("");
	}
	// Prev
	if (this.currObj > 0) {
		this.setClass('nav-prev', 'ctrl');
		$('#nav-prev').html ("<a href='javascript:" + this.myname + ".go(-1)'><b>&lt; Prev</b></a>");
	} else {
		this.setClass('nav-prev', 'noctrl');
		$('#nav-prev').html ("Prev");
	};
	// Next
	if (this.currObj < this.obj.length-1) {
		this.setClass('nav-next', 'ctrl');
		$('#nav-next').html ("<a href='javascript:" + this.myname + ".go(1)'><b>Next &gt;</b></a>");
	} else {
		this.setClass('nav-next', 'noctrl');
		$('#nav-next').html ("Next");
	};
	// No Zoom
	$('#no-zoom').html("<a href='javascript:" + this.myname + ".viewer.setZoom(null)'><small>No Zoom</small></a>");
}

// ---------------------------------------------------
// Dropdown Selection
//		Object declaration for dropdown selection
//		We have 3: file selection, viewer size selection, jitter display selection
// ---------------------------------------------------

function DDSelObj (form, name, opts, callback) {
	"use strict";
	this.form = form;				// name of the form containing this selector
	this.myname = name;				// name of this selector
	this.opts = opts;				// array of options for user to select
	this.callback = callback;		// when user makes a selection, the function to trigger
	this.val = -1;					// currently selected option
}

// Return the HTML for realizing the selector
DDSelObj.prototype.getHTML = function () {
	var html = "<select name='" + this.myname + "' onChange='javascript:" + this.callback + "'>";
	if (this.opts.length > 0) {
		for (var i=0; i<this.opts.length; i++)
			html += "<option value='" + this.opts[i] + "'>" + this.opts[i] + "</option>";
	} else 
		html += "<option value='None'>None</option>";
	html += "</select>";
	this.val = 0;
	return (html);
}	

// Set the currently selected index
DDSelObj.prototype.setval = function (v) {
	this.val = v;
	eval("document."+this.form+"."+this.myname+".selectedIndex") = v;
}

// Return the currently selected index
DDSelObj.prototype.getval = function () {
	return (this.val = eval("document."+this.form+"."+this.myname+".selectedIndex"));
}

// Return the text of the currently selected option
DDSelObj.prototype.getSelected = function () {
	this.getval();
	return (this.opts[this.val]);
}

// ---------------------------------------------------
// File based Navigation contol
//		Inheriited from NavCtrlObj to support multiple files
//		Basically NavCtrlObj only support a list of samples (presumably from a single file)
//		This extends it to support multiple list, each list associated to a filename
// ---------------------------------------------------

function FileNavCtrlObj (name, mviewer, sviewer, select, title) {
	"use strict";
	this.list = [];					// list of file (file = list of samples + filename)
	this.currList = 0;				// current file index
	this.title = title || 'title';			// ID of title display (used to display filename)
	this.select = select || 'file-sel';		// ID of the HTML element to contain selectors
	// file selector
	this.fselect = new DDSelObj ("sel", "fsel", [], name + ".fsel()");
	// screen size selector
	this.vsize = new DDSelObj ("sel", "vsel", [ '800x600', '1024x600', '1200x800' ], name + ".vsel()");	
	// jitter selector
	this.tscale = new DDSelObj ("sel", "tsel", [ 'Off', 'x1', 'x10', 'x20' ], name + ".tsel()");
	// call parent constructor
	NavCtrlObj.call(this, name, mviewer, sviewer);
}
inheritsFrom(FileNavCtrlObj, NavCtrlObj);

// set the list of files
FileNavCtrlObj.prototype.setList = function (list) {
	"use strict";
	this.list = list || [];			// array of files
	this.currList = 0;				// current file index
	// build up the options for file selector
	this.fselect.opts[i] = [];
	for (var i=0; i<this.list.length; i++)
		this.fselect.opts[i] = this.list[i].fname;
	this.initHTML();
	this.setFile(0);
}

// Set the current file to be "loaded"
FileNavCtrlObj.prototype.setFile = function (fid) {
	"use strict";
	if ((fid >= 0) && (fid < this.list.length))
		this.currList = fid;
	this.resetObj();
	$('#'+this.title).html(this.list[this.currList].fname);
}

// Reset the NavCtrl list of samples
FileNavCtrlObj.prototype.resetObj = function () {
	"use strict";
	// the list of samples for this file is a string, need to convert it before passing to NavCtrl
	this.setObj(JSON.parse(this.list[this.currList].objstr));
}

// Generates the HTML (mainly for the selectors)
FileNavCtrlObj.prototype.initHTML = function () {
	"use strict";
	$('#' + this.select).html("<form name='sel'>Input File: " + this.fselect.getHTML() + 
		' &nbsp; &nbsp; Viewer Size: ' + this.vsize.getHTML() + 
		' &nbsp; &nbsp; <font color="#d00000">Sampling Jitter: ' + this.tscale.getHTML() + "</font></form>");
	if (this.list.length > 0)
		$('#'+this.title).html(this.list[this.currList].fname);
	else
		$('#'+this.title).html("Sonar Sensing Signal Waveform");
}

// Callback when user makes a file selection
FileNavCtrlObj.prototype.fsel = function () {
	"use strict";
	var s = this.fselect.getval(); 
	if ((s>0) && (s<this.list.length))
		this.setFile(s);
}

// Callback when user change the viewer size
FileNavCtrlObj.prototype.vsel = function () {
	"use strict";
	var cnvw=null, cnvh=null;
	switch (this.vsize.getSelected()) {
		case "800x600": cnvw=800; cnvh=600; break;
		case "1024x600": cnvw=1024; cnvh=600; break;
		case "1200x800": cnvw=1200; cnvh=800; break;
	}
	this.viewer.resize(cnvw, cnvh);
}

// Callback when user set a jitter scale.
FileNavCtrlObj.prototype.tsel = function () {
	"use strict";
	switch (this.tscale.getSelected()) {
		case 'Off': this.viewer.main.timeDelayScale = 0;  break;	// we use 0 to represent no display of jitter
		case 'x1':  this.viewer.main.timeDelayScale = 1;  break;
		case 'x10': this.viewer.main.timeDelayScale = 10; break;
		case 'x20': this.viewer.main.timeDelayScale = 20; break;
	}
	this.viewer.updatePlot();
}

// ---------------------------------------------------
// Main Routine
// ---------------------------------------------------

var navctrl = null;

// triggered by basic2.html onload() 
function init() {
	// Using json2py, the samples will be converetd to a file 'samples.js' for this viewing
	// In bsic2.html, the 'samples.js' should be loaded by a <script> reference
	// 'samples.js' will define the variable 'json_samples' which should be the list of files
	// to be passed to FileNavCtrl.setList() method
	// We check if it is really defined
	if (typeof json_samples != 'undefined') {
		navctrl = new FileNavCtrlObj('navctrl', 'main-viewer', 'sub-viewer', 'file-sel', 'title');
		navctrl.setList(json_samples);
	} else {  
	// If 'jsaon_samples' is not defined, that means samples.js is not loaded
	// We simple test using a version 1 list of samples (not files!!)
	// but the variable name is testJSON, as declared below
		navctrl = new NavCtrlObj('navctrl', 'main-viewer', 'sub-viewer');
		navctrl.setObj(JSON.parse(testJSON));
	}
}

// ------------------------- Testing Only -------------------------
var testJSON = '\
[ { "meta": { "setup": 0, "num": 1000, "start": 123.456 },\
  "timing": [\
          0.000,  0.048,  0.096,  0.144,  0.192,  0.240,  0.288,  0.394,  0.457,  0.505,\
          0.553,  0.651,  0.762,  0.810,  0.858,  0.906,  1.016,  1.064,  1.158,  1.206,\
          1.254,  1.313,  1.361,  1.409,  1.457,  1.505,  1.601,  1.649,  1.697,  1.745,\
          1.793,  1.901,  1.980,  2.059,  2.125,  2.173,  2.266,  2.314,  2.362,  2.410,\
          2.458,  2.506,  2.554,  2.602,  2.693,  2.741,  2.820,  2.912,  2.960,  3.008,\
          3.056,  3.104,  3.152,  3.200,  3.284,  3.332,  3.380,  3.428,  3.476,  3.532,\
          3.580,  3.628,  3.676,  3.724,  3.772,  3.820,  3.868,  3.916,  4.004,  4.052,\
          4.100,  4.202,  4.250,  4.298,  4.346,  4.394,  4.442,  4.490,  4.538,  4.586,\
          4.634,  4.682,  4.730,  4.792,  4.845,  4.918,  4.966,  5.014,  5.062,  5.110,\
          5.158,  5.267,  5.315,  5.380,  5.428,  5.476,  5.565,  5.645,  5.693,  5.741,\
          5.789,  5.837,  5.885,  5.933,  5.981,  6.073,  6.121,  6.169,  6.217,  6.265,\
          6.313,  6.396,  6.444,  6.492,  6.540,  6.588,  6.636,  6.684,  6.732,  6.780,\
          6.879,  6.990,  7.055,  7.103,  7.151,  7.199,  7.305,  7.353,  7.401,  7.449,\
          7.497,  7.545,  7.593,  7.666,  7.714,  7.776,  7.824,  7.906,  7.954,  8.002,\
          8.050,  8.098,  8.146,  8.194,  8.242,  8.290,  8.371,  8.419,  8.467,  8.515,\
          8.563,  8.611,  8.659,  8.707,  8.755,  8.803,  8.851,  8.904,  8.964,  9.012,\
          9.060,  9.108,  9.156,  9.204,  9.252,  9.300,  9.411,  9.499,  9.547,  9.595,\
          9.681,  9.729,  9.777,  9.829,  9.877,  9.981, 10.029, 10.077, 10.125, 10.173,\
         10.234, 10.282, 10.330, 10.378, 10.475, 10.523, 10.579, 10.627, 10.675, 10.723,\
         10.771, 10.819, 10.867, 10.915, 10.963, 11.011, 11.059, 11.148, 11.196, 11.249,\
         11.297, 11.345, 11.393, 11.485, 11.533, 11.581, 11.629, 11.677, 11.725, 11.800,\
         11.848, 11.896, 11.944, 11.992, 12.040, 12.088, 12.136, 12.184, 12.263, 12.311,\
         12.359, 12.407, 12.455, 12.503, 12.551, 12.599, 12.647, 12.695, 12.743, 12.791,\
         12.839, 12.887, 12.985, 13.033, 13.081, 13.129, 13.177, 13.261, 13.309, 13.357,\
         13.429, 13.477, 13.525, 13.597, 13.645, 13.731, 13.779, 13.827, 13.892, 13.979,\
         14.027, 14.075, 14.123, 14.171, 14.219, 14.267, 14.315, 14.363, 14.411, 14.459,\
         14.507, 14.597, 14.645, 14.745, 14.793, 14.904, 14.952, 15.000, 15.048, 15.096,\
         15.144, 15.192, 15.240, 15.288, 15.336, 15.384, 15.432, 15.524, 15.572, 15.628,\
         15.676, 15.724, 15.772, 15.820, 15.868, 15.916, 15.964, 16.012, 16.060, 16.108,\
         16.156, 16.204, 16.252, 16.300, 16.348, 16.396, 16.457, 16.505, 16.553, 16.647,\
         16.695, 16.743, 16.822, 16.870, 16.963, 17.011, 17.059, 17.107, 17.215, 17.273,\
         17.321, 17.369, 17.452, 17.500, 17.548, 17.596, 17.644, 17.692, 17.740, 17.788,\
         17.836, 17.927, 18.003, 18.094, 18.142, 18.190, 18.238, 18.286, 18.334, 18.382,\
         18.430, 18.478, 18.557, 18.605, 18.653, 18.761, 18.809, 18.896, 18.944, 18.992,\
         19.040, 19.127, 19.175, 19.281, 19.329, 19.377, 19.425, 19.491, 19.539, 19.587,\
         19.686, 19.734, 19.782, 19.830, 19.912, 19.978, 20.026, 20.074, 20.122, 20.199,\
         20.310, 20.358, 20.406, 20.454, 20.502, 20.550, 20.598, 20.646, 20.706, 20.754,\
         20.807, 20.855, 20.903, 20.951, 21.027, 21.075, 21.123, 21.171, 21.238, 21.286,\
         21.334, 21.382, 21.444, 21.506, 21.554, 21.602, 21.650, 21.722, 21.796, 21.844,\
         21.940, 22.032, 22.080, 22.128, 22.206, 22.254, 22.302, 22.350, 22.454, 22.502,\
         22.588, 22.636, 22.684, 22.732, 22.780, 22.828, 22.925, 22.982, 23.030, 23.078,\
         23.126, 23.191, 23.239, 23.287, 23.381, 23.429, 23.477, 23.585, 23.633, 23.681,\
         23.729, 23.777, 23.825, 23.910, 23.958, 24.006, 24.054, 24.102, 24.150, 24.198,\
         24.246, 24.301, 24.352, 24.400, 24.508, 24.556, 24.604, 24.652, 24.736, 24.784,\
         24.832, 24.880, 24.928, 24.976, 25.024, 25.072, 25.120, 25.168, 25.216, 25.264,\
         25.312, 25.360, 25.408, 25.456, 25.504, 25.552, 25.600, 25.648, 25.696, 25.758,\
         25.806, 25.854, 25.902, 25.950, 25.998, 26.046, 26.094, 26.142, 26.190, 26.238,\
         26.286, 26.334, 26.382, 26.430, 26.478, 26.536, 26.584, 26.674, 26.722, 26.797,\
         26.845, 26.893, 26.941, 27.031, 27.079, 27.127, 27.175, 27.223, 27.271, 27.319,\
         27.367, 27.415, 27.463, 27.511, 27.593, 27.680, 27.728, 27.776, 27.830, 27.878,\
         27.926, 28.011, 28.059, 28.157, 28.205, 28.300, 28.348, 28.396, 28.444, 28.492,\
         28.540, 28.588, 28.636, 28.730, 28.778, 28.826, 28.881, 28.929, 28.977, 29.025,\
         29.073, 29.121, 29.169, 29.217, 29.265, 29.313, 29.361, 29.409, 29.457, 29.505,\
         29.553, 29.601, 29.649, 29.697, 29.745, 29.849, 29.897, 29.945, 30.018, 30.066,\
         30.114, 30.162, 30.210, 30.258, 30.306, 30.354, 30.402, 30.450, 30.498, 30.546,\
         30.615, 30.672, 30.720, 30.768, 30.875, 30.966, 31.014, 31.062, 31.120, 31.171,\
         31.219, 31.267, 31.315, 31.363, 31.416, 31.464, 31.512, 31.560, 31.608, 31.678,\
         31.726, 31.774, 31.822, 31.933, 31.981, 32.029, 32.077, 32.171, 32.275, 32.323,\
         32.371, 32.419, 32.508, 32.556, 32.604, 32.709, 32.818, 32.866, 32.962, 33.010,\
         33.058, 33.106, 33.207, 33.255, 33.324, 33.372, 33.420, 33.498, 33.546, 33.616,\
         33.724, 33.772, 33.820, 33.868, 33.916, 34.024, 34.072, 34.120, 34.172, 34.220,\
         34.268, 34.316, 34.378, 34.426, 34.517, 34.565, 34.613, 34.661, 34.709, 34.757,\
         34.805, 34.853, 34.901, 34.949, 34.997, 35.061, 35.109, 35.157, 35.205, 35.253,\
         35.301, 35.349, 35.397, 35.494, 35.542, 35.590, 35.638, 35.745, 35.793, 35.859,\
         35.919, 35.980, 36.061, 36.109, 36.205, 36.315, 36.403, 36.451, 36.499, 36.547,\
         36.626, 36.674, 36.722, 36.770, 36.818, 36.908, 36.956, 37.004, 37.052, 37.100,\
         37.148, 37.196, 37.244, 37.292, 37.340, 37.388, 37.436, 37.484, 37.532, 37.580,\
         37.628, 37.676, 37.724, 37.772, 37.821, 37.869, 37.917, 37.965, 38.013, 38.061,\
         38.109, 38.157, 38.205, 38.270, 38.318, 38.366, 38.414, 38.523, 38.600, 38.648,\
         38.696, 38.744, 38.792, 38.840, 38.888, 38.936, 38.984, 39.032, 39.080, 39.128,\
         39.212, 39.260, 39.308, 39.356, 39.446, 39.494, 39.542, 39.590, 39.686, 39.734,\
         39.782, 39.861, 39.909, 39.957, 40.005, 40.053, 40.101, 40.149, 40.197, 40.256,\
         40.333, 40.381, 40.429, 40.477, 40.525, 40.573, 40.621, 40.669, 40.717, 40.765,\
         40.813, 40.861, 40.909, 40.957, 41.005, 41.053, 41.101, 41.149, 41.197, 41.245,\
         41.293, 41.348, 41.424, 41.532, 41.580, 41.628, 41.676, 41.724, 41.776, 41.824,\
         41.872, 41.920, 41.968, 42.016, 42.093, 42.141, 42.189, 42.237, 42.344, 42.392,\
         42.503, 42.551, 42.599, 42.647, 42.695, 42.743, 42.794, 42.842, 42.890, 42.938,\
         42.986, 43.034, 43.082, 43.130, 43.183, 43.231, 43.298, 43.346, 43.394, 43.442,\
         43.511, 43.566, 43.614, 43.702, 43.750, 43.798, 43.860, 43.908, 43.956, 44.004,\
         44.052, 44.100, 44.148, 44.196, 44.244, 44.292, 44.340, 44.442, 44.490, 44.538,\
         44.586, 44.634, 44.682, 44.774, 44.822, 44.908, 44.956, 45.004, 45.052, 45.100,\
         45.148, 45.196, 45.288, 45.336, 45.384, 45.432, 45.480, 45.528, 45.576, 45.624,\
         45.672, 45.730, 45.818, 45.866, 45.914, 46.011, 46.079, 46.127, 46.175, 46.223,\
         46.271, 46.364, 46.412, 46.460, 46.516, 46.564, 46.612, 46.699, 46.747, 46.819,\
         46.875, 46.923, 46.971, 47.019, 47.130, 47.178, 47.228, 47.276, 47.324, 47.385,\
         47.433, 47.481, 47.529, 47.639, 47.718, 47.818, 47.866, 47.937, 47.985, 48.033,\
         48.084, 48.132, 48.180, 48.275, 48.323, 48.371, 48.419, 48.512, 48.622, 48.670,\
         48.718, 48.766, 48.814, 48.862, 48.910, 48.958, 49.006, 49.054, 49.102, 49.150,\
         49.198, 49.246, 49.294, 49.342, 49.390, 49.438, 49.486, 49.534, 49.582, 49.669,\
         49.717, 49.801, 49.861, 49.909, 49.998, 50.046, 50.094, 50.142, 50.190, 50.247,\
         50.295, 50.343, 50.391, 50.439, 50.487, 50.535, 50.583, 50.631, 50.679, 50.727,\
         50.775, 50.859, 50.916, 50.964, 51.012, 51.073, 51.121, 51.169, 51.217, 51.265,\
         51.313, 51.421, 51.469, 51.517, 51.578, 51.626, 51.674, 51.722, 51.770, 51.832,\
         51.882, 51.930, 52.008, 52.056, 52.104, 52.152, 52.200, 52.248, 52.296, 52.344,\
         52.392, 52.440, 52.490, 52.581, 52.629, 52.677, 52.725, 52.773, 52.821, 52.896,\
         52.944, 52.992, 53.040, 53.088, 53.143, 53.191, 53.239, 53.287, 53.335, 53.409,\
         53.497, 53.545, 53.593, 53.641, 53.689, 53.737, 53.848, 53.896, 53.977, 54.084,\
         54.132, 54.204, 54.252, 54.300, 54.348, 54.418, 54.466, 54.514, 54.562, 54.610,\
         54.658, 54.706, 54.798, 54.846, 54.894, 54.980, 55.081, 55.129, 55.220, 55.268,\
         55.316, 55.396, 55.444, 55.492, 55.540, 55.609, 55.688, 55.750, 55.798, 55.846 ],\
  "envelop": [\
          946,  941,  946,  944,  945,  948,  939,  945,  940,  941,\
          945,  942,  942,  945,  944,  940,  945,  944,  946,  939,\
          948,  943,  942,  941,  945,  946,  945,  945,  943,  940,\
          940,  946,  943,  944,  941,  942,  944,  935,  942,  942,\
          944,  936, 3871, 2939,  939,  931, 1051, 1060, 2222,  925,\
          927,  929,  934,  930,  930,  930,  932,  933, 1498, 2879,\
         2959, 2228, 1223, 1463, 1554, 1548, 1484, 1403, 1319, 1253,\
         1232, 1196, 1170, 1115, 1121, 1092, 1063, 1052, 1042, 1032,\
         1012, 1000,  998,  992,  989,  982,  978,  973,  969,  963,\
          965,  965,  956,  954,  951,  948,  950,  948,  953,  948,\
          949,  950,  949,  957,  953,  949,  945,  950,  942,  947,\
          947,  943,  951,  951,  941,  943,  946,  943,  945,  947,\
          941,  950,  943,  942,  938,  945,  943,  945,  945,  936,\
          944,  940,  947,  941,  942,  943,  938,  941,  938,  944,\
          944,  940,  937,  943,  948,  946,  940,  945,  955,  968,\
          971,  936,  927,  939,  937,  937,  938,  945,  953,  958,\
          959,  934,  939,  940,  938,  944,  939,  943,  940,  945,\
          944,  941,  940,  944,  937,  946,  946,  944,  948,  941,\
          940,  944,  945,  943,  937,  941,  941,  947,  946,  948,\
          940,  948,  943,  939,  939,  940,  947,  940,  940,  941,\
          943,  949,  946,  941,  942,  941,  938,  942,  941,  951,\
          948,  941,  942,  942,  951,  940,  936,  943,  936,  944,\
          948,  944,  945,  945,  942,  939,  940,  942,  949,  936,\
          936,  943,  941,  943,  947,  936,  944,  941,  937,  943,\
          942,  939,  940,  939,  939,  945,  938,  940,  948,  942,\
          941,  938,  942,  938,  937,  942,  944,  941,  940,  943,\
          948,  940,  938,  942,  942,  938,  937,  946,  944,  937,\
          941,  941,  942,  941,  942,  940,  941,  936,  948,  935,\
          936,  931,  931,  929,  929,  927,  925,  926,  924,  922,\
          919,  920,  917,  917,  919,  913,  911,  918,  915,  916,\
          911,  912,  910,  914,  909,  915,  911,  915,  913,  912,\
          912,  918,  916,  917,  917,  922,  922,  925,  923,  929,\
          927,  923,  925,  923,  923,  924,  928,  926,  931,  926,\
          923,  926,  923,  926,  922,  919,  919,  919,  920,  924,\
          927,  931,  932,  935,  934,  934,  933,  933,  929,  931,\
          931,  937,  939,  941,  939,  935,  934,  934,  939,  941,\
          942,  944,  948,  951,  952,  948,  948,  948,  946,  947,\
          944,  939,  939,  942,  943,  941,  936,  942,  939,  937,\
          939,  941,  942,  946,  948,  953,  957,  956,  959,  956,\
          952,  955,  954,  955,  949,  945,  947,  947,  944,  939,\
          935,  937,  941,  945,  943,  943,  942,  940,  938,  936,\
          936,  931,  931,  929,  927,  930,  931,  929,  927,  921,\
          921,  924,  918,  917,  919,  915,  916,  916,  913,  912,\
          910,  912,  911,  906,  906,  907,  911,  909,  911,  912,\
          914,  912,  905,  906,  907,  908,  905,  907,  909,  907,\
          910,  906,  902,  901,  901,  899,  903,  907,  905,  907,\
          911,  911,  914,  908,  909,  909,  912,  912,  918,  917,\
          917,  914,  913,  914,  913,  916,  915,  915,  914,  913,\
          910,  912,  912,  913,  918,  916,  911,  908,  904,  906,\
          905,  906,  907,  900,  899,  894,  891,  893,  891,  892,\
          891,  888,  885,  880,  880,  885,  884,  887,  885,  885,\
          886,  882,  882,  879,  876,  870,  877,  873,  874,  872,\
          872,  875,  878,  878,  875,  874,  873,  876,  881,  880,\
          874,  876,  869,  865,  866,  872,  876,  876,  879,  878,\
          881,  875,  879,  877,  874,  877,  877,  876,  872,  871,\
          871,  870,  870,  876,  872,  870,  875,  876,  873,  873,\
          872,  871,  867,  872,  874,  868,  868,  872,  866,  868,\
          869,  875,  880,  884,  878,  876,  878,  872,  874,  876,\
          876,  878,  872,  875,  882,  887,  884,  885,  883,  881,\
          888,  888,  885,  881,  886,  886,  893,  893,  893,  890,\
          885,  885,  888,  890,  885,  885,  882,  884,  882,  875,\
          879,  878,  877,  877,  873,  875,  875,  878,  872,  877,\
          875,  876,  877,  878,  881,  888,  887,  888,  883,  883,\
          881,  881,  882,  888,  891,  893,  886,  885,  884,  886,\
          881,  877,  877,  878,  878,  881,  881,  879,  879,  877,\
          877,  877,  878,  879,  881,  881,  874,  877,  879,  881,\
          875,  878,  881,  878,  877,  878,  873,  876,  870,  869,\
          868,  871,  872,  872,  875,  879,  878,  872,  867,  870,\
          872,  869,  875,  874,  876,  882,  879,  886,  887,  886,\
          890,  892,  892,  885,  879,  878,  871,  865,  868,  870,\
          869,  869,  870,  871,  878,  879,  878,  878,  878,  880,\
          881,  887,  889,  887,  889,  889,  895,  901,  899,  900,\
          895,  891,  896,  898,  897,  895,  890,  888,  890,  886,\
          886,  888,  893,  891,  892,  897,  895,  892,  893,  890,\
          892,  899,  898,  902,  909,  911,  906,  905,  905,  900,\
          895,  892,  891,  894,  891,  894,  892,  893,  890,  891,\
          890,  892,  889,  887,  883,  881,  878,  880,  881,  885,\
          888,  893,  894,  894,  895,  889,  886,  889,  885,  889,\
          889,  888,  887,  891,  889,  887,  893,  887,  885,  883,\
          880,  883,  881,  884,  885,  884,  883,  884,  883,  880,\
          883,  880,  884,  885,  884,  888,  892,  888,  885,  878,\
          878,  877,  876,  875,  874,  872,  871,  873,  876,  872,\
          876,  878,  871,  873,  877,  882,  879,  880,  876,  877,\
          881,  877,  881,  876,  872,  878,  875,  869,  871,  872,\
          872,  875,  881,  886,  886,  883,  884,  887,  888,  889,\
          891,  896,  897,  896,  894,  893,  887,  887,  888,  891,\
          891,  891,  889,  890,  884,  885,  885,  888,  885,  882,\
          888,  894,  892,  888,  881,  881,  883,  888,  892,  888,\
          886,  889,  890,  891,  894,  892,  893,  889,  889,  889,\
          895,  896,  903,  898,  903,  901,  902,  904,  908,  909,\
          910,  913,  912,  910,  904,  904,  897,  899,  898,  904,\
          904,  908,  911,  914,  915,  921,  920,  915,  911,  910,\
          912,  911,  911,  915,  918,  924,  928,  928,  922,  922,\
          928,  924,  917,  915,  909,  905,  905,  907,  908,  912,\
          914,  913,  912,  911,  915,  912,  910,  911,  911,  916,\
          917,  916,  915,  913,  913,  910,  908,  913,  912,  913,\
          909,  904,  905,  901,  903,  903,  903,  903,  902,  905,\
          908,  913,  913,  915,  917,  923,  921,  916,  914,  911,\
          912,  909,  915,  915,  921,  920,  925,  929,  929,  930,\
          928,  926,  932,  936,  936,  935,  938,  941,  944,  946 ]\
}]';
