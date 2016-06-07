// Whether to use AJAX or not
var DEF_AJAX = true;

// Result Display Object
function objResults () {
	// object members
	this.list = [];
	this.keyList = [];
	this.valList = [];
	// -- update method: update result list with new list
	this.update = function (string) {
		this.list = eval(string);
		this.keyList = [];
		this.valList = [];
		for (var i=0; i<this.list.length; ) {
			this.keyList.push(this.list[i++]);
			this.valList.push(this.list[i++]);
		}
	};
	// -- getVal method: get a specific result
	this.getVal = function (key) {
		for (var i=0; i<this.keyList.length; i++)
			if (key == this.keyList[i])
				return (this.valList[i]);
		return '--';
	};
	// -- show method: return HTML representation of results
	this.show = function () {
		var html = '';

		html = "<p align='center'>" +
				"<table border='0' cellpadding='2' cellspacing='5'>" +
				"<tr><td class='label-main'> Curb Height: </td>" +
				"<td class='value-main'> &nbsp; " + this.curbHeight(this.getVal('height')) + " &nbsp; </td></tr>" +
				"<tr><td class='label-main'> Distance: </td>" +
				"<td class='value-main'> &nbsp; " + this.distance(this.getVal('dist')) + " &nbsp; </td></tr>" +
				"<tr><td id='FV-ctrl' colspan='2'>" + this.getFVhtml(false) + "</td></tr>" +
				"</table></p>";

		return (html);
	};
	// -- getFVhtml: generate the feature vector display HTML
	this.getFVhtml = function (show) {
		if (show) {
			var html = "";
			var fvn = 0;
			for (var k=0; k<this.keyList.length; k++) {
				if ((this.keyList[k] == 'curb') || (this.keyList[k] == 'distance'))
					// skip curb or distance
					continue;
				html += ((fvn & 1) ? "" : "<tr>") +
					"<td class='label-sub'> &nbsp; " + this.keyList[k] + " &nbsp; </td>" +
					"<td class='value-sub'> &nbsp; " + this.valList[k] + " &nbsp; </td>" +
					((fvn & 1) ? "</tr>" : "<td></td>");
				fvn++;
			}
			return (
				"<a href='javascript:stateMachine.res.hideFV()'>Click here to hide feature vector</a></br>" +
				"<table border='0' cellpadding='2' cellspacing='2' width='100%'>" + html +
				"</table></br>"
			);
		} else {
			return ("<a href='javascript:stateMachine.res.showFV()'>Click here to show feature vector</a>");
		}
	};
	this.hideFV = function () {
		$('#FV-ctrl').html(this.getFVhtml(false));
	};
	this.showFV = function () {
		$('#FV-ctrl').html(this.getFVhtml(true));
	};
	this.curbHeight = function (val) {
		return val + " cm";
	}
	this.distance = function (val) {
		return val + " m"
	}
}

