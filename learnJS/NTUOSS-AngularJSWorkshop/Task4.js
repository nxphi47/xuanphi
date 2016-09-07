function mainCtrl() {
    var self = this;
    self.people = [{
        name: "Eric Simons",
        country: "USA"
    }, {
        name: "Albert Pai",
        country: "Singapore"
    }, {
        name: "Matthew Greenster",
        country: "Sweden"
    }, {
        name: "Tim Brown",
        country: "USA"
    }, {
        name: "Jake Trump",
        country: "Canada"
    }, {
        name: "Albert Potter",
        country: "Canada"
    }, {
        name: "Bob Dylan",
        country: "USA"
    }, {
        name: "Bernie Sanders",
        country: "Singapore"
    }, {
        name: "Dwayne Johnson",
        country: "USA"
    }, {
        name: "Albert Colbert",
        country: "Germany"
    }];
}

function makeUpperCase() {
    return function (item) {
        return item.toUpperCase();
    }
}

angular.module('app', [])
    .filter('makeUpperCase', makeUpperCase())
    .controller('mainCtrl', mainCtrl);
