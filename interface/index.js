#!/usr/bin/env node

var bluetooth = true;
var handle = null;

var term = require( 'terminal-kit' ).terminal;
const figlet = require("figlet");
const fs = require('fs');

var crack = require('./crack');

var logfile = null;
var calib = '';
var calib0 = '';
var logdata = true;

var TERM = {
    READ_COMMAND: 1,
    READ_DATA: 2
};

var term_state = TERM.READ_COMMAND;

var command = new Array();

const randomInt = (min,max) => {
    min = Math.floor(min);
    max = Math.floor(max);
    return Math.floor(min + Math.random() * (max-min));
};

const term_keypress = (name,matches,data) => {
    switch (term_state) {
    case TERM.READ_COMMAND:
	    switch (name) {
	    case 'CTRL_C': // Exit
	        term.grabInput(false);
	        setTimeout(function() {
		        process.exit()
	        }, 100);
	        break;
	    case 'r': // Renew connection
	    case 'R':
	        logCommands("Re-establishing serial connection...");
	        connectSerialPort();
	        break;
	    case 'o': // Open valve
	    case 'O':
	        command.push(Buffer.alloc(1,'O'));
	        logCommands("Command send: O - open valve");
	        break;
	    case 'c': // Close valve
	    case 'C':
	        command.push(Buffer.alloc(1,'C'));
	        logCommands("Command send: C - close valve");
	        break;
	    case 't':
	    case 'T':
	        logdata = false;
            term_state = TERM.READ_DATA;
	        //
	        term.moveTo(1,15,'Set opening time (R: random 100-1000 ms): ');
            //
            term.grabInput(false);
            term.inputField({
                history: [],
                autoComplete: [],
                autoCompleteMenu: false
            }, function(error,input) {
                if (error) {
                    logCommands(`Unexpected input: ${input}`);
                } else {
                    var num;
                    var okay = false;
                    if (input=='R' || input =='r') {
                        num = randomInt(100,1000);
                        okay = true;
                    } else {
                        num = parseInt(input);
                        if (isNaN(num))
                            logCommands(`Unexpected input: ${input}`);
                        else
                            okay = true;
                    }
                    if (okay) {
                        logCommands(`Performing spray test for ${num} ms`);
                        var buf = Buffer.allocUnsafe(2);
                        buf.writeInt16BE(num, 0);
	                    command.push(Buffer.alloc(1,'T'));
                        command.push(buf);
                    }
	            }
	            logdata = true;
                term_state = TERM.READ_COMMAND;
                setTimeout(function() {
                    term_keypress(0,0,0);
                }, 1);
	        }
                           );
	        break;
	    default:
	        break;
	    }
	    break;
    default:
        break;
    }
    //
    if (command.length > 0) {
        c = command.shift();
        //
        sendCommand(c);
    }
};

const init = () => {
    term.fullscreen(true);
    term.green(
        figlet.textSync("MozzyJet", {
            font: "big",
            horizontalLayout: "default",
            verticalLayout: "default"
        })
    );
    term.yellow("\n    by Kamil Erguler and the USRL @ CyI\n\n");
    //
    term.moveTo(1,11);
    term.blue.bold("Serial communication initiated!\n");
    term.yellow("Initial data transfer: [    ]\n");
    term.gray("[R: renew connection, O: open valve, C: close valve, T: run test]\n\n");
    //
    crack.reset();
    term_state = TERM.READ_COMMAND;
    //
    handle.on('data', crack.serial_data);
    //
    logfile = fs.createWriteStream("output.csv", {flags: 'a'});
    //
    term.grabInput();
    term.on('key', term_keypress);
    //
    crack.readSerialData();
}

const logCommands = (message) => {
    term.moveTo(1,14).gray(message+"\n");
};

crack.events.on('mj-log', function (data) {
    if (!logdata) return;
    //
    term.moveTo(50,1,"State: %d          ",data.state);
    term.moveTo(50,2,"Temp: %d          ",data.tt);
    term.moveTo(50,3,"Pres: %d          ",data.pp);
    term.moveTo(50,4,"ServoPos: %d          ",data.sp);
    term.moveTo(50,5,"PWM width: %d          ",data.sw);
    term.moveTo(50,6,"PWM signal: %d          ",data.sv);
    term.moveTo(50,7,"Calib. time: %d          ",data.tm);
    term.moveTo(50,8,"Calib. pres before: %d          ",data.p1);
    term.moveTo(50,9,"Calib. pres after: %d          ",data.p2);
    term.moveTo(50,10,"Received command: %d          ",data.comm);
    term.moveTo(1,15);
    //
    calib = `${data.tm},${data.p1},${data.p2}`;
    if (calib != calib0) {
        logfile.write(calib+"\n");
        calib0 = calib;
    }
});

const connectSerialPort = () => {
    const SerialPort = require("serialport"); 
    //
    const serial_error = (error) => {
	    console.dir(error);
    };
    //
    if (handle != null) {
	    handle.close(function (err) {
	        handle = new SerialPort("/dev/cu.uart-F7FF5184AB940D3F", { baudRate: 9600 });
	        handle.on('error', serial_error);
	        handle.on('open', init);
	        logCommands("Serial connection requested...");
	    });
    } else {
	    handle = new SerialPort("/dev/cu.uart-F7FF5184AB940D3F", { baudRate: 9600 });
	    handle.on('error', serial_error);
	    handle.on('open', init);
    }
};

const setupBluetooth = () => {
    var noble = require('./noble-mac-master');
    var peripheralIdOrAddress = '58ee3bc354e844dea7a084c930710ece';
    //
    noble.on('stateChange', function(state) {
        if (state === 'poweredOn') {
            noble.startScanning();
        } else {
            noble.stopScanning();
        }
    });
    //
    noble.on('discover', function(peripheral) {
        if (peripheral.id !== peripheralIdOrAddress) return;
        noble.stopScanning();
        //
        console.log('Peripheral with ID ' + peripheral.id + ' found');
        //
        peripheral.connect(error => {
            console.log('Connected to', peripheral.id);
            //
            peripheral.discoverSomeServicesAndCharacteristics(
                ['FFE0'], // serviceUUIDs,
                ['FFE1'], // characteristicUUIDs,
                function (error, services, characteristics) {
                    console.log('Discovered services and characteristics');
                    handle = characteristics[0];
                    handle.subscribe(error => {
                        if (error) {
                            console.error('Error subscribing to characteristic');
                        } else {
                            console.log('Subscribed for characteristic notifications');
                        }
                    });
                    init();
                }
            );
            //
        });
        //
        peripheral.on('disconnect', () => console.log('disconnected'));
    });
    //
};

const sendCommand = (command) => {
    if (bluetooth)
        handle.write(command,true);
    else
        handle.write(command);
};

const run = async () => {
    if (bluetooth)
        setupBluetooth();
    else
        connectSerialPort();
}

run();