function objSetting () {
	
	// -- init configure form	
	this.init = function () {
		if (this.defsvm == null) {
			this.defsvm = 'train.svm';
		}
		if (this.deftime == null) {
			this.deftime = "17,33,50,100,48";
		}
		if (this.defcycle == null) {
			this.defcycle = "100";
		}
		if (this.deffv == null) {
			this.deffv = ["peakValue", "totalArea", "peakTime", "refTimeWidth-immed", "areaRatio"];
		}
		
		// create form
		this.form = document.createElement("form");
		this.form.method = "post";
		this.form.id = "ConfigForm";
		
		var br = document.createElement("br")
		
		var svmDiv = this.getSVMDiv();				// create SVM form
		var timeCycleDiv = this.getTimeCycleDiv();	// create time and cycle form
		var fvDiv = this.getFVDiv();				// create fv form
				
		// join all form together
		this.form.appendChild(svmDiv);
		this.form.appendChild(br)
		this.form.appendChild(timeCycleDiv);
		this.form.appendChild(br)
		this.form.appendChild(fvDiv);
		
		// replace and show in browser
		document.getElementById("display-div").appendChild(this.form);
	}
	
	this.getSVMDiv = function () {
		var div = document.createElement('div')
		// create SVM label
		var svmLabel = document.createTextNode("SVM File: ");
		
		// create SVM text field		
		svmField = document.createElement("input");
		svmField.type = "text";
		svmField.id = "config-svm";
		svmField.defaultValue = this.defsvm;
		
		// create SVM file selector and add change listener
		var svmSelect = document.createElement("input");
		svmSelect.type = "file";
		svmSelect.id = "config-svmFile";
		svmSelect.accept = ".svm";
		
		$(svmSelect).on('change', function() {
			var svm = this.value.replace("C:\\fakepath\\", "");
			svmField.value = svm;
		});
		
		div.appendChild(svmLabel);
		div.appendChild(svmField);
		div.appendChild(svmSelect);
		
		return (div)
	}
	
	this.getTimeCycleDiv = function () {
		var div = document.createElement('div');
		
		// create time form
		var timeLabel = document.createTextNode("Timing: ");		
		var timeField = document.createElement("input");
		timeField.type = "text";
		timeField.id = "config-timing";
		timeField.defaultValue = this.deftime;
		
		// create cycle form
		var cycleLabel = document.createTextNode("Cycle: ");
		var cycleField = document.createElement("input");
		cycleField.type = "text";
		cycleField.id = "config-cycle";
		cycleField.defaultValue = this.defcycle;
		
		div.appendChild(document.createElement("br"));
		div.appendChild(timeLabel);
		div.appendChild(timeField);
		div.appendChild(cycleLabel);
		div.appendChild(cycleField);
		
		return (div)
	}
	
	this.getFVDiv = function () {
		var div = document.createElement('div')
		
		// create feature checkpoint
		var featureLabel = document.createTextNode("Features: ");
		var featureField = this.fvChkBox();
		
		div.appendChild(featureLabel);
		div.appendChild(featureField);
		
		return (div)
	}
	
	// place all feature as checkbox
	this.fvChkBox = function () {
		this.fvList = ["refTimeWidth-ave", "refTimeWidth-immed", "refTimeWidth-grad", "normTimeWidth", "peakValue", "totalArea", "areaRatio", 
						"peakTime", "+gradRatio", "-gradRatio", "0gradRatio", "peakWidthDiff", "peakWidthDiv", "gradChange"];
		
		var div = document.createElement('div')
		div.id = 'fv-checkbox';

		for (i = 0; i < this.fvList.length; i++) {
			if (i % 2 == 0) {
				div.appendChild(document.createElement("br"));
			}
			var fv = this.fvList[i];
			var chkboxLabel = document.createTextNode(fv);
			var chkbox = document.createElement("input");
			chkbox.type = "checkbox";
			chkbox.id = "config-fv-" + fv;
			chkbox.name = "fv-" + fv;
			
			// default feature checked
			if (this.deffv.indexOf(fv) > -1) {
				chkbox.checked = true;
			}
			
			div.appendChild(chkbox);
			div.appendChild(chkboxLabel);
		}
		return (div)
	}
	
	this.getCheckedBox = function () {
		var fvSelect = ''
		this.deffv = [];
		for (i = 0; i < this.fvList.length; i++) {
			var fv = this.fvList[i];
			var chkbox = this.form.elements["config-fv-" + fv];
			if (chkbox.checked == true) {
				fvSelect = fvSelect + fv + ',';
				this.deffv.push(fv);
			}
		}
		console.log(this.deffv);
		return (fvSelect.substring(0, fvSelect.length-1))
	}
	
	// -- setting when user click configure
	this.onSet = function () {
		this.defsvm = this.form.elements["config-svm"].value;
		this.deftime = this.form.elements["config-timing"].value;
		this.defcycle = this.form.elements["config-cycle"].value;

		var svmFile = 'SVM=' + this.defsvm;
		var timing = 'TIME=' + this.deftime;
		var cycle = 'CYCLE=' + this.defcycle;
		var feature = 'FV=' + this.getCheckedBox();
		var setting = 'CONFIGURE:' + svmFile + '_' + timing + '_' + cycle + '_' + feature + '\n'
		return (setting)
	}	
}

