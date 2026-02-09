const fs = require('fs');
const vm = require('vm');
const assert = require('assert');
const path = require('path');

// Read the source file
const sourcePath = path.join(__dirname, '../SD/graphoffline.js');
const sourceCode = fs.readFileSync(sourcePath, 'utf8');

// Mock jQuery
const mockJQueryObject = {
    // Chainable methods returning self
    change: function(cb) { return this; }, // Don't execute callback, just mock registration
    click: function() { return this; },
    keyup: function() { return this; },
    on: function() { return this; },
    bind: function() { return this; },
    resize: function() { return this; },
    hide: function() { return this; },
    show: function() { return this; },
    empty: function() { return this; },
    append: function() { return this; },
    prop: function() { return this; },
    css: function() { return this; },
    addClass: function() { return this; },
    removeClass: function() { return this; },

    // Value/Getter methods
    val: function() { return 0; }, // Return 0 or empty string as needed
    width: function() { return 100; },
    height: function() { return 100; },
    html: function() { return this; },
    children: function() { return this; },

    // Specific library mocks
    datetimepicker: function() { return this; },
    data: function(key) {
        if (key === 'DateTimePicker') {
            return {
                maxDate: function() {},
                minDate: function() {},
                date: function() {},
                viewDate: function() {
                    return {
                        format: function() { return Date.now(); },
                        diff: function() { return 3600; } // Default duration 1h
                    };
                }
            };
        }
        return {};
    }
};

// Make the function callable directly: $(...)
const $ = function(selector) {
    return mockJQueryObject;
};

// Add properties to $ function object itself if needed (e.g. $.ajax)
$.ajax = function() {};
$.isNumeric = function() { return true; };
$.plot = function() { return {}; };

// Mock window and document
const window = {
    location: {
        search: '',
        host: 'localhost'
    }
};

const document = {
    title: 'Test',
    createElement: function() { return { setAttribute: function() {}, style: {} }; },
    execCommand: function() {},
    body: {
        appendChild: function() {},
        removeChild: function() {}
    }
};

// Sandbox context
const sandbox = {
    window: window,
    document: document,
    $: $,
    jQuery: $,
    URLSearchParams: URLSearchParams,
    console: console,
    location: window.location,
    setTimeout: setTimeout,
    clearTimeout: clearTimeout,
    // Mock moment.js
    moment: function() {
        return {
            format: function() { return '2023-01-01'; },
            unix: function() { return Date.now() / 1000; }
        };
    }
};
sandbox.moment.unix = function() { return { format: function() { return '2023-01-01T00:00:00Z'; } }; };

// Create context and run script
vm.createContext(sandbox);

try {
    vm.runInContext(sourceCode, sandbox);
    console.log("Successfully loaded SD/graphoffline.js in sandbox.");
} catch (e) {
    console.error("Error loading script:", e);
    process.exit(1);
}

// Extract the function to test
const source_type = sandbox.source_type;

if (typeof source_type !== 'function') {
    console.error("Could not find source_type function in sandbox.");
    process.exit(1);
}

// Test cases
console.log("Running tests for source_type()...");

const testCases = [
    { input: "Volts", expected: "voltage" },
    { input: "Hz", expected: "voltage" },
    { input: "Watts", expected: "power" },
    { input: "Amps", expected: "power" }, // Based on current implementation
    { input: "Wh", expected: "power" },
    { input: "VA", expected: "power" },
    { input: "PF", expected: "power" },
    { input: "VAR", expected: "power" },
    { input: "VARh", expected: "power" },
    { input: "Unknown", expected: "power" },
    { input: "", expected: "power" }
];

let passed = 0;
let failed = 0;

testCases.forEach(test => {
    try {
        const result = source_type(test.input);
        assert.strictEqual(result, test.expected, `Expected source_type("${test.input}") to be "${test.expected}", got "${result}"`);
        passed++;
    } catch (e) {
        console.error(`FAIL: ${e.message}`);
        failed++;
    }
});

console.log(`\nTest Summary: ${passed} passed, ${failed} failed.`);

if (failed > 0) {
    process.exit(1);
} else {
    console.log("All tests passed!");
}
