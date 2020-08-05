var Model = /** @class */ (function () {
    function Model() {
        this.l = ["0", "0", "0", "0", "0", "0"];
    }
    Model.prototype.loadStates = function (data) {
        this.l[0] = data.lines[0];
        this.l[1] = data.lines[1];
        this.l[2] = data.lines[2];
        this.l[3] = data.lines[3];
        this.l[4] = data.lines[4];
        this.l[5] = data.lines[5];
    };
    Model.prototype.getLineStates = function () {
        var d = { "l0": this.l[0], "l1": this.l[1], "l2": this.l[2], "l3": this.l[3], "l4": this.l[4], "l5": this.l[5] };
        return d;
    };
    Model.prototype.toggleState = function (lineNumber) {
        switch (this.l[lineNumber]) {
            case "0":
                this.l[lineNumber] = "1";
                break;
            case "1":
                this.l[lineNumber] = "2";
                break;
            case "2":
                this.l[lineNumber] = "0";
                break;
        }
    };
    return Model;
}());
var ViewMainPage = /** @class */ (function () {
    function ViewMainPage(myf, m) {
        var _this = this;
        this.flagToggle = false;
        this.myf = myf;
        this.model = m;
        setInterval(function () { _this.updateImages(); }, 250);
        setInterval(function () { _this.blink(); }, 600);
    }
    ViewMainPage.prototype.blink = function () {
        if (this.flagToggle)
            this.flagToggle = false;
        else
            this.flagToggle = true;
    };
    ViewMainPage.prototype.changeImg = function (el, state, lineNumber) {
        switch (state) {
            case "0":
                el.src = "images/" + lineNumber + "_on.png";
                break;
            case "1":
                el.src = "images/" + lineNumber + "_off.png";
                break;
            case "2":
                if (this.flagToggle)
                    el.src = "images/" + lineNumber + "_on.png";
                else
                    el.src = "images/" + lineNumber + "_blank.png";
                break;
        }
    };
    ViewMainPage.prototype.updateImages = function () {
        var el = this.myf.getElementById("line_0");
        this.changeImg(el, this.model.l[0], 0);
        el = this.myf.getElementById("line_1");
        this.changeImg(el, this.model.l[1], 1);
        el = this.myf.getElementById("line_2");
        this.changeImg(el, this.model.l[2], 2);
        el = this.myf.getElementById("line_3");
        this.changeImg(el, this.model.l[3], 3);
        el = this.myf.getElementById("line_4");
        this.changeImg(el, this.model.l[4], 4);
        el = this.myf.getElementById("line_5");
        this.changeImg(el, this.model.l[5], 5);
    };
    return ViewMainPage;
}());
var Main = /** @class */ (function () {
    function Main() {
    }
    Main.prototype.handleEvent = function (evt) {
        var el = this.myf.getElementByEvent(evt);
        console.log("click en elemento:" + el.id);
        if (el.id == "btnMsg") {
        }
        else {
            // click en imagen
            var lineNumber = parseInt(el.id.split("_")[1]);
            console.log(lineNumber);
            this.model.toggleState(lineNumber);
            var d = this.model.getLineStates();
            //let data:object = {"data":d};
            console.log(d);
            this.myf.requestPOST("cgi-bin/setStates.py", d, this);
        }
    };
    Main.prototype.handleGETResponse = function (status, response) {
        if (status == 200) {
            var data = JSON.parse(response);
            //this.view.showDevices(data);    
            console.log(data);
            this.model.loadStates(data);
        }
    };
    Main.prototype.handlePOSTResponse = function (status, response) {
        if (status == 200) {
            console.log(response);
        }
    };
    Main.prototype.main = function () {
        var _this = this;
        this.myf = new MyFramework();
        this.model = new Model();
        this.view = new ViewMainPage(this.myf, this.model);
        setInterval(function () { _this.myf.requestGET("cgi-bin/getLines.py", _this); }, 1000);
        var i;
        for (i = 0; i < 6; i++) {
            var im = this.myf.getElementById("line_" + i);
            im.addEventListener("click", this);
        }
        var b = this.myf.getElementById("btnMsg");
        b.addEventListener("click", this);
    };
    return Main;
}());
window.onload = function () {
    var obj = new Main();
    obj.main();
};