var stateMachine = {
	state: 'IDLE',
	// -- init() method
	init: function () {
		// --- preload images --
		getImage("images/banner.png");
		getImage("images/matrix.gif");
		// --- initialize control button --
		this.btn = '#demo-real-btn';
		$(this.btn).hover(
			function() { $(this).css('background', '#f0e0e0'); },
			function() { $(this).css('background', '#e0e0e0'); }
		);
		$(this.btn).on('click', function() {
			stateMachine.btnClick(this.id);
		});
		// --- initialize control button --
		this.btn2 = '#demo-non-btn';
		$(this.btn2).hover(
			function() { $(this).css('background', '#f0e0e0'); },
			function() { $(this).css('background', '#e0e0e0'); }
		);
		$(this.btn2).on('click', function() {
			stateMachine.btnClick(this.id);
		});
		this.btn3 = '#setting-btn';
		$(this.btn3).on('click', function() {
			stateMachine.lnkClick(this.id)
		});
		// --- initialize display div ---
		this.dsply = '#display-div';
		// --- instantiate new result object --
		this.res = new objResults();
		// --- instantiate image switchers --
		this.imgVehicle = new objImageSwitcher('stateMachine.imgVehicle', 'vehicle-img', 'images/car-', '.png', 8, 500);
		// --- transit to IDLE state
		this.onIdle();
	},
	// -- when state change to IDLE
	onIdle: function () {
		this.state = 'IDLE';
		$(this.btn).html('START RealTime Demo');
		$(this.btn2).html('START Non-RealTime Demo')
		$(this.dsply).html(
			'</br><image src="images/banner.png"/></br>'
		);
	},
	// -- when state change to Collect
	onCollect: function () {
		this.state = 'COLLECT';
		$(this.btn2).html('STOP');
		$(this.dsply).html(
			'</br>Please wait for data to collect ...</br>' +
			'</br><image id="vehicle-img" src="images/banner.png" /></br>' +
			'</br>Press [STOP] when completed ...</br>'
		);
		this.imgVehicle.start();
	},
	// -- when state change to Collect-Real
	onCollectReal: function () {
		this.state = 'COLLECT-REAL';
		$(this.btn2).html('STOP');
		this.doAjaxUpdate('UPDATE', $(this.dsply));
		$(this.dsply).html(
			'</br>This is the results of Classifications</br>' +
			this.res.show() +
			'</br>Press [CLEAR] to reset ...</br>'
		);
	},
	// -- when state change to WaitResults
	onWaitRes: function () {
		this.state = 'WAIT-RES';
		$(this.dsply).html(
			'</br>Please wait while the sensor readings are being analyzed ... </br>' +
			'</br><image src="images/matrix.gif" height="200"/></br>'
		);
	},
	// -- when state change to ShowResults
	onResults: function () {
		this.state = 'RESULTS';
		$(this.btn2).html('CLEAR');
		$(this.dsply).html(
			'</br>This is the results of Classifications</br>' +
			this.res.show() +
			'</br>Press [CLEAR] to reset ...</br>'
		);
	},
	onConfigure: function () {
		$(this.dsply).html('');
		this.conf.init();
		
		$(this.btn).html('Configure');
		$(this.btn2).html('Cancel');
	},
	// -- when state change to ShowResults
	onPreIdle: function () {
		document.getElementById('demo-real-btn').style.visibility='visible';
	},
	// --- Link Press Handler
	lnkClick: function (id) {
		this.state = 'CONFIG-SET';
		if (this.conf == null) {
			this.conf = new objSetting();
		}
		this.onConfigure();
	},
	// --- Button Press Handler
	btnClick: function (id) {
		switch (this.state) {
			case 'IDLE':
				document.getElementById('demo-real-btn').style.visibility='hidden';
				// -- User click on 'START-realTime'
				if (id == 'demo-real-btn') {
					if (DEF_AJAX) {
						this.doAJAX('START-REAL')
					} else {
						this.onCollectReal();
					}
				} else { // -- User click on 'START-non realTime'
					if (DEF_AJAX) {
						this.doAJAX('START-NONREAL')
					} else {
						this.onCollect();
					}
				}
				return;
			case 'CONFIG-SET':
				if (id == 'demo-real-btn') {
					// -- User click on 'CONFIG'
					var setting = this.conf.onSet();
					if (DEF_AJAX) {
						this.doAJAX(setting);
					}
					console.log(setting);
				}
				this.onIdle();
				return;
			case 'COLLECT':
				// -- User click on 'STOP'
				this.imgVehicle.stop();
				if (DEF_AJAX) {
					this.onWaitRes();
					this.doAJAX('STOP');
				} else {
					this.res.update('["curb", 10, "distance", 3.0, "fv", 0.3]');
					this.onResults();
				}
				return;
			case 'COLLECT-REAL':
				// -- User click on 'STOP' at real time mode
				if (DEF_AJAX) {
					this.doAJAX('STOP');
				}
				this.onPreIdle();
				this.onIdle();
				return;
			case 'RESULTS':
				this.onPreIdle();
				// -- User click on 'CLEAR' --> Back to IDLE state
				this.onIdle();
				return;
		}
	},
	// -- Perform AJAX Call
	doAJAX: function (cmd) {
		$.ajax(
		{
			type: "POST",
			url: "/cgi/handle-ajax.py",
			data: "PARAM="+cmd,
			dataType: "text",
			success: function(response) {
				eval(response);
				return false;
			}
		});
	},
	updateDemo: function () {
		if (this.state != 'IDLE') {
			this.doAjaxUpdate();
		}
	},
	// -- Perform real time update call
	doAjaxUpdate: function (cmd, dsply) {
		$.ajax(
		{
			type: "POST",
			url: "/cgi/handle-ajax.py",
			data: "PARAM=UPDATE",
			dataType: "text",
			success: function(response) {
				eval(response);
				window.setTimeout('stateMachine.updateDemo()', 100);
				return false;
			}
		});
	},
	// -- Handle AJAX Response
	ajax_response: function (cmd, param) {
		switch (cmd) {
			case 'START-NONREAL':
				// --- Response to a 'CGI:START-NONREAL'
				// --- >>> Do nothing
				window.setTimeout('stateMachine.onCollect()', 100);
				return;
			case 'START-REAL':
				// --- Response to a 'CGI:START-REAL'
				// --- >>> Do nothing
				window.setTimeout('stateMachine.onCollectReal()', 100);
				return;
			case 'STOP':
				// --- Response to a 'CGI:STOP'
				// --- >>> Display the results
				this.res.update(param);
				window.setTimeout('stateMachine.onResults()', 100);
				return;
			case 'STOP-REAL':
				// --- Response to a 'CGI:STOP-REAL'
				return;
			case 'UPDATE':
				this.res.update(param);
				window.setTimeout('stateMachine.onCollectReal()', 100);
				return;
		}
	},
	
}

