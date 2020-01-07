const custom_events = require('events');

class MJ_Events extends custom_events {}

var info = {
    state: 0,
    tt: 0,
    pp: 0,
    sp: 0,
    spO: 0,
    spC: 0,
    sw: 0,
    sv: 0,
    tm: 0,
    p1: 0,
    p2: 0,
    lam: 0,
    pwm0: 0,
    pwm1: 0,
    v0: 0,
    spv: 0,
    comm: 0
};

var STATE = {
    START_AB: 1,
    START_CD: 2,
    START_EF: 3,
    STOP_FE: 4,
    STOP_DC: 5,
    STOP_BA: 6,
    READ_STATE: 7,
    READ_tt_0: 8,
    READ_tt_1: 9,
    READ_tt_2: 10,
    READ_tt_3: 11,
    READ_pp_0: 12,
    READ_pp_1: 13,
    READ_pp_2: 14,
    READ_pp_3: 15,
    READ_sp_0: 16,
    READ_sp_1: 17,
    READ_spO_0: 18,
    READ_spO_1: 19,
    READ_spC_0: 20,
    READ_spC_1: 21,
    READ_sw0_0: 22,
    READ_sw0_1: 23,
    READ_sv0_0: 24,
    READ_sv0_1: 25,
    READ_sw1_0: 26,
    READ_sw1_1: 27,
    READ_sv1_0: 28,
    READ_sv1_1: 29,
    READ_tm_0: 30,
    READ_tm_1: 31,
    READ_p1_0: 32,
    READ_p1_1: 33,
    READ_p1_2: 34,
    READ_p1_3: 35,
    READ_p2_0: 36,
    READ_p2_1: 37,
    READ_p2_2: 38,
    READ_p2_3: 39,
    READ_lam_0: 40,
    READ_lam_1: 41,
    READ_pwm0_1: 42,
    READ_pwm0_1: 43,
    READ_pwm1_0: 44,
    READ_pwm1_1: 45,
    READ_v0_0: 46,
    READ_v0_1: 47,
    READ_spv_0: 48,
    READ_spv_1: 49,
    READ_COMM: 50
}

var state = STATE.START_AB;

var buffer = new Array();

exports.events = new MJ_Events();

exports.reset = () => {
    state = STATE.START_AB;
    buffer = new Array();
};

exports.serial_data = (data, isNotification) => {
    for (let count=0; count<data.length; count++)
        buffer.push(data[count]);
};

