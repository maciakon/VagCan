#include "Can\MeasurementValueCalculator.h"

humanReadablePidValue MeasurementValueCalculator::calculateValueFromSensorReading(byte *sensorReading)
{
    byte pidCode = sensorReading[2];
    byte A = sensorReading[3];
    byte B = sensorReading[4];
    byte C = sensorReading[5];
    byte D = sensorReading[6];
    humanReadablePidValue kvp;

    switch (pidCode)
    {
        case PID_ENGINE_RPM:
            kvp.value =  (256 * A + B) / 4;
            kvp.humanReadable = "ENGINE_RPM";
            return kvp;
        case PID_ENGINE_LOAD:
            kvp.value = (100 * A) / 255;
            kvp.humanReadable = "ENGINE_LOAD";
            return kvp;
        case PID_FUEL_PRESSURE:
            kvp.value = 3 * A;
            kvp.humanReadable = "FUEL_PRESSURE";
            return kvp;
        case PID_INTAKE_TEMP:
            kvp.value = A - 40;
            kvp.humanReadable = "INTAKE_TEMP";
            return kvp;
        case PID_INTAKE_MAP:
            kvp.value = A;
            kvp.humanReadable = "INTAKE_MAP";
            return kvp;
        case PID_MAF_FLOW:
            kvp.value = (256 * A + B) / 100;
            kvp.humanReadable = "MAF_FLOW";
            return kvp;
        case PID_THROTTLE:
            kvp.value = (100 * A) / 255;
            kvp.humanReadable = "THROTTLE";
            return kvp;
        case PID_VEHICLE_SPEED:
            kvp.value = A;
            kvp.humanReadable = "VEHICLE_SPEED";
            return kvp;
        case PID_COOLANT_TEMP:
            kvp.value =  A - 40;
            kvp.humanReadable = "COOLANT_TEMP";
            return kvp;
        default:
            kvp.value = pidCode;
            kvp.humanReadable ="UNSUPPORTED_PID_ID";
            return kvp;
    }
}