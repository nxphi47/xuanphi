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
    personalInit: function () {
        var division = document.getElementById("content");
        while (division.hasChildNodes()){
            division.removeChild(division.lastChild);
        }
        function makeBr() {
            return document.createElement("br");
        }
        // form making
        var form = document.createElement("form");
        form.style.backgroundColor = "white";

        //personal fieldset
        var personalFieldset = document.createElement("fieldset");
        var legend = document.createElement("legend");
        legend.innerHTML = "Basic information";

        var firstText = document.createTextNode("First name: ");
        var first = document.createElement("input");
        first.type = "text";
        first.required = "required";
        first.placeholder = "Xuan Phi";

        var lastText = document.createTextNode("Last name: ");
        var last = document.createElement("input");
        last.type = "text";
        last.required = "required";
        last.placeholder = "Nguyen";

        var agetext = document.createTextNode("your age: ");
        var age = document.createElement("input");
        age.type = "number";
        age.required = "required";
        age.defaultValue = "20";
        age.min = "1";
        age.max = "100";

        var countryText = document.createTextNode("Nationality: ");
        var country = document.createElement("input");
        country.type = "text";
        country.required = "required";
        country.pattern = "[A-Z]{3}";
        country.placeholder = "VNM";

        var genderText = document.createTextNode("Gender: ");
        var gender = [];
        for (var i = 0; i < 2; i++){
            var gen = document.createElement("input");
            gen.type = "radio";
            gen.value = (i == 0)?"male":"female";
            gen.innerHTML = (i == 0)?"Male":"Female";
            gender.concat(gen);
        }

        personalFieldset.appendChild(firstText);
        personalFieldset.appendChild(first);
        personalFieldset.appendChild(lastText);
        personalFieldset.appendChild(last);
        personalFieldset.appendChild(makeBr());
        personalFieldset.appendChild(agetext);
        personalFieldset.appendChild(age);
        personalFieldset.appendChild(countryText);
        personalFieldset.appendChild(country);
        personalFieldset.appendChild(makeBr());
        personalFieldset.appendChild(genderText);

        personalFieldset.appendChild(legend);
        // finish personal fieldset

        // submit button
        var submit = document.createElement("input");
        submit.type = "submit";
        submit.value = "Submit";

        form.appendChild(personalFieldset);
        form.appendChild(submit);
        // finish formaking
        division.appendChild(form);
    },
    
    statusInit: {
        
    }
};

function curbChangeColor(text) {
    var style = text.style;
    if (text.innerHTML == "5 cm"){
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
        this.curbVal.innerHTML = "5 cm"; //  change according to....
        // need to add the default style and change of style
        //noinspection JSValidateTypes
        this.curbVal.addEventListener("click", function () {
            curbChangeValue(this);
        });
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
        this.log.value = "asdjkasldjklajklw\n" +
            "akwjdawjdlkawjdlakwdjwalk fwaidjawidjawd\n" +
            "aslkdjakljda\n" +
            "dkajdjwiaofjawj \nadawjddawdjkawdjawkldj\n" +
            "dawjdkawjdjaiodjawoidjawiodjawio\n" +
            "adklajwkdjawajwk fljawfioaw\n" +
            "awkjdawjdlkawjdklawjdklawjdkawljdkawjdkalwjdawljdalwkjd\n" +
            "wankaw jawja wjowj waopjf \n";
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
    },

    addStyle: function () {
        var table = document.getElementsByTagName("table")[0];// since we have 2 table
        table.style.border = "1px solid black";
        table.style.borderCollapse = "collapse";
        table.style.fontSize =  "150%";
        table.style.width = "500px";

        var td = document.getElementsByTagName("td");
        for (var i = 0; i < td.length; i++){
            td.item(i).style.border = "1px solid black";
            td.item(i).style.borderCollapse = "collapse";
            td.item(i).style.fontSize = "150%";
        }

        var tdLabel = document.getElementsByClassName("label");
        for (var i = 0; i < tdLabel.length; i++){
            tdLabel.item(i).style.backgroundColor = "#210ebd";
            tdLabel.item(i).style.color = "aliceblue";
        }
        var tdValue = document.getElementsByClassName("value");
        for (var i = 0; i < tdValue.length; i++){
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
    }

    

};