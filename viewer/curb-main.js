// Whether to use AJAX or not
var DEF_AJAX = true;

// Result Display Object
function objResults() {
    // object members
    this.list = [];
    this.keyList = [];
    this.valList = [];
    // -- update method: update result list with new list
    this.update = function (string) {
        this.list = eval(string);
        this.keyList = [];
        this.valList = [];
        for (var i = 0; i < this.list.length;) {
            this.keyList.push(this.list[i++]);
            this.valList.push(this.list[i++]);
        }
    };
    // -- getVal method: get a specific result
    this.getVal = function (key) {
        for (var i = 0; i < this.keyList.length; i++)
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

    this.showUpdateRealTime = function () {
        // -- need to check if realtime mode activated or not
        if (realtime.activated == false){
            console.log("Error realtime mode not activated at show update");
        }

        //need to update the status curb, distance and log according to the code
        var statusBurst;// true or false
        var curb;// string
        var dist;
        var statusString; // get the status
        var curbString; // get the curb string
        var distString; // get the distance

        // make status
        statusBurst = statusString.toLowerCase() == "burst";
        // make curb
        if (curbString.indexOf("curb") > -1){ // check if it is CURB
            // curb detected
            curb = curbString.substring(4);
        }
        else {
            curb = curbString.toLowerCase();
        }
        // make dist
        dist = distString;

        // code for update the status
        if (curb == "5"){
            curb = " 5";
        }
        realtime.updateStatus(statusBurst);
        realtime.updateCurb(curb);
        realtime.updateDist(dist);

        // log output
        var time = new Date();
        var log = time.toTimeString().substring(0,8) + "--";
        if (curb == "none"){
            log += "found nothing";
        }
        else if (curb == "unknown"){
            log += "found unknown at " + dist + " m";
        }
        else {
            //when found curb
            log += "found curb " + curb + " at " + dist + " m";
        }
        realtime.updateLog(log)

    };
    // -- getFVhtml: generate the feature vector display HTML
    this.getFVhtml = function (show) {
        if (show) {
            var html = "";
            var fvn = 0;
            for (var k = 0; k < this.keyList.length; k++) {
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

function objSetting() {

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

        $(svmSelect).on('change', function () {
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
        return (fvSelect.substring(0, fvSelect.length - 1))
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
            function () {
                $(this).css('background', '#f0e0e0');
            },
            function () {
                $(this).css('background', '#e0e0e0');
            }
        );
        $(this.btn).on('click', function () {
            stateMachine.btnClick(this.id);
        });
        // --- initialize control button --
        this.btn2 = '#demo-non-btn';
        $(this.btn2).hover(
            function () {
                $(this).css('background', '#f0e0e0');
            },
            function () {
                $(this).css('background', '#e0e0e0');
            }
        );
        $(this.btn2).on('click', function () {
            stateMachine.btnClick(this.id);
        });
        this.btn3 = '#setting-btn';
        $(this.btn3).on('click', function () {
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
        /*
        $(this.dsply).html(
            '</br>This is the results of Classifications</br>' +
            this.res.show() +
            '</br>Press [CLEAR] to reset ...</br>'
        );
        */

        if (realtime.activated == false){
            console.log("realtime mode not activated to update collection");
            return;
        }
        this.res.showUpdateRealTime();
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
    onResultsReal: function () {
        this.state = 'RESULT';
        $(this.btn2).html('CLEAR');
        this.res.showUpdateRealTime();
        //realtime.activated = false;
    },
    onConfigure: function () {
        $(this.dsply).html('');
        this.conf.init();

        $(this.btn).html('Configure');
        $(this.btn2).html('Cancel');
    },
    // -- when state change to ShowResults
    onPreIdle: function () {
        document.getElementById('demo-real-btn').style.visibility = 'visible';
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
                document.getElementById('demo-real-btn').style.visibility = 'hidden';
                // -- User click on 'START-realTime'
                if (id == 'demo-real-btn') {
                    if (DEF_AJAX) {
                        realtime.activated = true;
                        realtime.init();
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
                    this.doAJAX('STOP'); // call the cgi to stop the recording for non realtime
                } else {
                    this.res.update('["curb", 10, "distance", 3.0, "fv", 0.3]');
                    this.onResults();
                }
                return;
            case 'COLLECT-REAL':
                // -- User click on 'STOP' at real time mode
                if (DEF_AJAX) {
                    this.doAJAX('STOP-REAL'); // call the cgi to stop recording on realtime
                }
                this.onPreIdle();
                realtime.deactivate();
                this.onIdle();
                //realtime.activated = false;
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
                data: "PARAM=" + cmd,
                dataType: "text",
                success: function (response) {
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
                success: function (response) {
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
                // activate the realtime mode interface
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
                this.res.update(param);// need another update method
                window.setTimeout('stateMachine.onCollectReal()', 100);
                return;
        }
    },

}

// --- Preload Images ---
var imgObj = [];
var numImg = 0;

// Retrieve an image object
function getImage(src) {
    var i;
    for (i = 0; i < numImg; i++)
        if (imgObj[i].src == src)
            return imgObj[i];
    imgObj[numImg] = new Image();
    imgObj[numImg].src = src;
    numImg++;
    return imgObj[numImg - 1];
}

// --- Image switcher
function objImageSwitcher(myName, imgId, srcBase, srcExt, max, period) {
    this.name = myName;
    this.imgId = '#' + imgId;
    this.srcBase = srcBase;
    this.srcExt = srcExt;
    this.currIdx = 0;
    this.maxIdx = max;
    this.period = period;
    this.enable = false;
    // -- preload --
    for (var i = 0; i < this.maxId; i++)
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
    this.stop = function () {
        this.enable = false;
    };
}

function curbChangeColor(text) {
    var style = text.style;
    if (text.innerHTML == " 5 cm"){
        style.backgroundColor = "green";
        style.color = "black";
    }
    else if (text.innerHTML == "10 cm"){
        style.backgroundColor = "yellow";
        style.color = "black";
    }
    else if (text.innerHTML == "15 cm"){
        style.backgroundColor = "red";
        style.color = "white";
    }
    else if (text.innerHTML.toLowerCase() == "none"){
        style.backgroundColor = "lightblue";
        style.color = "black";
    }
    else if (text.innerHTML.toLowerCase() == "unknown"){
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
    if (val == 15){
        newVal =5;
    }
    else {
        newVal += 5;
    }
    text.innerHTML = newVal + " cm";
    curbChangeColor(text);
}

var realtime = {
    activated: false, // to check whether the realtime mode is activated
    init: function () {
        this.div = document.getElementById("display-div");
        // remove everything from the element
        while (this.div.hasChildNodes()){
            this.div.removeChild(this.div.lastChild);
        }
        this.table = document.createElement("table");
        this.table.id = "display-table";
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
        /*
        this.curbVal.addEventListener("click", function () {
            curbChangeValue(this);
        });
        */
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
        this.log.value = "Log testing\n" +
            "10:26:30 detect curb 10 at 3.01 m (testing)\n";

        this.log.rows = "14";
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
        this.paragraph = document.createElement("p");
        this.paragraph.style.alignSelf = "center";
        this.paragraph.appendChild(this.table);
        this.div.appendChild(this.paragraph);
        this.addStyle();
        this.activated = true;
    },

    deactivate: function () {
        // remove everything from the element
        this.activated = false;
        while (this.div.hasChildNodes()){
            this.div.removeChild(this.div.lastChild);
        }
    },

    addStyle: function () {
        var table = document.getElementById("display-table");// since we have 2 table
        table.style.border = "1px solid black";
        table.style.borderCollapse = "collapse";
        table.style.fontSize =  "150%";
        table.style.width = "500px";

        var tdLabel = document.getElementsByClassName("label");
        for (var i = 0; i < tdLabel.length; i++){
            tdLabel.item(i).style.backgroundColor = "#210ebd";
            tdLabel.item(i).style.color = "aliceblue";
            tdLabel.item(i).style.border = "1px solid black";
            tdLabel.item(i).style.borderCollapse = "collapse";
            tdLabel.item(i).style.fontSize = "150%";
        }
        var tdValue = document.getElementsByClassName("value");
        for (var i = 0; i < tdValue.length; i++){
            tdValue.item(i).style.backgroundColor = "white";
            tdValue.item(i).style.color = "black";
            tdValue.item(i).style.textAlign = "center";
            tdValue.item(i).style.border = "1px solid black";
            tdValue.item(i).style.borderCollapse = "collapse";
            tdValue.item(i).style.fontSize = "150%";
        }
        curbChangeColor(this.curbVal);
    },

    updateStatus: function (burstOrNot) {
        if (burstOrNot){
            this.statusVal.innerHTML = "Burst";
        }
        else {
            this.statusVal.innerHTML = "Measurement";
        }
    },

    updateCurb: function (val) {
        switch (val){
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
    }

};

