#pragma once

#include "CommunicationCodes.hh" 
#include <cstdint>
#include <ostream>

enum class TOFCommandCode : uint16_t {
    START_DAQ                        = 0x5000,
    STOP_DAQ                         = 0x5001,
    RESET_DAQ                        = 0x5002,

    INIT_SYSTEM                      = 0x5100,
    MAKE_BIAS_CALIB_TABLE            = 0x5101,
    MAKE_SIMPLE_BIAS_SET_TABLE       = 0x5102,
    MAKE_SIMPLE_CHANNEL_MAP          = 0x5103,
    MAKE_SIMPLE_DISC_SET_TABLE       = 0x5104,
    READ_TEMPERATURE_SENSORS         = 0x5105,
    ACQUIRE_THRESHOLD_CALIBRATION    = 0x5106,
    ACQUIRE_TDC_CALIBRATION          = 0x5107,
    ACQUIRE_QDC_CALIBRATION          = 0x5108,
    ACQUIRE_SIPM_DATA                = 0x5109,
    ACQUIRE_THRESHOLD_CALIBRATION_BN = 0x5110,
    ACQUIRE_THRESHOLD_CALIBRATION_D  = 0x5111,

    PROCESS_THRESHOLD_CALIBRATION    = 0x5200,
    PROCESS_TDC_CALIBRATION          = 0x5201,
    PROCESS_QDC_CALIBRATION          = 0x5202,
    CONVERT_RAW_TO_RAW               = 0x5203,
    CONVERT_RAW_TO_SINGLES           = 0x5204,

    CONVERT_STG1_TO_STG2             = 0x5300,
    PROCESS_QA_COIN                  = 0x5301,
    PROCESS_QA_IRIDIUM               = 0x5302,

    MONITOR_DATA_STREAM              = 0x5400,

    ACK                              = 0x5FFF,
    CALLBACK                         = 0x5FFE,
    STATUS                           = 0x5FFD,
    DUMMY_TEST                       = 0x5FFC,

    HEART_BEAT                       = 0xFFFF,
    UNKNOWN                          = 0x0000
};

enum class AckStatus : uint8_t {
    FAILURE = 0,
    SUCCESS = 1
};

inline std::ostream& operator<<(std::ostream& os, TOFCommandCode code) {
    switch (code) {
        case TOFCommandCode::START_DAQ:                        return os << "START_DAQ";
        case TOFCommandCode::STOP_DAQ:                         return os << "STOP_DAQ";
        case TOFCommandCode::RESET_DAQ:                        return os << "RESET_DAQ";

        case TOFCommandCode::INIT_SYSTEM:                      return os << "INIT_SYSTEM";
        case TOFCommandCode::MAKE_BIAS_CALIB_TABLE:            return os << "MAKE_BIAS_CALIB_TABLE";
        case TOFCommandCode::MAKE_SIMPLE_BIAS_SET_TABLE:       return os << "MAKE_SIMPLE_BIAS_SET_TABLE";
        case TOFCommandCode::MAKE_SIMPLE_CHANNEL_MAP:          return os << "MAKE_SIMPLE_CHANNEL_MAP";
        case TOFCommandCode::MAKE_SIMPLE_DISC_SET_TABLE:       return os << "MAKE_SIMPLE_DISC_SET_TABLE";
        case TOFCommandCode::READ_TEMPERATURE_SENSORS:         return os << "READ_TEMPERATURE_SENSORS";
        case TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION:    return os << "ACQUIRE_THRESHOLD_CALIBRATION";
        case TOFCommandCode::ACQUIRE_TDC_CALIBRATION:          return os << "ACQUIRE_TDC_CALIBRATION";
        case TOFCommandCode::ACQUIRE_QDC_CALIBRATION:          return os << "ACQUIRE_QDC_CALIBRATION";
        case TOFCommandCode::ACQUIRE_SIPM_DATA:                return os << "ACQUIRE_SIPM_DATA";
        case TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_BN: return os << "ACQUIRE_THRESHOLD_CALIBRATION_BN";
        case TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_D:  return os << "ACQUIRE_THRESHOLD_CALIBRATION_D";
        
        case TOFCommandCode::PROCESS_THRESHOLD_CALIBRATION:    return os << "PROCESS_THRESHOLD_CALIBRATION";
        case TOFCommandCode::PROCESS_TDC_CALIBRATION:          return os << "PROCESS_TDC_CALIBRATION";
        case TOFCommandCode::PROCESS_QDC_CALIBRATION:          return os << "PROCESS_QDC_CALIBRATION";
        case TOFCommandCode::CONVERT_RAW_TO_RAW:               return os << "CONVERT_RAW_TO_RAW";
        case TOFCommandCode::CONVERT_RAW_TO_SINGLES:           return os << "CONVERT_RAW_TO_SINGLES";

        case TOFCommandCode::CONVERT_STG1_TO_STG2:             return os << "CONVERT_STG1_TO_STG2";
        case TOFCommandCode::PROCESS_QA_COIN:                  return os << "PROCESS_QA_COIN";
        case TOFCommandCode::PROCESS_QA_IRIDIUM:               return os << "PROCESS_QA_IRIDIUM";
        
        case TOFCommandCode::MONITOR_DATA_STREAM:              return os << "MONITOR_DATA_STREAM";

        case TOFCommandCode::ACK:                              return os << "ACK";
        case TOFCommandCode::CALLBACK:                         return os << "CALLBACK";
        case TOFCommandCode::STATUS:                           return os << "STATUS";
        case TOFCommandCode::DUMMY_TEST:                       return os << "DUMMY_TEST";

        case TOFCommandCode::HEART_BEAT:                       return os << "HEART_BEAT";
        default:                                               return os << "UNKNOWN";
    }
}