exports.readSerialData = () => {
    var c;
    if (buffer.length > 0) {
        c = buffer.shift();
        //
        switch (state) {
        case STATE.START_AB: if (c == 0xAB) state = STATE.START_CD; break;
        case STATE.START_CD: if (c == 0xCD) state = STATE.START_EF; break;
        case STATE.START_EF:
            if (c == 0xEF) {
                info.state = 0;
                info.tt = 0;
                info.pp = 0;
                info.sp = 0;
                info.spO = 0;
                info.spC = 0;
                info.sw0 = 0;
                info.sv0 = 0;
                info.sw1 = 0;
                info.sv1 = 0;
                info.tm = 0;
                info.p1 = 0;
                info.p2 = 0;
                info.lam = 0;
                info.pwm0 = 0;
                info.pwm1 = 0;
                info.v0 = 0;
                info.spv = 0;
		        info.comm = 0;
                //
                state = STATE.READ_STATE;
            }
            break;
        case STATE.STOP_FE: if (c == 0xFE) state = STATE.STOP_DC; break;
        case STATE.STOP_DC: if (c == 0xDC) state = STATE.STOP_BA; break;
        case STATE.STOP_BA:
            if (c == 0xBA) {
                exports.events.emit('mj-log',info);
                //
                state = STATE.START_AB;
            }
            break;
        case STATE.READ_STATE: info.state = c;     state = STATE.READ_tt_0;   break;
        case STATE.READ_tt_0:  info.tt |= c;       state = STATE.READ_tt_1;   break;
        case STATE.READ_tt_1:  info.tt |= (c<<8);  state = STATE.READ_tt_2;   break;
        case STATE.READ_tt_2:  info.tt |= (c<<16); state = STATE.READ_tt_3;   break;
        case STATE.READ_tt_3:  info.tt |= (c<<24); state = STATE.READ_pp_0;   break;
        case STATE.READ_pp_0:  info.pp |= c;       state = STATE.READ_pp_1;   break;
        case STATE.READ_pp_1:  info.pp |= (c<<8);  state = STATE.READ_pp_2;   break;
        case STATE.READ_pp_2:  info.pp |= (c<<16); state = STATE.READ_pp_3;   break;
        case STATE.READ_pp_3:  info.pp |= (c<<24); state = STATE.READ_sp_0;   break;
        case STATE.READ_sp_0:  info.sp |= c;       state = STATE.READ_sp_1;   break;
        case STATE.READ_sp_1:  info.sp |= (c<<8);  state = STATE.READ_spO_0;  break;
        case STATE.READ_spO_0: info.spO |= c;      state = STATE.READ_spO_1;  break;
        case STATE.READ_spO_1: info.spO |= (c<<8); state = STATE.READ_spC_0;  break;
        case STATE.READ_spC_0: info.spC |= c;      state = STATE.READ_spC_1;  break;
        case STATE.READ_spC_1: info.spC |= (c<<8); state = STATE.READ_sw0_0;  break;
        case STATE.READ_sw0_0: info.sw0 |= c;      state = STATE.READ_sw0_1;  break;
        case STATE.READ_sw0_1: info.sw0 |= (c<<8); state = STATE.READ_sv0_0;  break;
        case STATE.READ_sv0_0: info.sv0 |= c;      state = STATE.READ_sv0_1;  break;
        case STATE.READ_sv0_1: info.sv0 |= (c<<8); state = STATE.READ_sw1_0;  break;
        case STATE.READ_sw1_0: info.sw1 |= c;      state = STATE.READ_sw1_1;  break;
        case STATE.READ_sw1_1: info.sw1 |= (c<<8); state = STATE.READ_sv1_0;  break;
        case STATE.READ_sv1_0: info.sv1 |= c;      state = STATE.READ_sv1_1;  break;
        case STATE.READ_sv1_1: info.sv1 |= (c<<8); state = STATE.READ_tm_0;   break;
        case STATE.READ_tm_0:  info.tm |= c;       state = STATE.READ_tm_1;   break;
        case STATE.READ_tm_1:  info.tm |= (c<<8);  state = STATE.READ_p1_0;   break;
        case STATE.READ_p1_0:  info.p1 |= c;       state = STATE.READ_p1_1;   break;
        case STATE.READ_p1_1:  info.p1 |= (c<<8);  state = STATE.READ_p1_2;   break;
        case STATE.READ_p1_2:  info.p1 |= (c<<16); state = STATE.READ_p1_3;   break;
        case STATE.READ_p1_3:  info.p1 |= (c<<24); state = STATE.READ_p2_0;   break;
        case STATE.READ_p2_0:  info.p2 |= c;       state = STATE.READ_p2_1;   break;
        case STATE.READ_p2_1:  info.p2 |= (c<<8);  state = STATE.READ_p2_2;   break;
        case STATE.READ_p2_2:  info.p2 |= (c<<16); state = STATE.READ_p2_3;   break;
        case STATE.READ_p2_3:  info.p2 |= (c<<24); state = STATE.READ_lam_0;  break;
        case STATE.READ_lam_0: info.lam |= c;      state = STATE.READ_lam_1;  break;
        case STATE.READ_lam_1: info.lam |= (c<<8); state = STATE.READ_pwm0_0; break;
        case STATE.READ_pwm0_0:info.pwm0 |= c;     state = STATE.READ_pwm0_1; break;
        case STATE.READ_pwm0_1:info.pwm0 |= (c<<8);state = STATE.READ_pwm1_0; break;
        case STATE.READ_pwm1_0:info.pwm1 |= c;     state = STATE.READ_pwm1_1; break;
        case STATE.READ_pwm1_1:info.pwm1 |= (c<<8);state = STATE.READ_v0_0;   break;
        case STATE.READ_v0_0:  info.v0 |= c;       state = STATE.READ_v0_1;   break;
        case STATE.READ_v0_1:  info.v0 |= (c<<8);  state = STATE.READ_spv_0;  break;
        case STATE.READ_spv_0: info.spv |= c;      state = STATE.READ_spv_1;  break;
        case STATE.READ_spv_1: info.spv |= (c<<8); state = STATE.READ_COMM;   break;
        case STATE.READ_COMM:  info.comm |= c;     state = STATE.STOP_FE;     break;
        default: break;
        }
    }
    //
    setTimeout(exports.readSerialData,1);
};

