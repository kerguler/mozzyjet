#!/usr/bin/env node

var bluetooth = true;
var diagnose = false;
var handle = null;

var term = require( 'terminal-kit' ).terminal;
const figlet = require("figlet");
const fs = require('fs');

var crack = require('./crack');

var key = null;

var logfile = null;
var calib = '';
var calib0 = '';
var logdata = true;

var logline = 19;
var cmdline = 20;

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

const getInstruction = (id,title) => {
	logdata = false;
    term_state = TERM.READ_DATA;
	//
	term.moveTo(1,cmdline,title);
    //
    term.inputField({
        history: [],
        autoComplete: [],
        autoCompleteMenu: false
    }, function(error,input) {
        if (error) {
            logCommands(`Unexpected input: ${input}`);
        } else {
            var okay = false;
            var num = parseInt(input);
            if (isNaN(num))
                logCommands(`Unexpected input: ${input}`);
            else
                okay = true;
            if (okay) {
                logCommands(`Command successful`);
                var buf = Buffer.allocUnsafe(2);
                buf.writeInt16BE(num, 0);
	            command.push(Buffer.alloc(1,id));
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
	        command.push(Buffer.alloc(1,'R'));
	        logCommands(`Spraying ${key.spv} mL`);
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
	        term.moveTo(1,cmdline,"Set opening time (R: random 100-1000 ms): ");
            //
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
	    case 'v':
	    case 'V':
            getInstruction('V','Set container volume: ');
	        break;
	    case 's':
	    case 'S':
            getInstruction('S','Set spray volume: ');
            break;
	    case 'l':
	    case 'L':
            getInstruction('L','Set PWM lambda: ');
            break;
	    case '<':
            getInstruction('<','Set PWM 0 thr. L: ');
            break;
	    case '>':
            getInstruction('>','Set PWM 0 thr. R: ');
            break;
	    case 'n':
	    case 'N':
            getInstruction('N','Set PWM servo O: ');
            break;
	    case 'm':
	    case 'M':
            getInstruction('M','Set PWM servo C: ');
            break;
	    default:
	        break;
	    }
	    break;
    default:
        break;
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
    term.gray("O: open valve           C: close valve\n");
    term.gray("T: run test             R: spray\n");
    term.gray("V: set container volume S: set spray volume\n");
    term.gray("L: set PWM lambda       \n");
    term.gray("<: set PWM 0 thr. L     >: set PWM 0 thr. R\n");
    term.gray("N: set PWM servo O      M: set PWM servo C\n\n");
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
    term.moveTo(1,logline).gray("%s",message).eraseLineAfter();
};

crack.events.on('mj-log', function (data) {
    key = data;
    if (!logdata) return;
    //
    term.moveTo(50,1, "State: %d   ",data.state).eraseLineAfter();
    term.moveTo(50,2, "Temp: %d   ",data.tt).eraseLineAfter();
    term.moveTo(50,3, "Pres: %d   ",data.pp).eraseLineAfter();
    term.moveTo(50,4, "ServoPos: [%d: %d - %d]   ",data.sp,data.spO,data.spC).eraseLineAfter();
    term.moveTo(50,5, "PWM [%d: %d - %d]   ",data.lam,data.pwm0,data.pwm1).eraseLineAfter();
    term.moveTo(50,6, "   w0: %d   s0: %d   ",data.sw0,data.sv0).eraseLineAfter();
    term.moveTo(50,7, "   w1: %d   s1: %d   ",data.sw1,data.sv1).eraseLineAfter();
    term.moveTo(50,8, "Calibration").eraseLineAfter();
    term.moveTo(50,9, "   Duration: %d   ",data.tm).eraseLineAfter();
    term.moveTo(50,10,"   Pressure before: %d   ",data.p1).eraseLineAfter();
    term.moveTo(50,11,"   Pressure after: %d   ",data.p2).eraseLineAfter();
    term.moveTo(50,12,"Container volume: %d   ",data.v0).eraseLineAfter();
    term.moveTo(50,13,"Spray volume: %d   ",data.spv).eraseLineAfter();
    term.moveTo(50,14,"Received command: %d   ",data.comm).eraseLineAfter();
    term.moveTo(1,cmdline).eraseLineAfter();
    //
    calib = `${data.tm},${data.p1},${data.p2}`;
    if (calib != calib0) {
        logfile.write(calib+"\n");
        calib0 = calib;
    }
    //
    if (command.length > 0) {
        c = command.shift();
        //
        sendCommand(c);
    }
});

const connectSerialPort = () => {
    const SerialPort = require("serialport");
    // const portId = "/dev/cu.uart-F7FF5184AB940D3F";
    const portId = "/dev/cu.HC-06-SPPDev-1";
    //
    const serial_error = (error) => {
	    console.dir(error);
    };
    //
    if (handle != null) {
	    handle.close(function (err) {
	        handle = new SerialPort(portId, { baudRate: 9600 });
	        handle.on('error', serial_error);
	        handle.on('open', init);
	        logCommands("Serial connection requested...");
	    });
    } else {
	    handle = new SerialPort(portId, { baudRate: 9600 });
	    handle.on('error', serial_error);
	    handle.on('open', init);
    }
};

const setupBluetooth = () => {
    var noble = require('./noble-mac-master');
    // var peripheralIdOrAddress = '58ee3bc354e844dea7a084c930710ece';
    var peripheralIdOrAddress =    '2504f5f6e24440ebafd8d5eb9e71882a';
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
                    //
                    handle = characteristics[0];
                    handle.subscribe(error => {
                        if (error) {
                            console.error('Error subscribing to characteristic');
                        } else {
                            console.log('Subscribed for characteristic notifications');
                            init();
                        }
                    });
                }
            );
            //
        });
        //
        peripheral.on('disconnect', () => console.log('disconnected'));
    });
    //
};

const diagnoseBluetooth = () => {
    var noble = require('./noble-mac-master');
    // var peripheralIdOrAddress = '58ee3bc354e844dea7a084c930710ece';
    var peripheralIdOrAddress =    '2504f5f6e24440ebafd8d5eb9e71882a';
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
        console.log('Scanning:' + peripheral.id + ' ' + peripheral.address);
        if (peripheral.id !== peripheralIdOrAddress) return;
        noble.stopScanning();
        //
        console.log('Peripheral with ID ' + peripheral.id + ' found');
        //
        peripheral.connect(error => {
            console.log('Connected to', peripheral.id);
            console.dir(peripheral);
            peripheral.discoverServices(null, function(error, services) {
                console.log('discovered the following services:');
                for (var i in services) {
                    console.log('  ' + i + ' uuid: ' + services[i].uuid);
                    services[i].discoverCharacteristics(null, function(error, characteristics) {
                        console.log('discovered the following characteristics:');
                        for (var j in characteristics) {
                            console.log(' service: ' + characteristics[j]._serviceUuid + ' uuid: ' + characteristics[j].uuid + ' name: ' + characteristics[j].name + ' type: ' + characteristics[j].type + ' properties: ' + characteristics[j].properties);
                        }
                    });
                }
            });
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
    if (bluetooth) {
        if (diagnose)
            diagnoseBluetooth();
        else
            setupBluetooth();
    } else
        connectSerialPort();
}

run();