// --- Preload Images ---
var imgObj = [];
var numImg = 0;

// Retrieve an image object
function getImage (src) {
	var i;
	for (i=0; i<numImg; i++)
		if (imgObj[i].src == src)
			return imgObj[i];
	imgObj[numImg] = new Image();
	imgObj[numImg].src = src;
	numImg++;
	return imgObj[numImg-1];
}

// --- Image switcher
function objImageSwitcher (myName, imgId, srcBase, srcExt, max, period) {
	this.name = myName;
	this.imgId = '#' + imgId;
	this.srcBase = srcBase;
	this.srcExt = srcExt;
	this.currIdx = 0;
	this.maxIdx = max;
	this.period = period;
	this.enable = false;
	// -- preload --
	for (var i=0; i<this.maxId; i++)
		getImage(this.srcBase + i + this.srcExt);
	// -- method: start periodic change --
	this.start = function () {
		if (!this.enable) {
			this.enable = true;
			this.change();
		}
	};
	// -- method: do periodic change --
	this.change = function () {
		if (this.enable) {
			this.currIdx += 1;
			if (this.currIdx >= this.maxIdx)
				this.currIdx = 0;
			$(this.imgId).attr("src", this.srcBase + this.currIdx + this.srcExt);
			window.setTimeout(this.name + ".change()", this.period);
		}
	};
	// -- method: stop periodic change --
	this.stop = function () { this.enable = false; };
}