namespace tof_bridge {

using namespace pgrams::communication;

// Convert TOFCommandCode -> CommunicationCodes
inline CommunicationCodes toCommCode(TOFCommandCode code) {
    switch (code) {
        case TOFCommandCode::START_DAQ:                        return CommunicationCodes::TOF_Start_DAQ;
        case TOFCommandCode::STOP_DAQ:                         return CommunicationCodes::TOF_Stop_DAQ;
        case TOFCommandCode::RESET_DAQ:                        return CommunicationCodes::TOF_Reset_DAQ;

        case TOFCommandCode::INIT_SYSTEM:                      return CommunicationCodes::TOF_Init_System;
        case TOFCommandCode::MAKE_BIAS_CALIB_TABLE:            return CommunicationCodes::TOF_Make_Bias_Calib_Table;
        case TOFCommandCode::MAKE_SIMPLE_BIAS_SET_TABLE:       return CommunicationCodes::TOF_Make_Simple_Bias_Set_Table;
        case TOFCommandCode::MAKE_SIMPLE_CHANNEL_MAP:          return CommunicationCodes::TOF_Make_Simple_Channel_Map;
        case TOFCommandCode::MAKE_SIMPLE_DISC_SET_TABLE:       return CommunicationCodes::TOF_Make_Simple_Disc_Set_Table;
        case TOFCommandCode::READ_TEMPERATURE_SENSORS:         return CommunicationCodes::TOF_Read_Temperature_Sensors;
        case TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION:    return CommunicationCodes::TOF_Acquire_Threshold_Calibration;
        case TOFCommandCode::ACQUIRE_TDC_CALIBRATION:          return CommunicationCodes::TOF_Acquire_TDC_Calibration;
        case TOFCommandCode::ACQUIRE_QDC_CALIBRATION:          return CommunicationCodes::TOF_Acquire_QDC_Calibration;
        case TOFCommandCode::ACQUIRE_SIPM_DATA:                return CommunicationCodes::TOF_Acquire_SiPM_Data;
        case TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_BN: return CommunicationCodes::TOF_Acquire_Threshold_Calibration_BN;
        case TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_D:  return CommunicationCodes::TOF_Acquire_Threshold_Calibration_D;

        case TOFCommandCode::PROCESS_THRESHOLD_CALIBRATION:    return CommunicationCodes::TOF_Process_Threshold_Calibration;
        case TOFCommandCode::PROCESS_TDC_CALIBRATION:          return CommunicationCodes::TOF_Process_TDC_Calibration;
        case TOFCommandCode::PROCESS_QDC_CALIBRATION:          return CommunicationCodes::TOF_Process_QDC_Calibration;
        case TOFCommandCode::CONVERT_RAW_TO_RAW:               return CommunicationCodes::TOF_Convert_Raw_To_Raw;
        case TOFCommandCode::CONVERT_RAW_TO_SINGLES:           return CommunicationCodes::TOF_Convert_Raw_To_Singles;

        case TOFCommandCode::CONVERT_STG1_TO_STG2:             return CommunicationCodes::TOF_Convert_Stg1_To_Stg2;
        case TOFCommandCode::PROCESS_QA_COIN:                  return CommunicationCodes::TOF_Process_QA_Coin;
        case TOFCommandCode::PROCESS_QA_IRIDIUM:               return CommunicationCodes::TOF_Process_QA_Iridium;

        case TOFCommandCode::MONITOR_DATA_STREAM:              return static_cast<pgrams::communication::CommunicationCodes>(
                                                                   pgrams::communication::TelemetryCodes::TOF_Monitor_Data_Stream);

        case TOFCommandCode::ACK:                              return CommunicationCodes::TOF_ACK;
        case TOFCommandCode::CALLBACK:                         return CommunicationCodes::TOF_Callback;
        case TOFCommandCode::DUMMY_TEST:                       return CommunicationCodes::TOF_DummyTest;

        case TOFCommandCode::HEART_BEAT:                       return CommunicationCodes::COM_HeartBeat;

        default: return CommunicationCodes::TOF_Status; // safe fallback
    }
}

// Convert CommunicationCodes -> TOFCommandCode
inline TOFCommandCode toTOFCommand(CommunicationCodes code) {
    switch (code) {
        case CommunicationCodes::TOF_Start_DAQ:                        return TOFCommandCode::START_DAQ;
        case CommunicationCodes::TOF_Stop_DAQ:                         return TOFCommandCode::STOP_DAQ;
        case CommunicationCodes::TOF_Reset_DAQ:                        return TOFCommandCode::RESET_DAQ;

        case CommunicationCodes::TOF_Init_System:                      return TOFCommandCode::INIT_SYSTEM;
        case CommunicationCodes::TOF_Make_Bias_Calib_Table:            return TOFCommandCode::MAKE_BIAS_CALIB_TABLE;
        case CommunicationCodes::TOF_Make_Simple_Bias_Set_Table:       return TOFCommandCode::MAKE_SIMPLE_BIAS_SET_TABLE;
        case CommunicationCodes::TOF_Make_Simple_Channel_Map:          return TOFCommandCode::MAKE_SIMPLE_CHANNEL_MAP;
        case CommunicationCodes::TOF_Make_Simple_Disc_Set_Table:       return TOFCommandCode::MAKE_SIMPLE_DISC_SET_TABLE;
        case CommunicationCodes::TOF_Read_Temperature_Sensors:         return TOFCommandCode::READ_TEMPERATURE_SENSORS;
        case CommunicationCodes::TOF_Acquire_Threshold_Calibration:    return TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION;
        case CommunicationCodes::TOF_Acquire_TDC_Calibration:          return TOFCommandCode::ACQUIRE_TDC_CALIBRATION;
        case CommunicationCodes::TOF_Acquire_QDC_Calibration:          return TOFCommandCode::ACQUIRE_QDC_CALIBRATION;
        case CommunicationCodes::TOF_Acquire_SiPM_Data:                return TOFCommandCode::ACQUIRE_SIPM_DATA;
        case CommunicationCodes::TOF_Acquire_Threshold_Calibration_BN: return TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_BN;
        case CommunicationCodes::TOF_Acquire_Threshold_Calibration_D:  return TOFCommandCode::ACQUIRE_THRESHOLD_CALIBRATION_D;

        case CommunicationCodes::TOF_Process_Threshold_Calibration:    return TOFCommandCode::PROCESS_THRESHOLD_CALIBRATION;
        case CommunicationCodes::TOF_Process_TDC_Calibration:          return TOFCommandCode::PROCESS_TDC_CALIBRATION;
        case CommunicationCodes::TOF_Process_QDC_Calibration:          return TOFCommandCode::PROCESS_QDC_CALIBRATION;
        case CommunicationCodes::TOF_Convert_Raw_To_Raw:               return TOFCommandCode::CONVERT_RAW_TO_RAW;
        case CommunicationCodes::TOF_Convert_Raw_To_Singles:           return TOFCommandCode::CONVERT_RAW_TO_SINGLES;

        case CommunicationCodes::TOF_Convert_Stg1_To_Stg2:             return TOFCommandCode::CONVERT_STG1_TO_STG2;
        case CommunicationCodes::TOF_Process_QA_Coin:                  return TOFCommandCode::PROCESS_QA_COIN;
        case CommunicationCodes::TOF_Process_QA_Iridium:               return TOFCommandCode::PROCESS_QA_IRIDIUM;

        case static_cast<pgrams::communication::CommunicationCodes>(
          pgrams::communication::TelemetryCodes::TOF_Monitor_Data_Stream): return TOFCommandCode::MONITOR_DATA_STREAM;

        case CommunicationCodes::TOF_ACK:                              return TOFCommandCode::ACK;
        case CommunicationCodes::TOF_Callback:                         return TOFCommandCode::CALLBACK;
        case CommunicationCodes::TOF_DummyTest:                        return TOFCommandCode::DUMMY_TEST;

        case CommunicationCodes::COM_HeartBeat:                        return TOFCommandCode::HEART_BEAT;

        default: return TOFCommandCode::UNKNOWN; // safe default
    }
}

} // namespace tof_bridge
